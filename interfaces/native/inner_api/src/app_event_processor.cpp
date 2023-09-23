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
#include "app_event_processor.h"

#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
void AppEventProcessor::OnEvent(std::shared_ptr<AppEventPack> event)
{
    std::vector<UserId> userIds;
    std::vector<UserProperty> userProperties;
    AppEventInfo appEventInfo = {
        .domain = event->GetDomain(),
        .name = event->GetName(),
        .eventType = event->GetType(),
        .timestamp = event->GetTime(),
        .params = event->GetParamStr(),
    };
    OnReport(userIds, userProperties, {appEventInfo});
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
