/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "app_event_observer_mgr.h"

#include "app_state_callback.h"
#include "app_event_processor_proxy.h"
#include "app_event_store.h"
#include "app_event_watcher.h"
#include "application_context.h"
#include "ffrt_inner.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hilog/log.h"
#include "os_event_listener.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ObserverMgr"

namespace OHOS {
namespace HiviewDFX {
using HiAppEvent::AppEventFilter;
using HiAppEvent::TriggerCondition;
using namespace AppEventCacheCommon;
namespace {
constexpr int MILLI_TO_MICRO = 1000;
constexpr int REFRESH_FREE_SIZE_INTERVAL = 10 * 60 * 1000; // 10 minutes
constexpr int TIMEOUT_INTERVAL_MILLI = HiAppEvent::TIMEOUT_STEP * 1000; // 30s
constexpr int MAX_SIZE_OF_INIT = 100;
constexpr int TIMEOUT_LIMIT_FOR_ADDPROCESSOR = 500;
constexpr int CHECK_DB_INTERVAL = 1;

void StoreEventsToDb(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    for (auto& event : events) {
        int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(event);
        if (eventSeq <= 0) {
            HILOG_WARN(LOG_CORE, "failed to store event to db");
            continue;
        }
        event->SetSeq(eventSeq);
        AppEventStore::GetInstance().QueryCustomParamsAdd2EventPack(event);
    }
}

void StoreEventMappingToDb(const std::vector<std::shared_ptr<AppEventPack>>& events,
    const std::vector<std::shared_ptr<AppEventObserver>>& observers)
{
    std::vector<EventObserverInfo> eventObserverInfos;
    for (const auto& observer : observers) {
        for (const auto& event : events) {
            if (observer->VerifyEvent(event)) {
                eventObserverInfos.emplace_back(EventObserverInfo(event->GetSeq(), observer->GetSeq()));
            }
        }
    }
    if (AppEventStore::GetInstance().InsertEventMapping(eventObserverInfos) < 0) {
        HILOG_ERROR(LOG_CORE, "failed to add mapping record to db");
    }
}

void SendEventsToObserver(const std::vector<std::shared_ptr<AppEventPack>>& events,
    std::shared_ptr<AppEventObserver> observer)
{
    std::vector<std::shared_ptr<AppEventPack>> realTimeEvents;
    for (const auto& event : events) {
        if (!observer->VerifyEvent(event)) {
            continue;
        }
        if (observer->IsRealTimeEvent(event)) {
            realTimeEvents.emplace_back(event);
        } else {
            observer->ProcessEvent(event);
        }
    }
    if (!realTimeEvents.empty()) {
        observer->OnEvents(realTimeEvents);
    }
}

int64_t StoreObserverToDb(std::shared_ptr<AppEventObserver> observer, const std::string& filters, int64_t hashCode)
{
    std::string name = observer->GetName();
    int64_t observerSeq = AppEventStore::GetInstance().InsertObserver(Observer(name, hashCode, filters));
    if (observerSeq <= 0) {
        HILOG_ERROR(LOG_CORE, "failed to insert observer=%{public}s to db", name.c_str());
        return -1;
    }
    observer->SetSeq(observerSeq);
    return observerSeq;
}

int64_t InitObserverFromDb(std::shared_ptr<AppEventObserver> observer, const std::string& filters, int64_t hashCode = 0)
{
    int64_t observerSeq = observer->GetSeq();
    if (observerSeq <= 0) {
        HILOG_INFO(LOG_CORE, "the observer does not exist in database, name=%{public}s, hash=%{public}" PRId64,
            observer->GetName().c_str(), hashCode);
        return StoreObserverToDb(observer, filters, hashCode);
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq, MAX_SIZE_OF_INIT) < 0) {
        HILOG_ERROR(LOG_CORE, "failed to take events, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    if (!events.empty()) {
        if (hashCode == 0) {
            // send old events to watcher where init
            SendEventsToObserver(events, observer);
        } else {
            TriggerCondition triggerCond;
            for (auto event : events) {
                triggerCond.row++;
                triggerCond.size += static_cast<int>(event->GetEventStr().size());
            }
            observer->SetCurrCondition(triggerCond);
        }
    }
    return observerSeq;
}
}

AppEventObserverMgr& AppEventObserverMgr::GetInstance()
{
    static AppEventObserverMgr instance;
    return instance;
}

AppEventObserverMgr::AppEventObserverMgr()
{
    RegisterAppStateCallback();
    moduleLoader_ = std::make_unique<ModuleLoader>();
    queue_ = std::make_shared<ffrt::queue>("AppEventQueue");
    SendRefreshFreeSizeTask();
}

void AppEventObserverMgr::RegisterAppStateCallback()
{
    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (context == nullptr) {
        HILOG_WARN(LOG_CORE, "app context is null");
        return;
    }
    appStateCallback_ = std::make_shared<AppStateCallback>();
    context->RegisterAbilityLifecycleCallback(appStateCallback_);
    HILOG_INFO(LOG_CORE, "succ to register application state callback");
}

AppEventObserverMgr::~AppEventObserverMgr()
{
    UnregisterAppStateCallback();
    queue_ = nullptr;
}

void AppEventObserverMgr::UnregisterAppStateCallback()
{
    if (appStateCallback_ == nullptr) {
        return;
    }

    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (context == nullptr) {
        HILOG_WARN(LOG_CORE, "app context is null");
        return;
    }
    context->UnregisterAbilityLifecycleCallback(appStateCallback_);
    appStateCallback_ = nullptr;
    HILOG_INFO(LOG_CORE, "succ to unregister application state callback");
}

void AppEventObserverMgr::SubmitTaskToFFRTQueue(std::function<void()>&& task, const std::string& taskName)
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (queue_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "queue is null, failed to submit task=%{public}s", taskName.c_str());
        return;
    }
    queue_->submit(task, ffrt::task_attr().name(taskName.c_str()));
}

