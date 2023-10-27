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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_H

#include <string>

#include "app_event_observer.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
struct AppEventInfo {
    std::string domain;
    std::string name;
    int eventType = 0;
    uint64_t timestamp = 0;
    std::string params;
};

class AppEventProcessor {
public:
    AppEventProcessor() = default;
    virtual ~AppEventProcessor() = default;

    virtual int OnReport(
        int64_t processorSeq,
        const std::vector<UserId>& userIds,
        const std::vector<UserProperty>& userProperties,
        const std::vector<AppEventInfo>& events) = 0;
    virtual int ValidateUserId(const UserId& userId) = 0;
    virtual int ValidateUserProperty(const UserProperty& userProperty) = 0;
    virtual int ValidateEvent(const AppEventInfo& event) = 0;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_H
