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

#include "ndk_app_event_watcher_proxy.h"

#include "app_event_observer_mgr.h"
#include "hilog/log.h"
#include "hiappevent_base.h"
#include "app_event_store.h"

namespace OHOS {
namespace HiviewDFX {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Ndk_HiAppEvent_Watcher" };

NdkAppEventWatcherProxy::NdkAppEventWatcherProxy(const std::string &name)
    : watcher_(std::make_shared<NdkAppEventWatcher>(name)) {}

NdkAppEventWatcherProxy::~NdkAppEventWatcherProxy()
{
    if (isRegistered_) {
        AppEventObserverMgr::GetInstance().UnregisterObserver(watcher_->GetSeq());
        isRegistered_ = false;
    }
}

int NdkAppEventWatcherProxy::SetTriggerCondition(int row, int size, int timeOut)
{
    watcher_->SetTriggerCondition(row, size, timeOut);
    return 0;
}

int NdkAppEventWatcherProxy::SetAppEventFilter(const char *domain, uint8_t eventTypes,
                                               const char *const *names, int namesLen)
{
    return watcher_->AddAppEventFilter(domain, eventTypes, names, namesLen);
}

int NdkAppEventWatcherProxy::SetWatcherOnTrigger(OH_HiAppEvent_OnTrigger onTrigger)
{
    watcher_->SetOnTrigger(onTrigger);
    return 0;
}

int NdkAppEventWatcherProxy::SetWatcherOnReceiver(OH_HiAppEvent_OnReceive onReceiver)
{
    watcher_->SetOnOnReceive(onReceiver);
    return 0;
}

int NdkAppEventWatcherProxy::AddWatcher()
{
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher_);
    isRegistered_ = true;
    return 0;
}

int NdkAppEventWatcherProxy::TakeWatcherData(uint32_t size, OH_HiAppEvent_OnTake onTake)
{
    if (!isRegistered_) {
        HiLog::Warn(LABEL, "failed to query events, the observer has not been added");
        return ErrorCode::ERROR_WATCHER_NOT_ADDED;
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    if (AppEventStore::GetInstance().TakeEvents(events, watcher_->GetSeq(), size) != 0) {
        HiLog::Warn(LABEL, "failed to query events, seq=%{public}" PRId64, watcher_->GetSeq());
        return ErrorCode::ERROR_UNKNOWN;
    }
    std::vector<std::string> eventStrs(events.size());
    std::vector<const char*> retEvents(events.size());
    for (size_t t = 0; t < events.size(); ++t) {
        eventStrs[t] = events[t]->GetEventStr();
        retEvents[t] = eventStrs[t].c_str();
    }
    onTake(retEvents.data(), static_cast<int32_t>(eventStrs.size()));
    return 0;
}

int NdkAppEventWatcherProxy::RemoveWatcher()
{
    if (isRegistered_) {
        AppEventObserverMgr::GetInstance().UnregisterObserver(watcher_->GetSeq());
        isRegistered_ = false;
        return 0;
    }
    return ErrorCode::ERROR_WATCHER_NOT_ADDED;
}
}
}