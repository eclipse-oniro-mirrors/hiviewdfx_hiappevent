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
#include "app_event_processor_proxy.h"

#include <algorithm>
#include <sstream>

#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hiappevent_userinfo.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ProcessorProxy"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
constexpr int MAX_SIZE_ON_EVENTS = 100;

AppEventInfo CreateAppEventInfo(std::shared_ptr<AppEventPack> event)
{
    AppEventInfo appEventInfo = {
        .domain = event->GetDomain(),
        .name = event->GetName(),
        .eventType = event->GetType(),
        .timestamp = event->GetTime(),
        .params = event->GetParamStr(),
    };
    return appEventInfo;
}

std::string GetStr(const std::unordered_set<std::string>& strSet)
{
    if (strSet.empty()) {
        return "[]";
    }
    std::string resStr("[");
    for (const auto& str : strSet) {
        resStr += str + ",";
    }
    resStr.pop_back(); // for delete ','
    return resStr + "]";
}

std::string GetStr(const std::vector<EventConfig>& eventConfigs)
{
    if (eventConfigs.empty()) {
        return "[]";
    }
    std::string resStr("[");
    for (const auto& eventConfig : eventConfigs) {
        resStr += eventConfig.ToString() + ",";
    }
    resStr.pop_back(); // for delete ','
    return resStr + "]";
}

std::string GetStr(const std::unordered_map<std::string, std::string>& customConfigs)
{
    if (customConfigs.empty()) {
        return "[]";
    }
    std::string resStr("[");
    for (const auto& customConfig : customConfigs) {
        resStr += "{" + customConfig.first + "," + customConfig.second + "}" + ",";
    }
    resStr.pop_back(); // for delete ','
    return resStr +"]";
}
}

bool EventConfig::IsValidEvent(std::shared_ptr<AppEventPack> event) const
{
    if (domain.empty() && name.empty()) {
        return false;
    }
    if (!domain.empty() && (domain != event->GetDomain())) {
        return false;
    }
    if (!name.empty() && (name != event->GetName())) {
        return false;
    }
    return true;
}

bool EventConfig::IsRealTimeEvent(std::shared_ptr<AppEventPack> event) const
{
    return IsValidEvent(event) && isRealTime;
}

std::string EventConfig::ToString() const
{
    std::stringstream strStream;
    strStream << "{" << domain << "," << name << "," << isRealTime << "}";
    return strStream.str();
}

std::string TriggerCondition::ToString() const
{
    std::stringstream strStream;
    strStream << "{" << row << "," << size << "," << timeout << "," << onStartup << "," << onBackground << "}";
    return strStream.str();
}

std::string ReportConfig::ToString() const
{
    std::stringstream strStream;
    strStream << "{" << name << "," << debugMode << "," << routeInfo << "," << appId << "," << triggerCond.ToString()
        << "," << GetStr(userIdNames) << "," << GetStr(userPropertyNames) << "," << GetStr(eventConfigs) << ","
        << configId << "," << GetStr(customConfigs) << "," << configName << "}";
    return strStream.str();
}

void AppEventProcessorProxy::OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    if (events.empty()) {
        return;
    }

    std::vector<UserId> userIds;
    GetValidUserIds(userIds);
    std::vector<UserProperty> userProperties;
    GetValidUserProperties(userProperties);
    int64_t observerSeq = GetSeq();
    std::vector<AppEventInfo> eventInfos;
    std::vector<int64_t> eventSeqs;
    for (const auto& event : events) {
        eventInfos.emplace_back(CreateAppEventInfo(event));
        eventSeqs.emplace_back(event->GetSeq());
    }
    if (processor_->OnReport(observerSeq, userIds, userProperties, eventInfos) == 0) {
        if (!AppEventStore::GetInstance().DeleteData(observerSeq, eventSeqs)) {
            HILOG_ERROR(LOG_CORE, "failed to delete mapping data, seq=%{public}" PRId64 ", event num=%{public}zu",
                observerSeq, eventSeqs.size());
        }
    } else {
        HILOG_DEBUG(LOG_CORE, "failed to report event, seq=%{public}" PRId64 ", event num=%{public}zu",
            observerSeq, eventSeqs.size());
    }
}

