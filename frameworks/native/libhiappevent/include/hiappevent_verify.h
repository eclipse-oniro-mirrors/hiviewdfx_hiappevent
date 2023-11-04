/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef HI_APP_EVENT_VERIFY_H
#define HI_APP_EVENT_VERIFY_H

#include <memory>
#include <string>

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
bool IsValidDomain(const std::string& domain);
bool IsValidEventName(const std::string& eventName);
bool IsValidWatcherName(const std::string& watcherName);
bool IsValidEventType(int eventType);
int VerifyAppEvent(std::shared_ptr<AppEventPack>& appEventPack);
bool IsValidProcessorName(const std::string& name);
bool IsValidRouteInfo(const std::string& name);
bool IsValidPeriodReport(int timeout);
bool IsValidBatchReport(int count);
bool IsValidUserIdName(const std::string& name);
bool IsValidUserIdValue(const std::string& value);
bool IsValidUserPropName(const std::string& name);
bool IsValidUserPropValue(const std::string& value);
} // namespace HiviewDFX
} // namespace OHOS
#endif // HI_APP_EVENT_VERIFY_H
