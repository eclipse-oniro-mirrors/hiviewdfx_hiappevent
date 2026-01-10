/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "app_event_observer.h"

#include "app_event.h"
#include "hiappevent_base.h"
#include "hiappevent_common.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Observer"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
constexpr uint64_t BIT_MASK = 1;
struct OsEventPosInfo {
    std::string name;
    EventType type;
    uint8_t pos; // means position in binary
};
const std::vector<OsEventPosInfo> OS_EVENT_POS_INFOS = {
    { EVENT_APP_CRASH, FAULT, 0 },
    { EVENT_APP_FREEZE, FAULT, 1 },
    { EVENT_APP_LAUNCH, BEHAVIOR, 2 },
    { EVENT_SCROLL_JANK, FAULT, 3 },
    { EVENT_CPU_USAGE_HIGH, FAULT, 4 },
    { EVENT_BATTERY_USAGE, STATISTIC, 5 },
    { EVENT_RESOURCE_OVERLIMIT, FAULT, 6 },
    { EVENT_ADDRESS_SANITIZER, FAULT, 7 },
    { EVENT_MAIN_THREAD_JANK, FAULT, 8 },
    { EVENT_APP_HICOLLIE, FAULT, 10 },
    { EVENT_APP_KILLED, STATISTIC, 11 },
    { EVENT_AUDIO_JANK_FRAME, FAULT, 12 },
    { EVENT_SCROLL_ARKWEB_FLING_JANK, STATISTIC, 13 },
};

bool MeetNumberCondition(int currNum, int maxNum)
{
    return maxNum > 0 && currNum >= maxNum;
}

void ResetCondition(TriggerCondition& cond)
{
    cond.row = 0;
    cond.size = 0;
    cond.timeout = 0;
}
}

AppEventFilter::AppEventFilter(const std::string& domain, const std::unordered_set<std::string>& names,
    uint32_t types) : domain(domain), names(names), types(types)
{}

AppEventFilter::AppEventFilter(const std::string& domain, uint32_t types) : domain(domain), types(types)
{}

bool AppEventFilter::IsValidEvent(std::shared_ptr<AppEventPack> event) const
{
    return IsValidEvent(event->GetDomain(), event->GetName(), event->GetType());
}

bool AppEventFilter::IsValidEvent(const std::string& eventDomain, const std::string& eventName, int eventType) const
{
    if (domain.empty()) {
        return false;
    }
    if (!domain.empty() && domain != eventDomain) {
        return false;
    }
    if (!names.empty() && (names.find(eventName) == names.end())) {
        return false;
    }
    if (types != 0 && !(types & (1 << eventType))) { // 1: bit mask
        return false;
    }
    return true;
}

uint64_t AppEventFilter::GetOsEventsMask() const
{
    uint64_t mask = 0;
    for (const auto& event : OS_EVENT_POS_INFOS) {
        if (IsValidEvent(DOMAIN_OS, event.name, event.type)) {
            mask |= (BIT_MASK << event.pos);
        }
    }
    return mask;
}

bool AppEventObserver::VerifyEvent(std::shared_ptr<AppEventPack> event)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (filters_.empty()) {
        return true;
    }
    auto it = std::find_if(filters_.begin(), filters_.end(), [event](const auto& filter) {
        return filter.IsValidEvent(event);
    });
    return it != filters_.end();
}

void AppEventObserver::ProcessEvent(std::shared_ptr<AppEventPack> event)
{
    HILOG_DEBUG(LOG_CORE, "observer=%{public}s start to process event", name_.c_str());
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    ++currCond_.row;
    currCond_.size += static_cast<int>(event->GetEventStr().size());
    if (MeetNumberCondition(currCond_.row, triggerCond_.row)
        || MeetNumberCondition(currCond_.size, triggerCond_.size)) {
        OnTrigger(currCond_);
        ResetCondition(currCond_);
    }
}

void AppEventObserver::ResetCurrCondition()
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    ResetCondition(currCond_);
}

void AppEventObserver::ProcessTimeout()
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    currCond_.timeout += TIMEOUT_STEP;
    if (MeetNumberCondition(currCond_.timeout, triggerCond_.timeout) && currCond_.row > 0) {
        OnTrigger(currCond_);
        ResetCondition(currCond_);
    }
}

bool AppEventObserver::HasTimeoutCondition()
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    return triggerCond_.timeout > 0 && currCond_.row > 0;
}

void AppEventObserver::ProcessStartup()
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    if (triggerCond_.onStartup && currCond_.row > 0) {
        OnTrigger(currCond_);
        ResetCondition(currCond_);
    }
}

void AppEventObserver::ProcessBackground()
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    if (triggerCond_.onBackground && currCond_.row > 0) {
        OnTrigger(currCond_);
        ResetCondition(currCond_);
    }
}

std::string AppEventObserver::GetName()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    return name_;
}

int64_t AppEventObserver::GetSeq()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    return seq_;
}

void AppEventObserver::SetSeq(int64_t seq)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    seq_ = seq;
}

void AppEventObserver::SetCurrCondition(const TriggerCondition& triggerCond)
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    currCond_ = triggerCond;
}

void AppEventObserver::SetTriggerCond(const TriggerCondition& triggerCond)
{
    std::lock_guard<std::mutex> lockGuard(condMutex_);
    triggerCond_ = triggerCond;
}

std::vector<AppEventFilter> AppEventObserver::GetFilters()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    return filters_;
}

void AppEventObserver::SetFilters(const std::vector<AppEventFilter>& filters)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    filters_ = filters;
}

void AppEventObserver::AddFilter(const AppEventFilter& filter)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    filters_.emplace_back(filter);
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
