/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "app_event_handler.h"
#include "app_event_processor_proxy.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "module_loader.h"

namespace OHOS {
namespace HiviewDFX {
using HiAppEvent::AppEventFilter;
using HiAppEvent::TriggerCondition;
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_ObserverMgr" };
constexpr int TIMEOUT_INTERVAL = 1000; // 1s

int StoreEventToDb(std::shared_ptr<AppEventPack> event)
{
    return AppEventStore::GetInstance().InsertEvent(event);
}

int64_t StoreEventMappingToDb(int64_t eventSeq, int64_t observerSeq)
{
    return AppEventStore::GetInstance().InsertEventMapping(eventSeq, observerSeq);
}

int64_t GetObserverExistSeq(std::shared_ptr<AppEventObserver> observer)
{
    std::string observerName = observer->GetName();
    std::vector<int64_t> observerSeqs;
    if (AppEventStore::GetInstance().QueryObserverSeqs(observerName, observerSeqs) < 0) {
        HiLog::Error(LABEL, "failed to query observer=%{public}s seqs", observerName.c_str());
        return -1;
    }
    if (observerSeqs.empty()) {
        return -1;
    }
    return observerSeqs.back();
}

int64_t InitObserverSeq(std::shared_ptr<AppEventObserver> observer)
{
    int64_t observerSeq = GetObserverExistSeq(observer);
    if (observerSeq > 0) {
        std::vector<std::shared_ptr<AppEventPack>> events;
        if (AppEventStore::GetInstance().QueryEvents(events, observerSeq) < 0) {
            HiLog::Error(LABEL, "failed to take events, seq=%{public}" PRId64, observerSeq);
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
    } else {
        std::string observerName = observer->GetName();
        observerSeq = AppEventStore::GetInstance().InsertObserver(observerName);
        if (observerSeq <= 0) {
            HiLog::Error(LABEL, "failed to insert observer=%{public}s to db", observerName.c_str());
            return -1;
        }
    }
    observer->SetSeq(observerSeq);
    return observerSeq;
}
}

AppEventObserverMgr::AppEventObserverMgr()
{
    if (!CreateEventHandler()) {
        HiLog::Warn(LABEL, "failed to create handler");
        return;
    }
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
}

bool AppEventObserverMgr::CreateEventHandler()
{
    auto runner = AppExecFwk::EventRunner::Create("AppEventHandler");
    if (runner == nullptr) {
        HiLog::Error(LABEL, "failed to create event runner");
        return false;
    }
    handler_ = std::make_shared<AppEventHandler>(runner);
    return true;
}

AppEventObserverMgr::~AppEventObserverMgr()
{
    DestroyEventHandler();
}

void AppEventObserverMgr::DestroyEventHandler()
{
    handler_= nullptr;
}

int64_t AppEventObserverMgr::RegisterObserver(std::shared_ptr<AppEventObserver> observer)
{
    int64_t observerSeq = InitObserverSeq(observer);
    if (observerSeq <= 0) {
        return observerSeq;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    observers_[observerSeq] = observer;
    HiLog::Info(LABEL, "register observer=%{public}" PRId64 " successfully", observerSeq);
    return observerSeq;
}

int64_t AppEventObserverMgr::RegisterObserver(const std::string& observerName)
{
    if (observerName.empty()) {
        HiLog::Warn(LABEL, "observer name is empty");
        return -1;
    }

    auto observer = HiAppEvent::ModuleLoader::GetInstance().CreateProcessorProxy(observerName);
    if (observer == nullptr) {
        HiLog::Warn(LABEL, "observer is null");
        return -1;
    }
    return RegisterObserver(observer);
}

int AppEventObserverMgr::UnregisterObserver(int64_t observerSeq)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (observers_.find(observerSeq) == observers_.end()) {
        HiLog::Warn(LABEL, "observer seq=%{public}" PRId64 " is not exist", observerSeq);
        return 0;
    }
    if (int ret = AppEventStore::GetInstance().DeleteObserver(observerSeq); ret < 0) {
        HiLog::Error(LABEL, "failed to unregister observer seq=%{public}" PRId64, observerSeq);
        return ret;
    }
    observers_.erase(observerSeq);
    HiLog::Info(LABEL, "unregister observer seq=%{public}" PRId64 " successfully", observerSeq);
    return 0;
}

int AppEventObserverMgr::UnregisterObserver(const std::string& observerName)
{
    std::vector<int64_t> deleteSeqs;
    if (int ret = AppEventStore::GetInstance().QueryObserverSeqs(observerName, deleteSeqs); ret < 0) {
        HiLog::Error(LABEL, "failed to query observer=%{public}s seqs", observerName.c_str());
        return ret;
    }
    int ret = 0;
    for (auto deleteSeq : deleteSeqs) {
        if (int tempRet = UnregisterObserver(deleteSeq); tempRet < 0) {
            HiLog::Error(LABEL, "failed to unregister observer seq=%{public}" PRId64, deleteSeq);
            ret = tempRet;
        }
    }
    return ret;
}

void AppEventObserverMgr::HandleEvent(std::shared_ptr<AppEventPack> event)
{
    HiLog::Info(LABEL, "start to handle event");
    int64_t eventSeq = StoreEventToDb(event);
    if (eventSeq <= 0) {
        HiLog::Warn(LABEL, "failed to add event to db");
        return;
    }

    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        auto observer = it->second;
        if (!observer->VerifyEvent(event)) {
            continue;
        }

        if (observer->IsRealTimeEvent(event)) {
            observer->OnEvents({event});
        } else {
            int64_t observerSeq = observer->GetSeq();
            if (StoreEventMappingToDb(eventSeq, observerSeq) < 0) {
                HiLog::Warn(LABEL, "failed to add mapping record to db, seq=%{public}" PRId64, observerSeq);
                return;
            }
            observer->ProcessEvent(event);
        }
    }
}

void AppEventObserverMgr::HandleTimeOut()
{
    HiLog::Debug(LABEL, "start to handle timeout");
    if (handler_ == nullptr) {
        HiLog::Error(LABEL, "failed to handle timeOut: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        it->second->ProcessTimeout();
    }
}

void AppEventObserverMgr::HandleStartup()
{
    HiLog::Debug(LABEL, "start to handle startup");
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        it->second->ProcessStartup();
    }
}

void AppEventObserverMgr::HandleBackground()
{
    HiLog::Debug(LABEL, "start to handle background");
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        it->second->ProcessBackground();
    }
}

int AppEventObserverMgr::SetReportConfig(int64_t observerSeq, const ReportConfig& config)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    if (observers_.find(observerSeq) == observers_.end()) {
        HiLog::Warn(LABEL, "failed to set config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    observers_[observerSeq]->SetReportConfig(config);
    return 0;
}

int AppEventObserverMgr::GetReportConfig(int64_t observerSeq, ReportConfig& config)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    if (observers_.find(observerSeq) == observers_.end()) {
        HiLog::Warn(LABEL, "failed to get config, seq=%{public}" PRId64, observerSeq);
        return -1;
    }
    config = observers_[observerSeq]->GetReportConfig();
    return 0;
}
} // namespace HiviewDFX
} // namespace OHOS
