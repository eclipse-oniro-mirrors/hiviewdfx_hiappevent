/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ndk_app_event_watcher.h"
#include "app_event_observer_mgr.h"
#include "hilog/log.h"
#include "hiappevent_base.h"
#include "app_event_store.h"

namespace OHOS {
namespace HiviewDFX {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Ndk_HiAppEvent_Watcher" };

NdkAppEventWatcher::NdkAppEventWatcher(const std::string &name)
    : observer_(std::make_shared<NdkAppEventObserver>(name)) {}

NdkAppEventWatcher::~NdkAppEventWatcher()
{
    if (isRegistered) {
        AppEventObserverMgr::GetInstance().UnregisterObserver(observer_->GetSeq());
        isRegistered = false;
    }
}

int NdkAppEventWatcher::SetTriggerCondition(uint32_t row, uint32_t size, uint32_t timeOut)
{
    observer_->SetTriggerCondition(row, size, timeOut);
    return 0;
}

int NdkAppEventWatcher::SetAppEventFilter(const char *domain, uint8_t eventTypes,
                                          const char *const *names, int namesLen)
{
    return observer_->AddAppEventFilter(domain, eventTypes, names, namesLen);
}

int NdkAppEventWatcher::SetWatcherOnTrigger(OH_HiAppEvent_OnTrigger onTrigger)
{
    observer_->SetOnTrigger(onTrigger);
    return 0;
}

int NdkAppEventWatcher::SetWatcherOnReceiver(OH_HiAppEvent_OnReceive onReceiver)
{
    observer_->SetOnOnReceive(onReceiver);
    return 0;
}

int NdkAppEventWatcher::AddWatcher()
{
    AppEventObserverMgr::GetInstance().RegisterObserver(observer_);
    isRegistered = true;
    return 0;
}

int NdkAppEventWatcher::TakeWatcherData(uint32_t size, OH_HiAppEvent_OnTake onTake)
{
    if (!isRegistered) {
        HiLog::Warn(LABEL, "failed to query events, the observer has not been added");
        return ErrorCode::ERROR_OUT_OF_SEQUENCE;
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    if (AppEventStore::GetInstance().TakeEvents(events, observer_->GetSeq(), size) != 0) {
        HiLog::Warn(LABEL, "failed to query events, seq=%{public}" PRId64, observer_->GetSeq());
        return ErrorCode::ERROR_UNKNOWN;
    }
    std::vector<std::string> eventStrs(events.size());
    std::vector<const char*> retEvents;
    for (size_t t = 0; t < events.size(); ++t) {
        eventStrs[t] = events[t]->GetEventStr();
        retEvents.emplace_back(eventStrs[t].c_str());
    }
    onTake(retEvents.data(), static_cast<int32_t>(eventStrs.size()));
    return 0;
}

int NdkAppEventWatcher::RemoveWatcher()
{
    if (isRegistered) {
        AppEventObserverMgr::GetInstance().UnregisterObserver(observer_->GetSeq());
        isRegistered = false;
        return 0;
    }
    return ErrorCode::ERROR_OUT_OF_SEQUENCE;
}
}
}