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
#include "app_event_processor_proxy.h"

#include <algorithm>

#include "hiappevent_base.h"
#include "hiappevent_userinfo.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
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
    std::vector<AppEventInfo> eventInfos(events.size());
    std::transform(events.begin(), events.end(), eventInfos.begin(), [](auto event) {
        return CreateAppEventInfo(event);
    });
    (void)processor_->OnReport(GetSeq(), userIds, userProperties, eventInfos);
}

void AppEventProcessorProxy::GetValidUserIds(std::vector<UserId>& userIds)
{
    int64_t userIdVerForAll = HiAppEvent::UserInfo::GetInstance().GetUserIdVersion();
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
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
