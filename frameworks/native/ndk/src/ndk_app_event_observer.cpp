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

#include "ndk_app_event_observer.h"
#include "hilog/log.h"
#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Ndk_HiAppEvent_Watcher" };

NdkAppEventObserver::NdkAppEventObserver(const std::string &name) : HiAppEvent::AppEventObserver(name) {}

void NdkAppEventObserver::SetTriggerCondition(uint32_t row, uint32_t size, uint32_t timeOut)
{
    reportConfig_.triggerCond = {static_cast<int>(row), static_cast<int>(size), static_cast<int>(timeOut)};
}

int NdkAppEventObserver::AddAppEventFilter(const char* domain, uint8_t eventTypes,
    const char *const *names, int namesLen)
{
    if (!domain) {
        return ErrorCode::ERROR_NULL_POINT;
    }
    HiAppEvent::AppEventFilter filter{domain, eventTypes};
    if (!names && namesLen > 0) {
        return ErrorCode::ERROR_NULL_POINT;
    }
    for (int i = 0; i < namesLen; ++i) {
        if (!names[i]) {
            return ErrorCode::ERROR_NULL_POINT;
        }
        filter.names.insert(names[i]);
    }
    filters_.emplace_back(std::move(filter));
    return 0;
}

void NdkAppEventObserver::SetOnTrigger(OH_HiAppEvent_OnTrigger onTrigger)
{
    onTrigger_ = onTrigger;
}

void NdkAppEventObserver::SetOnOnReceive(OH_HiAppEvent_OnReceive onReceive)
{
    onReceive_ = onReceive;
}

void NdkAppEventObserver::OnEvents(const std::vector<std::shared_ptr<AppEventPack>> &events)
{
    if (events.empty() || !onReceive_) {
        return;
    }
    std::unordered_map<std::string, std::vector<HiAppEvent_AppEventInfo>> eventMap;
    constexpr size_t strNumPieceEvent = 3;
    std::vector<std::string> strings(strNumPieceEvent * events.size());
    size_t strIndex = 0;
    for (const auto &event : events) {
        auto& appEventInfo = eventMap[event->GetName()].emplace_back();
        strings[strIndex] = event->GetDomain();
        appEventInfo.domain = strings[strIndex++].c_str();
        strings[strIndex] = event->GetName();
        appEventInfo.name = strings[strIndex++].c_str();
        strings[strIndex] = event->GetParamStr();
        appEventInfo.params = strings[strIndex++].c_str();
        appEventInfo.type = EventType(event->GetType());
    }
    std::vector<HiAppEvent_AppEventGroup> appEventGroup(eventMap.size());
    uint32_t appEventIndex = 0;
    for (const auto &[k, v] : eventMap) {
        appEventGroup[appEventIndex].name = k.c_str();
        appEventGroup[appEventIndex].appEventInfos = v.data();
        appEventGroup[appEventIndex].infoLen = v.size();
        appEventIndex++;
    }
    std::string domain = events[0]->GetDomain();
    onReceive_(domain.c_str(), appEventGroup.data(), static_cast<uint32_t>(eventMap.size()));
}

void NdkAppEventObserver::OnTrigger(const HiAppEvent::TriggerCondition &triggerCond)
{
    HiLog::Debug(LABEL, "onTrigger start");
    if (!onTrigger_) {
        HiLog::Warn(LABEL, "onTrigger_ is nullptr");
        return;
    }
    onTrigger_(triggerCond.row, triggerCond.size);
}

bool NdkAppEventObserver::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    return onReceive_ != nullptr;
}
}
}