/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_COMMON_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_COMMON_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
const std::string DOMAIN_OS = "OS";
const std::string APP_CRASH = "APP_CRASH";
const std::string APP_FREEZE = "APP_FREEZE";
const std::string APP_LAUNCH = "APP_LAUNCH";
const std::string SCROLL_JANK = "SCROLL_JANK";
const std::string CPU_USAGE_HIGH = "CPU_USAGE_HIGH";
const std::string BATTERY_USAGE = "BATTERY_USAGE";
const std::string RESOURCE_OVERLIMIT = "RESOURCE_OVERLIMIT";
const std::string ADDRESS_SANITIZER = "ADDRESS_SANITIZER";
const std::string MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
const std::string APP_START = "APP_START";
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_COMMON_H
