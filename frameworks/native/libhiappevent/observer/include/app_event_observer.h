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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_APP_EVENT_OBSERVER_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_APP_EVENT_OBSERVER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base_type.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
constexpr int TIMEOUT_STEP = 30; // step of time is 30s

struct AppEventFilter {
    /* Filtering events by event domain */
    std::string domain;

    /* Filtering events by event names */
    std::unordered_set<std::string> names;

    /* Filtering events by event types, stored in bits */
    uint32_t types = 0;

    AppEventFilter(
        const std::string& domain = "",
        const std::unordered_set<std::string>& names = {},
        uint32_t types = 0);
    AppEventFilter(const std::string& domain, uint32_t types);

    bool IsValidEvent(std::shared_ptr<AppEventPack> event) const;
    bool IsValidEvent(const std::string& eventDomain, const std::string& eventName, int eventType) const;
    uint64_t GetOsEventsMask() const;
};

class AppEventObserver {
public:
    AppEventObserver(const std::string& name) : name_(name) {}
    AppEventObserver(
        const std::string& name,
        const std::vector<AppEventFilter>& filters,
        TriggerCondition cond) : name_(name), filters_(filters), triggerCond_(cond) {}
    virtual ~AppEventObserver() = default;
    virtual void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) {}
    virtual bool VerifyEvent(std::shared_ptr<AppEventPack> event);
    virtual bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event) { return false; }
    virtual void OnTrigger(const TriggerCondition& triggerCond) {}
    void ProcessEvent(std::shared_ptr<AppEventPack> event);
    void ProcessTimeout();
    void ProcessStartup();
    void ProcessBackground();
    bool HasTimeoutCondition();

    std::string GetName();
    int64_t GetSeq();
    void SetSeq(int64_t seq);
    void SetCurrCondition(const TriggerCondition& triggerCond);
    // used to reset the current status when condition is met or data is cleared.
    void ResetCurrCondition();
    void SetTriggerCond(const TriggerCondition& triggerCond);
    std::vector<AppEventFilter> GetFilters();
    void SetFilters(const std::vector<AppEventFilter>& filters);
    void AddFilter(const AppEventFilter& filter);

private:
    std::string name_;
    int64_t seq_ = 0; // observer sequence, used to uniquely identify an observer
    std::vector<AppEventFilter> filters_;
    TriggerCondition triggerCond_;
    TriggerCondition currCond_;
    std::mutex mutex_;
    std::mutex condMutex_;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_APP_EVENT_OBSERVER_H