void AppEventProcessorProxy::GetValidUserIds(std::vector<UserId>& userIds)
{
    int64_t userIdVerForAll = HiAppEvent::UserInfo::GetInstance().GetUserIdVersion();
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (userIdVerForAll == userIdVersion_) {
        userIds = userIds_;
        return;
    }
    std::vector<UserId> allUserIds = HiAppEvent::UserInfo::GetInstance().GetUserIds();
    std::for_each(allUserIds.begin(), allUserIds.end(), [&userIds, this](const auto& userId) {
        if (reportConfig_.userIdNames.find(userId.name) != reportConfig_.userIdNames.end()
            && processor_->ValidateUserId(userId) == 0) {
            userIds.emplace_back(userId);
        }
    });
    userIds_ = userIds;
    userIdVersion_ = userIdVerForAll;
}

void AppEventProcessorProxy::GetValidUserProperties(std::vector<UserProperty>& userProperties)
{
    int64_t userPropertyVerForAll = HiAppEvent::UserInfo::GetInstance().GetUserPropertyVersion();
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (userPropertyVerForAll == userPropertyVersion_) {
        userProperties = userProperties_;
        return;
    }
    std::vector<UserProperty> allUserProperties = HiAppEvent::UserInfo::GetInstance().GetUserProperties();
    std::for_each(allUserProperties.begin(), allUserProperties.end(),
        [&userProperties, this](const auto& userProperty) {
            if (reportConfig_.userPropertyNames.find(userProperty.name) != reportConfig_.userPropertyNames.end()
                && processor_->ValidateUserProperty(userProperty) == 0) {
                userProperties.emplace_back(userProperty);
            }
        }
    );
    userProperties_ = userProperties;
    userPropertyVersion_ = userPropertyVerForAll;
}

bool AppEventProcessorProxy::VerifyEvent(std::shared_ptr<AppEventPack> event)
{
    return AppEventObserver::VerifyEvent(event)
        && (processor_->ValidateEvent(CreateAppEventInfo(event)) == 0);
}

bool AppEventProcessorProxy::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    const auto& eventConfigs = reportConfig_.eventConfigs;
    if (eventConfigs.empty()) {
        return false;
    }
    auto it = std::find_if(eventConfigs.begin(), eventConfigs.end(), [event](const auto& config) {
        return config.IsRealTimeEvent(event);
    });
    return it != eventConfigs.end();
}

ReportConfig AppEventProcessorProxy::GetReportConfig()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    return reportConfig_;
}

void AppEventProcessorProxy::SetReportConfig(const ReportConfig& reportConfig)
{
    {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        reportConfig_ = reportConfig;
    }
    SetTriggerCond(reportConfig.triggerCond);

    std::vector<AppEventFilter> filters;
    // if event configs is empty, do not report event
    if (reportConfig.eventConfigs.empty()) {
        filters.emplace_back(AppEventFilter()); // invalid filter
        SetFilters(filters);
        return;
    }

    for (const auto& eventConfig : reportConfig.eventConfigs) {
        if (eventConfig.domain.empty() && eventConfig.name.empty()) {
            continue;
        }
        std::unordered_set<std::string> names;
        if (!eventConfig.name.empty()) {
            names.emplace(eventConfig.name);
        }
        filters.emplace_back(AppEventFilter(eventConfig.domain, names));
    }
    SetFilters(filters);
}

int64_t AppEventProcessorProxy::GenerateHashCode()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (hashCode_ != 0) {
        return hashCode_;
    }
    hashCode_ = (reportConfig_.configId > 0)
        ? static_cast<int64_t>(reportConfig_.configId)
        : static_cast<int64_t>(std::hash<std::string>{}(reportConfig_.ToString()));
    return hashCode_;
}

void AppEventProcessorProxy::OnTrigger(const TriggerCondition& triggerCond)
{
    std::vector<std::shared_ptr<AppEventPack>> events;
    QueryEventsFromDb(events);
    if (!events.empty()) {
        OnEvents(events);
    }
}

void AppEventProcessorProxy::QueryEventsFromDb(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    int64_t seq = GetSeq();
    std::string name = GetName();
    if (AppEventStore::GetInstance().QueryEvents(events, seq, MAX_SIZE_ON_EVENTS) != 0) {
        HILOG_WARN(LOG_CORE, "failed to take data from observer=%{public}s, seq=%{public}" PRId64,
            name.c_str(), seq);
        return;
    }
    HILOG_INFO(LOG_CORE, "end to take data from observer=%{public}s, seq=%{public}" PRId64 ", size=%{public}zu",
        name.c_str(), seq, events.size());
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