int64_t AppEventObserverMgr::GetSeqFromWatchers(const std::string& name, std::string& filters)
{
    std::lock_guard<std::mutex> lock(watcherMutex_);
    for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
        if (it->second->GetName() == name) {
            filters = it->second->GetFiltersStr();
            return it->second->GetSeq();
        }
    }
    return -1;
}

int64_t AppEventObserverMgr::GetSeqFromProcessors(const std::string& name, int64_t hashCode)
{
    std::lock_guard<std::mutex> lock(processorMutex_);
    for (auto it = processors_.cbegin(); it != processors_.cend(); ++it) {
        if (it->second->GetName() == name && it->second->GenerateHashCode() == hashCode) {
            return it->second->GetSeq();
        }
    }
    return -1;
}

void AppEventObserverMgr::DeleteWatcher(int64_t observerSeq)
{
    std::lock_guard<std::mutex> lock(watcherMutex_);
    watchers_.erase(observerSeq);
    UnregisterOsEventListener();
}

void AppEventObserverMgr::DeleteProcessor(int64_t observerSeq)
{
    std::lock_guard<std::mutex> lock(processorMutex_);
    processors_.erase(observerSeq);
}

bool AppEventObserverMgr::IsExistInWatchers(int64_t observerSeq)
{
    std::lock_guard<std::mutex> lock(watcherMutex_);
    return watchers_.find(observerSeq) != watchers_.cend();
}

bool AppEventObserverMgr::IsExistInProcessors(int64_t observerSeq)
{
    std::lock_guard<std::mutex> lock(processorMutex_);
    return processors_.find(observerSeq) != processors_.cend();
}

std::vector<std::shared_ptr<AppEventObserver>> AppEventObserverMgr::GetObservers()
{
    std::vector<std::shared_ptr<AppEventObserver>> observers;
    {
        std::lock_guard<std::mutex> watcherLock(watcherMutex_);
        for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
            observers.emplace_back(it->second);
        }
    }
    {
        std::lock_guard<std::mutex> processorLock(processorMutex_);
        for (auto it = processors_.cbegin(); it != processors_.cend(); ++it) {
            observers.emplace_back(it->second);
        }
    }
    return observers;
}

