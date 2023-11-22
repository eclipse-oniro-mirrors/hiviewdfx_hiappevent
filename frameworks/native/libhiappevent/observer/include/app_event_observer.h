/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

namespace HiAppEvent {
struct UserId {
    std::string name;
    std::string value;
};

struct UserProperty {
    std::string name;
    std::string value;
};

struct TriggerCondition {
    /* Triggered by the number of events */
    int row = 0;

    /* Triggered by the size of events */
    int size = 0;

    /* Triggered periodically */
    int timeout = 0;

    /* Triggered by application startup */
    bool onStartup = false;

    /* Triggered by switching to the background */
    bool onBackground = false;

    std::string ToString() const;
};

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
};

struct EventConfig {
    /* Specifies the event domain that can be reported */
    std::string domain;

    /* Specifies the event name that can be reported */
    std::string name;

    /* Specifies whether the event is a real-time report event */
    bool isRealTime = false;

    bool IsValidEvent(std::shared_ptr<AppEventPack> event) const;

    bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event) const;

    std::string ToString() const;
};

struct ReportConfig {
    /* The report name */
    std::string name;

    /* Enable the developer to debug */
    bool debugMode = false;

    /* The server location information */
    std::string routeInfo;

    /* The application ID */
    std::string appId;

    /* The condition for triggering reporting */
    TriggerCondition triggerCond;

    /* The user id names that can be reported */
    std::unordered_set<std::string> userIdNames;

    /* The user property names that can be reported */
    std::unordered_set<std::string> userPropertyNames;

    /* Event configurations to report */
    std::vector<EventConfig> eventConfigs;

    std::string ToString() const;
};

class AppEventObserver {
public:
    AppEventObserver(const std::string& name) : name_(name) {}
    virtual ~AppEventObserver() = default;
    virtual void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) = 0;
    virtual bool VerifyEvent(std::shared_ptr<AppEventPack> event);
    virtual bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event);
    void ProcessEvent(std::shared_ptr<AppEventPack> event);
    void ProcessTimeout();
    void ProcessStartup();
    void ProcessBackground();
    bool HasOsDomain();

    std::string GetName();
    int64_t GetSeq();
    ReportConfig GetReportConfig();
    void SetSeq(int64_t seq);
    void SetCurrCondition(const TriggerCondition& triggerCond);
    void SetReportConfig(const ReportConfig& reportConfig);

    // used to identify the observer with the same config
    int64_t GenerateHashCode();

    // userd to reset the current status when condition is met or data is cleared.
    void ResetCurrCondition();

protected:
    virtual void OnTrigger(const TriggerCondition& triggerCond);

private:
    int StoreEventToDb(std::shared_ptr<AppEventPack> event);
    void QueryEventsFromDb(std::vector<std::shared_ptr<AppEventPack>>& events);
    bool MeetProcessCondition();
    bool MeetTimeoutCondition();
    bool MeetStartupCondition();
    bool MeetBackgroundCondition();

protected:
    std::vector<AppEventFilter> filters_;
    ReportConfig reportConfig_;

private:
    std::string name_;
    int64_t seq_ = 0; // observer sequence, used to uniquely identify an observer
    TriggerCondition currCond_;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_APP_EVENT_OBSERVER_H
