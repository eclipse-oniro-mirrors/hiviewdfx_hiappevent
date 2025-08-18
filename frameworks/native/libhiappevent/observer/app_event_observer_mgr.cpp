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
#include "app_event_handler.h"
#include "app_event_processor_proxy.h"
#include "app_event_store.h"
#include "application_context.h"
#include "ffrt_inner.h"
#include "hiappevent_base.h"
#include "hiappevent_ffrt.h"
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
constexpr int ONE_MINUTE = 60 * 1000;
constexpr int REFRESH_FREE_SIZE_INTERVAL = 10 * ONE_MINUTE; // 10 minutes
constexpr int TIMEOUT_INTERVAL_MICRO = HiAppEvent::TIMEOUT_STEP * 1000 * 1000; // 30s
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
    std::shared_ptr<AppEventObserver> observer)
{
    for (const auto& event : events) {
        if (observer->VerifyEvent(event)) {
            int64_t observerSeq = observer->GetSeq();
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

int64_t InitObserverFromDb(std::shared_ptr<AppEventObserver> observer,
    const std::string& name, int64_t hashCode)
{
    std::string filters;
    int64_t observerSeq = AppEventStore::GetInstance().QueryObserverSeqAndFilters(name, hashCode, filters);
    if (observerSeq <= 0) {
        HILOG_INFO(LOG_CORE, "the observer does not exist in database, name=%{public}s, hash=%{public}" PRId64,
            name.c_str(), hashCode);
        return -1;
    }
    std::string newFilters = observer->GetFiltersStr();
    if (filters != newFilters && AppEventStore::GetInstance().UpdateObserver(observerSeq, newFilters) < 0) {
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
        }
    }
    return observerSeq;
}

int64_t InitObserver(std::shared_ptr<AppEventObserver> observer, bool& isExist)
{
    std::string observerName = observer->GetName();
    int64_t observerHashCode = observer->GenerateHashCode();
    int64_t observerSeq = InitObserverFromDb(observer, observerName, observerHashCode);
    if (observerSeq <= 0) {
        observerSeq = AppEventStore::GetInstance().InsertObserver(AppEventCacheCommon::Observer(observerName,
            observerHashCode, observer->GetFiltersStr()));
        if (observerSeq <= 0) {
            HILOG_ERROR(LOG_CORE, "failed to insert observer=%{public}s to db", observerName.c_str());
            return -1;
        }
    } else {
        isExist = true;
    }
    observer->SetSeq(observerSeq);
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
    std::lock_guard<std::mutex> lock(handlerMutex_);
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
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (const auto& observer : observers) {
            auto observerPtr = std::make_shared<AppEventObserver>(observer.name);
            observerPtr->SetSeq(observer.seq);
            observerPtr->SetFilters(observer.filters);
            observers_[observer.seq] = observerPtr;
        }
        HILOG_INFO(LOG_CORE, "init watchers");
    });
}

int64_t AppEventObserverMgr::RegisterObserver(std::shared_ptr<AppEventObserver> observer)
{
    InitWatchers();
    bool isExist = false;
    int64_t observerSeq = InitObserver(observer, isExist);
    if (observerSeq <= 0) {
        return observerSeq;
    }

    std::lock_guard<std::mutex> lock(observerMutex_);
    if (!InitObserverFromListener(observer, isExist)) {
        return -1;
    }
    observers_[observerSeq] = observer;
    HILOG_INFO(LOG_CORE, "register observer=%{public}" PRId64 " successfully", observerSeq);
    return observerSeq;
}

