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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_PROXY_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_PROXY_H

#include <string>

#include "app_event_observer.h"
#include "app_event_processor.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class AppEventProcessorProxy : public AppEventObserver {
public:
    AppEventProcessorProxy(const std::string& name, std::shared_ptr<AppEventProcessor> processor)
        : AppEventObserver(name), processor_(processor), userIdVersion_(-1), userPropertyVersion_(-1) {}
    ~AppEventProcessorProxy() = default;

    void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) override;
    bool VerifyEvent(std::shared_ptr<AppEventPack> event) override;

private:
    void GetValidUserIds(std::vector<UserId>& userIds);
    void GetValidUserProperties(std::vector<UserProperty>& userProperties);

private:
    std::shared_ptr<AppEventProcessor> processor_;
    int64_t userIdVersion_;
    int64_t userPropertyVersion_;
    std::vector<UserId> userIds_;
    std::vector<UserProperty> userProperties_;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_PROXY_H
