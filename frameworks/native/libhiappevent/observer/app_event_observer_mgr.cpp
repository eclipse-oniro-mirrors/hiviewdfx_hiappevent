/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "app_event_handler.h"
#include "app_event_processor_proxy.h"
#include "app_event_store.h"
#include "application_context.h"
#include "hiappevent_base.h"
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
namespace {
constexpr int TIMEOUT_INTERVAL = HiAppEvent::TIMEOUT_STEP * 1000; // 30s
constexpr int ONE_MINUTE = 60 * 1000;
constexpr int REFRESH_FREE_SIZE_INTERVAL = 10 * ONE_MINUTE; // 10 minutes
constexpr int MAX_SIZE_OF_INIT = 100;

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
    std::shared_ptr<AppEventObserver> observer)
{
    for (const auto& event : events) {
        if (observer->VerifyEvent(event)) {
            int64_t observerSeq = observer->GetSeq();
            if (observerSeq == 0) {
                HILOG_INFO(LOG_CORE, "observer=%{public}s seq not set", observer->GetName().c_str());
                continue;
            }
            if (AppEventStore::GetInstance().InsertEventMapping(event->GetSeq(), observerSeq) < 0) {
                HILOG_ERROR(LOG_CORE, "failed to add mapping record to db, seq=%{public}" PRId64, observerSeq);
            }
        }
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

int64_t InitObserverFromDb(std::shared_ptr<AppEventObserver> observer, const std::string& name, int64_t hashCode)
{
    std::string filters;
    int64_t observerSeq = AppEventStore::GetInstance().QueryObserverSeq(name, hashCode, filters);
    if (observerSeq <= 0) {
        HILOG_INFO(LOG_CORE, "the observer does not exist in database, name=%{public}s, hash=%{public}" PRId64,
            name.c_str(), hashCode);
        return -1;
    }
    std::string newFilters = observer->GetFiltersStr();
    if (filters != newFilters && AppEventStore::GetInstance().UpdateObserver(observerSeq, newFilters) <= 0) {
        HILOG_ERROR(LOG_CORE, "failed to update observer=%{public}s to db", name.c_str());
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq, MAX_SIZE_OF_INIT) < 0) {
        HILOG_ERROR(LOG_CORE, "failed to take events, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    observer->SetSeq(observerSeq);
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
            observer->ProcessStartup();
        }
    }
    return observerSeq;
}

int64_t InitWatcher(std::shared_ptr<AppEventObserver> observer, bool& isExist)
{
    std::string observerName = observer->GetName();
    int64_t observerHashCode = observer->GenerateHashCode();
    int64_t observerSeq = InitObserverFromDb(observer, observerName, observerHashCode);
    if (observerSeq <= 0) {
        observerSeq = AppEventStore::GetInstance().InsertObserver(observerName, observerHashCode,
            observer->GetFiltersStr());
        if (observerSeq <= 0) {
            HILOG_ERROR(LOG_CORE, "failed to insert watcher=%{public}s to db", observerName.c_str());
            return -1;
        }
    } else {
        isExist = true;
    }
    observer->SetSeq(observerSeq);
    return observerSeq;
}

void InitProcessor(std::shared_ptr<AppEventObserver> observer, int64_t observerHashCode)
{
    ffrt::submit([observer, observerHashCode]() {
        std::string observerName = observer->GetName();
        int64_t seq = InitObserverFromDb(observer, observerName, observerHashCode);
        if (seq > 0) {
            HILOG_INFO(LOG_CORE, "processor=%{public}s has inserted", observerName.c_str());
            return;
        }
        seq = AppEventStore::GetInstance().InsertObserver(observerName, observerHashCode, observer->GetFiltersStr());
        if (seq <= 0) {
            HILOG_ERROR(LOG_CORE, "failed to insert processor, name=%{public}s to db", observerName.c_str());
            return;
        }
        HILOG_INFO(LOG_CORE, "processor=%{public}s insert success", observerName.c_str());
        observer->SetSeq(seq);
        }, {}, {}, ffrt::task_attr().name("insert_observer").qos(ffrt::qos_user_initiated));
}

bool HandleEventsByMap(std::vector<std::shared_ptr<AppEventPack>>& events,
    std::unordered_map<int64_t, std::shared_ptr<AppEventObserver>>& observers)
{
    for (auto it = observers.cbegin(); it != observers.cend(); ++it) {
        StoreEventMappingToDb(events, it->second);
    }
    bool needSend = false;
    for (auto it = observers.cbegin(); it != observers.cend(); ++it) {
        // send events to observer, and then delete events not in event mapping
        SendEventsToObserver(events, it->second);
        needSend |= it->second->HasTimeoutCondition();
    }
    return needSend;
}
}