int64_t AppEventObserverMgr::AddProcessorWithTimeLimited(std::shared_ptr<AppEventObserver> observer)
{
    if (isFirstAddProcessor_) {
        isFirstAddProcessor_ = false;
        auto syncPromise = std::make_shared<ffrt::promise<int64_t>>();
        if (syncPromise == nullptr) {
            HILOG_ERROR(LOG_CORE, "Failed to create syncPromise.");
            return -1;
        }
        ffrt::future syncFuture = syncPromise->get_future();
        ffrt::submit([this, syncPromise, observer]() {
            syncPromise->set_value(RegisterObserver(observer));
            this->isDbInit_ = true;
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
    return RegisterObserver(observer);
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

    int64_t observerSeq = AddProcessorWithTimeLimited(observer);
    if (observerSeq <= 0) {
        return -1;
    }
    observer->ProcessStartup();
    return observerSeq;
}

int AppEventObserverMgr::UnregisterObserver(int64_t observerSeq)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observers_.find(observerSeq) == observers_.cend()) {
        HILOG_WARN(LOG_CORE, "observer seq=%{public}" PRId64 " is not exist", observerSeq);
        return 0;
    }
    if (int ret = AppEventStore::GetInstance().DeleteObserver(observerSeq); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to unregister observer seq=%{public}" PRId64, observerSeq);
        return ret;
    }
    observers_.erase(observerSeq);
    UnregisterOsEventListener();
    HILOG_INFO(LOG_CORE, "unregister observer seq=%{public}" PRId64 " successfully", observerSeq);
    return 0;
}

int AppEventObserverMgr::UnregisterObserver(const std::string& observerName)
{
    std::vector<int64_t> deleteSeqs;
    if (int ret = AppEventStore::GetInstance().QueryObserverSeqs(observerName, deleteSeqs); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to query observer=%{public}s seqs", observerName.c_str());
        return ret;
    }
    int ret = 0;
    for (auto deleteSeq : deleteSeqs) {
        if (int tempRet = UnregisterObserver(deleteSeq); tempRet < 0) {
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
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observers_.empty() || events.empty()) {
        return;
    }
    HILOG_DEBUG(LOG_CORE, "start to handle events size=%{public}zu", events.size());
    StoreEventsToDb(events);
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        StoreEventMappingToDb(events, it->second);
    }
    bool needSend = false;
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        // send events to observer, and then delete events not in event mapping
        SendEventsToObserver(events, it->second);
        needSend |= it->second->HasTimeoutCondition();
    }
    if (needSend && !hasHandleTimeout_) {
        SendEventToHandler();
        hasHandleTimeout_ = true;
    }
}

void AppEventObserverMgr::HandleTimeout()
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    bool needSend = false;
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
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
    HiAppEvent::Submit([this] {
        ffrt::this_task::sleep_for(std::chrono::microseconds(TIMEOUT_INTERVAL_MICRO));
        HandleTimeout();
        }, {}, {}, ffrt::task_attr().name("appevent_timeout"));
}

void AppEventObserverMgr::SendRefreshFreeSizeEvent()
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    if (handler_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to SendRefreshFreeSizeEvent: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::REFRESH_FREE_SIZE, 0, REFRESH_FREE_SIZE_INTERVAL);
}

void AppEventObserverMgr::HandleBackground()
{
    HILOG_INFO(LOG_CORE, "start to handle background");
    HiAppEvent::Submit([this] {
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
            it->second->ProcessBackground();
        }
        }, {}, {}, ffrt::task_attr().name("app_background"));
}

void AppEventObserverMgr::HandleClearUp()
{
    HILOG_INFO(LOG_CORE, "start to handle clear up");
    std::lock_guard<std::mutex> lock(observerMutex_);
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        it->second->ResetCurrCondition();
    }
}

int AppEventObserverMgr::SetReportConfig(int64_t observerSeq, const ReportConfig& config)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observers_.find(observerSeq) == observers_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to set config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    observers_[observerSeq]->SetReportConfig(config);
    return 0;
}

int AppEventObserverMgr::GetReportConfig(int64_t observerSeq, ReportConfig& config)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observers_.find(observerSeq) == observers_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to get config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    config = observers_[observerSeq]->GetReportConfig();
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
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
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
