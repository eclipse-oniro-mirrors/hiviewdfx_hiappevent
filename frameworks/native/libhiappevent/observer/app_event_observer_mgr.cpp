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
#include "module_loader.h"
#include "os_event_listener.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventObserverMgr"

namespace OHOS {
namespace HiviewDFX {
using HiAppEvent::AppEventFilter;
using HiAppEvent::TriggerCondition;
namespace {
constexpr int TIMEOUT_INTERVAL = 1000; // 1s

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

int64_t StoreEventMappingToDb(int64_t eventSeq, int64_t observerSeq)
{
    return AppEventStore::GetInstance().InsertEventMapping(eventSeq, observerSeq);
}

int64_t InitObserverFromDb(std::shared_ptr<AppEventObserver> observer,
    const std::string& name, int64_t hashCode)
{
    int64_t observerSeq = AppEventStore::GetInstance().QueryObserverSeq(name, hashCode);
    if (observerSeq <= 0) {
        HILOG_INFO(LOG_CORE, "the observer does not exist in database, name=%{public}s, hash=%{public}" PRId64,
            name.c_str(), hashCode);
        return -1;
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq) < 0) {
        HILOG_ERROR(LOG_CORE, "failed to take events, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    if (!events.empty()) {
        TriggerCondition triggerCond;
        for (auto event : events) {
            triggerCond.row++;
            triggerCond.size += static_cast<int>(event->GetEventStr().size());
        }
        observer->SetCurrCondition(triggerCond);
    }
    return observerSeq;
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
            int64_t observerSeq = observer->GetSeq();
            if (StoreEventMappingToDb(event->GetSeq(), observerSeq) < 0) {
                HILOG_WARN(LOG_CORE, "failed to add mapping record to db, seq=%{public}" PRId64, observerSeq);
                return;
            }
            observer->ProcessEvent(event);
        }
    }
    if (!realTimeEvents.empty()) {
        observer->OnEvents(realTimeEvents);
    }
}
}
std::mutex AppEventObserverMgr::instanceMutex_;

AppEventObserverMgr& AppEventObserverMgr::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    static AppEventObserverMgr instance;
    return instance;
}

AppEventObserverMgr::AppEventObserverMgr()
{
    CreateEventHandler();
    RegisterAppStateCallback();
}

void AppEventObserverMgr::CreateEventHandler()
{
    auto runner = AppExecFwk::EventRunner::Create("OS_AppEvent");
    if (runner == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to create event runner");
        return;
    }
    handler_ = std::make_shared<AppEventHandler>(runner);
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
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
    handler_= nullptr;
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

int64_t AppEventObserverMgr::RegisterObserver(std::shared_ptr<AppEventObserver> observer)
{
    int64_t observerSeq = InitObserver(observer);
    if (observerSeq <= 0) {
        return observerSeq;
    }

    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    observers_[observerSeq] = observer;
    HILOG_INFO(LOG_CORE, "register observer=%{public}" PRId64 " successfully", observerSeq);
    return observerSeq;
}

int64_t AppEventObserverMgr::RegisterObserver(const std::string& observerName, const ReportConfig& config)
{
    if (observerName.empty()) {
        HILOG_WARN(LOG_CORE, "observer name is empty");
        return -1;
    }

    auto observer = HiAppEvent::ModuleLoader::GetInstance().CreateProcessorProxy(observerName);
    if (observer == nullptr) {
        HILOG_WARN(LOG_CORE, "observer is null");
        return -1;
    }
    observer->SetReportConfig(config);

    int64_t observerSeq = RegisterObserver(observer);
    if (observerSeq <= 0) {
        return -1;
    }
    observer->ProcessStartup();
    return observerSeq;
}

int AppEventObserverMgr::UnregisterObserver(int64_t observerSeq)
{
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
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

int64_t AppEventObserverMgr::InitObserver(std::shared_ptr<AppEventObserver> observer)
{
    std::string observerName = observer->GetName();
    int64_t observerHashCode = observer->GenerateHashCode();
    int64_t observerSeq = InitObserverFromDb(observer, observerName, observerHashCode);
    bool sendFlag = false;
    if (observerSeq <= 0) {
        observerSeq = AppEventStore::GetInstance().InsertObserver(observerName, observerHashCode);
        if (observerSeq <= 0) {
            HILOG_ERROR(LOG_CORE, "failed to insert observer=%{public}s to db", observerName.c_str());
            return -1;
        }
    } else {
        sendFlag = true;
    }
    observer->SetSeq(observerSeq);

    if (!InitObserverFromListener(observer, sendFlag)) {
        return -1;
    }
    return observerSeq;
}

void AppEventObserverMgr::HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    if (observers_.empty()) {
        return;
    }
    HILOG_INFO(LOG_CORE, "start to handle events");
    StoreEventsToDb(events);
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        SendEventsToObserver(events, it->second);
    }
}

void AppEventObserverMgr::HandleTimeout()
{
    if (handler_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to handle timeOut: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        it->second->ProcessTimeout();
    }
}

void AppEventObserverMgr::HandleBackground()
{
    HILOG_INFO(LOG_CORE, "start to handle background");
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        it->second->ProcessBackground();
    }
}

void AppEventObserverMgr::HandleClearUp()
{
    HILOG_INFO(LOG_CORE, "start to handle clear up");
    std::lock_guard<ffrt::mutex> lock(observerMutex_);
    for (auto it = observers_.cbegin(); it != observers_.cend(); ++it) {
        it->second->ResetCurrCondition();
    }
}

int AppEventObserverMgr::SetReportConfig(int64_t observerSeq, const ReportConfig& config)
{
    if (observers_.find(observerSeq) == observers_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to set config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    observers_[observerSeq]->SetReportConfig(config);
    return 0;
}

int AppEventObserverMgr::GetReportConfig(int64_t observerSeq, ReportConfig& config)
{
    if (observers_.find(observerSeq) == observers_.cend()) {
        HILOG_WARN(LOG_CORE, "failed to get config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    config = observers_[observerSeq]->GetReportConfig();
    return 0;
}

bool AppEventObserverMgr::InitObserverFromListener(std::shared_ptr<AppEventObserver> observer, bool sendFlag)
{
    if (!observer->HasOsDomain()) {
        return true;
    }
    if (listener_ == nullptr) {
        listener_ = std::make_shared<OsEventListener>();
        if (!listener_->StartListening()) {
            return false;
        }
    }
    if (sendFlag) {
        std::vector<std::shared_ptr<AppEventPack>> events;
        listener_->GetEvents(events);
        StoreEventsToDb(events);
        SendEventsToObserver(events, observer);
    }
    return true;
}

void AppEventObserverMgr::UnregisterOsEventListener()
{
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        if (it->second->HasOsDomain()) {
            return;
        }
    }
    if (listener_ != nullptr) {
        listener_->RemoveOsEventDir();
        listener_ = nullptr;
    }
}
} // namespace HiviewDFX
} // namespace OHOS
