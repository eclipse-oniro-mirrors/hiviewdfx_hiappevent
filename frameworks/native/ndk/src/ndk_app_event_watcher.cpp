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

#include "hilog/log.h"
#include "hiappevent_base.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NdkAppEventWatcher"

namespace OHOS {
namespace HiviewDFX {

NdkAppEventWatcher::NdkAppEventWatcher(const std::string &name) : AppEventWatcher(name) {}

void NdkAppEventWatcher::SetTriggerCondition(int row, int size, int timeOut)
{
    reportConfig_.triggerCond = {row, size, timeOut};
}

int NdkAppEventWatcher::AddAppEventFilter(const char* domain, uint8_t eventTypes,
                                          const char *const *names, int namesLen)
{
    if (domain == nullptr) {
        return ErrorCode::ERROR_INVALID_EVENT_DOMAIN;
    }
    uint32_t types = eventTypes << 1;
    HiAppEvent::AppEventFilter filter{domain, types};
    if (names == nullptr && namesLen > 0) {
        return ErrorCode::ERROR_INVALID_EVENT_NAME;
    }
    for (int i = 0; i < namesLen; ++i) {
        if (names[i] == nullptr) {
            return ErrorCode::ERROR_INVALID_EVENT_NAME;
        }
        filter.names.insert(names[i]);
    }
    filters_.emplace_back(std::move(filter));
    return 0;
}

void NdkAppEventWatcher::SetOnTrigger(OH_HiAppEvent_OnTrigger onTrigger)
{
    onTrigger_ = onTrigger;
}

void NdkAppEventWatcher::SetOnOnReceive(OH_HiAppEvent_OnReceive onReceive)
{
    onReceive_ = onReceive;
}

void NdkAppEventWatcher::OnEvents(const std::vector<std::shared_ptr<AppEventPack>> &events)
{
    if (events.empty() || onReceive_ == nullptr) {
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

void NdkAppEventWatcher::OnTrigger(const HiAppEvent::TriggerCondition &triggerCond)
{
    HILOG_DEBUG(LOG_CORE, "onTrigger start");
    if (onTrigger_ == nullptr) {
        HILOG_WARN(LOG_CORE, "onTrigger_ is nullptr");
        return;
    }
    onTrigger_(triggerCond.row, triggerCond.size);
}

bool NdkAppEventWatcher::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    return onReceive_ != nullptr;
}
}
}