void AppEventObserverMgr::InitWatchers()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&]() {
        std::vector<Observer> observers;
        if (AppEventStore::GetInstance().QueryWatchers(observers) != 0) {
            HILOG_WARN(LOG_CORE, "failed to query observers from db");
            return;
        }
        std::lock_guard<std::mutex> lock(watcherMutex_);
        for (const auto& observer : observers) {
            auto watcherPtr = std::make_shared<AppEventWatcher>(observer.name);
            watcherPtr->SetSeq(observer.seq);
            watcherPtr->SetFiltersStr(observer.filters);
            watchers_[observer.seq] = watcherPtr;
        }
        HILOG_INFO(LOG_CORE, "init watchers");
    });
}

void AppEventObserverMgr::InitWatcherFromCache(std::shared_ptr<AppEventWatcher> watcher, bool& isExist)
{
    std::string name = watcher->GetName();
    std::string filters = watcher->GetFiltersStr();
    std::string historyFilters;
    if (int64_t seq = GetSeqFromWatchers(name, historyFilters); seq > 0) {
        isExist = true;
        watcher->SetSeq(seq);
        if (historyFilters != filters && AppEventStore::GetInstance().UpdateObserver(seq, filters) < 0) {
            HILOG_ERROR(LOG_CORE, "failed to update watcher=%{public}s to db", name.c_str());
        }
    }
}

int64_t AppEventObserverMgr::AddWatcher(std::shared_ptr<AppEventWatcher> watcher)
{
    InitWatchers();
    bool isExist = false;
    InitWatcherFromCache(watcher, isExist);
    std::string filters = watcher->GetFiltersStr();
    int64_t observerSeq = InitObserverFromDb(watcher, filters);
    if (observerSeq <= 0) {
        return -1;
    }
    std::lock_guard<std::mutex> lock(watcherMutex_);
    if (!InitWatcherFromListener(watcher, isExist)) {
        return -1;
    }
    watchers_[observerSeq] = watcher;
    HILOG_INFO(LOG_CORE, "register watcher=%{public}" PRId64 " successfully", observerSeq);
    return observerSeq;
}

int64_t AppEventObserverMgr::AddProcessorWithTimeLimited(const std::string& name, int64_t hashCode,
    std::shared_ptr<AppEventProcessorProxy> processor)
{
    if (isFirstAddProcessor_) {
        isFirstAddProcessor_ = false;
        auto syncPromise = std::make_shared<ffrt::promise<int64_t>>();
        if (syncPromise == nullptr) {
            HILOG_ERROR(LOG_CORE, "Failed to create syncPromise.");
            return -1;
        }
        ffrt::future syncFuture = syncPromise->get_future();
        ffrt::submit([this, syncPromise, processor, name, hashCode]() {
            processor->SetSeq(AppEventStore::GetInstance().QueryObserverSeq(name, hashCode));
            this->isDbInit_ = true;
            syncPromise->set_value(InitObserverFromDb(processor, "", hashCode));
            }, ffrt::task_attr()
                .name("ADD_PROCESSOR_TASK")
                .qos(static_cast<int>(ffrt_qos_user_initiated)));
        ffrt::future_status wait = syncFuture.wait_for(std::chrono::milliseconds(TIMEOUT_LIMIT_FOR_ADDPROCESSOR));
        if (wait != ffrt::future_status::ready) {
            HILOG_WARN(LOG_CORE, "AddProcessor task execution timeout");
            return -1;
        }
        return syncFuture.get();
    }
    int remainTime = TIMEOUT_LIMIT_FOR_ADDPROCESSOR;
    while (!isDbInit_ && remainTime >= 0) {
        remainTime -= CHECK_DB_INTERVAL;
        std::this_thread::sleep_for(std::chrono::milliseconds(CHECK_DB_INTERVAL));
    }
    if (remainTime < 0) {
        HILOG_WARN(LOG_CORE, "AddProcessor task execution timeout");
        return -1;
    }
    processor->SetSeq(AppEventStore::GetInstance().QueryObserverSeq(name, hashCode));
    return InitObserverFromDb(processor, "", hashCode);
}

