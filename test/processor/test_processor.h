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
#ifndef HIAPPEVENT_TEST_PROCESSOR_H
#define HIAPPEVENT_TEST_PROCESSOR_H

#include "app_event_processor.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class TestProcessor : public AppEventProcessor {
    int OnReport(
        int64_t processorSeq,
        const std::vector<UserId>& userIds,
        const std::vector<UserProperty>& userProperties,
        const std::vector<AppEventInfo>& events) override;
    int ValidateUserId(const UserId& userId) override;
    int ValidateUserProperty(const UserProperty& userProperty) override;
    int ValidateEvent(const AppEventInfo& event) override;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_TEST_PROCESSOR_H