AppEventObserverMgr& AppEventObserverMgr::GetInstance()
{
    static AppEventObserverMgr instance;
    return instance;
}

AppEventObserverMgr::AppEventObserverMgr()
{
    CreateEventHandler();
    SendRefreshFreeSizeEvent();
    RegisterAppStateCallback();
    moduleLoader_ = std::make_unique<ModuleLoader>();
}

void AppEventObserverMgr::CreateEventHandler()
{
    auto runner = AppExecFwk::EventRunner::Create("OS_AppEvent_Hd", AppExecFwk::ThreadMode::FFRT);
    if (runner == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to create event runner");
        return;
    }
    handler_ = std::make_shared<AppEventHandler>(runner);
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
    DestroyEventHandler();
    UnregisterAppStateCallback();
}

void AppEventObserverMgr::DestroyEventHandler()
{
    if (handler_ != nullptr) {
        HILOG_INFO(LOG_CORE, "start to TaskCancelAndWait");
        // stop and wait task
        handler_->TaskCancelAndWait();
    }
    std::lock_guard<ffrt::mutex> lock(handlerMutex_);
    handler_ = nullptr;
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

void AppEventObserverMgr::InitWatchers()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&]() {
        std::vector<AppEventCacheCommon::Observer> observers;
        if (AppEventStore::GetInstance().QueryWatchers(observers) != 0) {
            HILOG_WARN(LOG_CORE, "failed to query observers from db");
            return;
        }
        std::lock_guard<ffrt::mutex> lock(observerMutex_);
        for (const auto& observer : observers) {
            auto observerPtr = std::make_shared<AppEventObserver>(observer.name);
            observerPtr->SetSeq(observer.seq);
            observerPtr->SetFilters(observer.filters);
            watchers_[observer.seq] = observerPtr;
        }
        HILOG_INFO(LOG_CORE, "init watchers");
    });
}

int64_t AppEventObserverMgr::RegisterObserver(std::shared_ptr<AppEventObserver> observer)
{
    InitWatchers();
    bool isExist = false;
    int64_t observerSeq = InitWatcher(observer, isExist);
    if (observerSeq <= 0) {
        return observerSeq;
    }

    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    if (!InitObserverFromListener(observer, isExist)) {
        return -1;
    }
    watchers_[observerSeq] = observer;
    HILOG_INFO(LOG_CORE, "register observer=%{public}" PRId64 " successfully", observerSeq);
    return observerSeq;
}

int64_t AppEventObserverMgr::RegisterObserver(const std::string& observerName, const ReportConfig& config)
{
    if (observerName.empty()) {
        HILOG_WARN(LOG_CORE, "observer name is empty");
        return -1;
    }

    auto observer = moduleLoader_->CreateProcessorProxy(observerName);
    if (observer == nullptr) {
        HILOG_WARN(LOG_CORE, "observer is null");
        return -1;
    }
    observer->SetReportConfig(config);
    int64_t observerHashCode = observer->GenerateHashCode();
    InitProcessor(observer, observerHashCode);
    if (observerHashCode == 0) {
        return -1;
    }
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    processors_[observerHashCode] = observer;
    return observerHashCode;
}

int AppEventObserverMgr::UnregisterObserver(int64_t observerSeq, ObserverType type)
{
    bool isWatcher = (type == ObserverType::WATCHER);
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    if ((isWatcher && watchers_.find(observerSeq) == watchers_.cend()) ||
        (!isWatcher && processors_.find(observerSeq) == processors_.cend())) {
        HILOG_WARN(LOG_CORE, "observer seq=%{public}" PRId64 " is not exist", observerSeq);
        return 0;
    }
    if (int ret = AppEventStore::GetInstance().DeleteObserver(observerSeq, type); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to unregister observer seq=%{public}" PRId64, observerSeq);
        return ret;
    }
    if (isWatcher) {
        watchers_.erase(observerSeq);
        UnregisterOsEventListener();
    } else {
        processors_.erase(observerSeq);
    }
    HILOG_INFO(LOG_CORE, "unregister observer seq=%{public}" PRId64 " successfully", observerSeq);
    return 0;
}