int64_t AppEventObserverMgr::AddProcessor(const std::string& name, const ReportConfig& config)
{
    if (name.empty()) {
        HILOG_WARN(LOG_CORE, "processor name is empty");
        return -1;
    }

    // build processor object
    auto processor = moduleLoader_->CreateProcessorProxy(name);
    if (processor == nullptr) {
        HILOG_WARN(LOG_CORE, "processor is null");
        return -1;
    }
    processor->SetReportConfig(config);

    int64_t hashCode = processor->GenerateHashCode();
    if (int64_t seq = GetSeqFromProcessors(name, hashCode); seq > 0) {
        HILOG_INFO(LOG_CORE, "register processor=%{public}" PRId64 " exit", seq);
        return seq;
    }

    int64_t observerSeq = AddProcessorWithTimeLimited(name, hashCode, processor);
    if (observerSeq <= 0) {
        return -1;
    }
    processor->ProcessStartup();
    std::lock_guard<std::mutex> lock(processorMutex_);
    processors_[observerSeq] = processor;
    HILOG_INFO(LOG_CORE, "register processor=%{public}" PRId64 " successfully", observerSeq);
    return observerSeq;
}

int AppEventObserverMgr::RemoveObserver(int64_t observerSeq)
{
    if (!IsExistInWatchers(observerSeq) && !IsExistInProcessors(observerSeq)) {
        HILOG_WARN(LOG_CORE, "observer seq=%{public}" PRId64 " is not exist", observerSeq);
        return 0;
    }
    if (int ret = AppEventStore::GetInstance().DeleteObserver(observerSeq); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to unregister observer seq=%{public}" PRId64, observerSeq);
        return ret;
    }
    DeleteProcessor(observerSeq);
    DeleteWatcher(observerSeq);
    HILOG_INFO(LOG_CORE, "unregister observer seq=%{public}" PRId64 " successfully", observerSeq);
    return 0;
}

int AppEventObserverMgr::RemoveObserver(const std::string& observerName)
{
    std::vector<int64_t> deleteSeqs;
    if (int ret = AppEventStore::GetInstance().QueryObserverSeqs(observerName, deleteSeqs); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to query observer=%{public}s seqs", observerName.c_str());
        return ret;
    }
    int ret = 0;
    for (auto deleteSeq : deleteSeqs) {
        if (int tempRet = RemoveObserver(deleteSeq); tempRet < 0) {
            HILOG_ERROR(LOG_CORE, "failed to unregister observer seq=%{public}" PRId64, deleteSeq);
            ret = tempRet;
        }
    }
    return ret;
}

int AppEventObserverMgr::Load(const std::string& moduleName)
{
    return moduleLoader_->Load(moduleName);
}

int AppEventObserverMgr::RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor)
{
    return moduleLoader_->RegisterProcessor(name, processor);
}

int AppEventObserverMgr::UnregisterProcessor(const std::string& name)
{
    return moduleLoader_->UnregisterProcessor(name);
}

void AppEventObserverMgr::HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    InitWatchers();
    auto observers = GetObservers();
    if (observers.empty() || events.empty()) {
        return;
    }
    HILOG_DEBUG(LOG_CORE, "start to handle events size=%{public}zu", events.size());
    StoreEventsToDb(events);
    StoreEventMappingToDb(events, observers);
    bool isNeedSend = false;
    for (const auto& observer : observers) {
        // send events to observer, and then delete events not in event mapping
        SendEventsToObserver(events, observer);
        isNeedSend |= observer->HasTimeoutCondition();
    }
    // timeout condition > 0 and the current event row > 0, send timeout task.
    // There can be only one timeout task.
    std::lock_guard<std::mutex> lock(isTimeoutTaskExistMutex_);
    if (isNeedSend && !isTimeoutTaskExist_) {
        SendTimeoutTask();
        isTimeoutTaskExist_ = true;
    }
}