int AppEventObserverMgr::UnregisterObserver(const std::string& observerName, ObserverType type)
{
    std::vector<int64_t> deleteSeqs;
    if (int ret = AppEventStore::GetInstance().QueryObserverSeqs(observerName, deleteSeqs, type); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to query observer=%{public}s seqs", observerName.c_str());
        return ret;
    }
    int ret = 0;
    for (auto deleteSeq : deleteSeqs) {
        if (int tempRet = UnregisterObserver(deleteSeq, type); tempRet < 0) {
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
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    if ((watchers_.empty() && processors_.empty()) || events.empty()) {
        return;
    }
    HILOG_DEBUG(LOG_CORE, "start to handle events size=%{public}zu", events.size());
    StoreEventsToDb(events);
    bool needSend = HandleEventsByMap(events, watchers_);
    needSend |= HandleEventsByMap(events, processors_);
    if (needSend && !hasHandleTimeout_) {
        SendEventToHandler();
        hasHandleTimeout_ = true;
    }
}

void AppEventObserverMgr::HandleTimeout()
{
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    bool needSend = false;
    for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
        it->second->ProcessTimeout();
        needSend |= it->second->HasTimeoutCondition();
    }
    for (auto it = processors_.cbegin(); it != processors_.cend(); ++it) {
        it->second->ProcessTimeout();
        needSend |= it->second->HasTimeoutCondition();
    }
    if (needSend) {
        SendEventToHandler();
    } else {
        hasHandleTimeout_ = false;
    }
}

void AppEventObserverMgr::SendEventToHandler()
{
    std::lock_guard<ffrt::mutex> lock(handlerMutex_);
    if (handler_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to SendEventToHandler: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
}

void AppEventObserverMgr::SendRefreshFreeSizeEvent()
{
    std::lock_guard<ffrt::mutex> lock(handlerMutex_);
    if (handler_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to SendRefreshFreeSizeEvent: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::REFRESH_FREE_SIZE, 0, REFRESH_FREE_SIZE_INTERVAL);
}

void AppEventObserverMgr::HandleBackground()
{
    HILOG_INFO(LOG_CORE, "start to handle background");
    ffrt::submit([this] {
        std::lock_guard<ffrt::mutex> lock(observerMutex_);
        for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
            it->second->ProcessBackground();
        }
        for (auto it = processors_.cbegin(); it != processors_.cend(); ++it) {
            it->second->ProcessBackground();
        }
        }, {}, {}, ffrt::task_attr().name("app_background"));
}

void AppEventObserverMgr::HandleClearUp()
{
    HILOG_INFO(LOG_CORE, "start to handle clear up");
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
        it->second->ResetCurrCondition();
    }
    for (auto it = processors_.cbegin(); it != processors_.cend(); ++it) {
        it->second->ResetCurrCondition();
    }
}

int AppEventObserverMgr::SetReportConfig(int64_t observerSeq, const ReportConfig& config)
{
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    if (processors_.find(observerSeq) == processors_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to set config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    processors_[observerSeq]->SetReportConfig(config);
    return 0;
}

int AppEventObserverMgr::GetReportConfig(int64_t observerSeq, ReportConfig& config)
{
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    if (processors_.find(observerSeq) == processors_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to get config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    config = processors_[observerSeq]->GetReportConfig();
    return 0;
}

bool AppEventObserverMgr::InitObserverFromListener(std::shared_ptr<AppEventObserver> observer, bool isExist)
{
    uint64_t mask = observer->GetOsEventsMask();
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
        StoreEventMappingToDb(events, observer);
        SendEventsToObserver(events, observer);
    }
    return true;
}

void AppEventObserverMgr::UnregisterOsEventListener()
{
    if (listener_ == nullptr) {
        return;
    }
    uint64_t mask = 0;
    for (auto it = watchers_.begin(); it != watchers_.end(); ++it) {
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