void AppEventObserverMgr::HandleTimeout()
{
    auto observers = GetObservers();
    bool isNeedSend = false;
    for (const auto& observer : observers) {
        observer->ProcessTimeout();
        isNeedSend |= observer->HasTimeoutCondition();
    }
    if (isNeedSend) {
        SendTimeoutTask();
    } else {
        // when the current event row <= 0, do not send timeout task.
        std::lock_guard<std::mutex> lock(isTimeoutTaskExistMutex_);
        isTimeoutTaskExist_ = false;
    }
}

void AppEventObserverMgr::SendTimeoutTask()
{
    ffrt::submit([this] {
        HandleTimeout();
        }, ffrt::task_attr().name("appevent_timeout").delay(TIMEOUT_INTERVAL_MILLI * MILLI_TO_MICRO));
}

void AppEventObserverMgr::SendRefreshFreeSizeTask()
{
    ffrt::submit([this] {
        HiAppEventConfig::GetInstance().RefreshFreeSize();
        SendRefreshFreeSizeTask();
        }, ffrt::task_attr().name("appevent_refresh").delay(REFRESH_FREE_SIZE_INTERVAL * MILLI_TO_MICRO));
}

void AppEventObserverMgr::HandleBackground()
{
    HILOG_INFO(LOG_CORE, "start to handle background");
    SubmitTaskToFFRTQueue([this] {
        auto observers = GetObservers();
        for (const auto& observer : observers) {
            observer->ProcessBackground();
        }
        }, "app_background");
}

void AppEventObserverMgr::HandleClearUp()
{
    HILOG_INFO(LOG_CORE, "start to handle clear up");
    auto observers = GetObservers();
    for (const auto& observer : observers) {
        observer->ResetCurrCondition();
    }
}

int AppEventObserverMgr::SetReportConfig(int64_t observerSeq, const ReportConfig& config)
{
    std::lock_guard<std::mutex> lock(processorMutex_);
    if (processors_.find(observerSeq) == processors_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to set config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    processors_[observerSeq]->SetReportConfig(config);
    return 0;
}

int AppEventObserverMgr::GetReportConfig(int64_t observerSeq, ReportConfig& config)
{
    std::lock_guard<std::mutex> lock(processorMutex_);
    if (processors_.find(observerSeq) == processors_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to get config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    config = processors_[observerSeq]->GetReportConfig();
    return 0;
}

bool AppEventObserverMgr::InitWatcherFromListener(std::shared_ptr<AppEventWatcher> watcher, bool isExist)
{
    uint64_t mask = watcher->GetOsEventsMask();
    if (mask == 0) {
        return true;
    }
    if (listener_ == nullptr) {
        listener_ = std::make_shared<OsEventListener>();
        if (!listener_->StartListening()) {
            return false;
        }
    }
    if (!listener_->AddListenedEvents(mask)) {
        return false;
    }
    if (isExist) {
        std::vector<std::shared_ptr<AppEventPack>> events;
        listener_->GetEvents(events);
        StoreEventMappingToDb(events, {watcher});
        SendEventsToObserver(events, watcher);
    }
    return true;
}

void AppEventObserverMgr::UnregisterOsEventListener()
{
    if (listener_ == nullptr) {
        return;
    }
    uint64_t mask = 0;
    for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
        mask |= it->second->GetOsEventsMask();
    }
    if (mask > 0) {
        listener_->SetListenedEvents(mask);
        return;
    }
    listener_->RemoveOsEventDir();
    listener_ = nullptr;
}
} // namespace HiviewDFX
} // namespace OHOS
