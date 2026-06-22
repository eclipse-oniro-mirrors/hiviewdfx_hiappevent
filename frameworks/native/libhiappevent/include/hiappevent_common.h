/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
constexpr const char* DOMAIN_OS = "OS";
constexpr const char* EVENT_APP_CRASH = "APP_CRASH";
constexpr const char* EVENT_APP_FREEZE = "APP_FREEZE";
constexpr const char* EVENT_APP_FREEZE_WARNING = "APPFREEZE_WARNING";
constexpr const char* EVENT_APP_LAUNCH = "APP_LAUNCH";
constexpr const char* EVENT_SCROLL_JANK = "SCROLL_JANK";
constexpr const char* EVENT_CPU_USAGE_HIGH = "CPU_USAGE_HIGH";
constexpr const char* EVENT_BATTERY_USAGE = "BATTERY_USAGE";
constexpr const char* EVENT_RESOURCE_OVERLIMIT = "RESOURCE_OVERLIMIT";
constexpr const char* EVENT_ADDRESS_SANITIZER = "ADDRESS_SANITIZER";
constexpr const char* EVENT_MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
constexpr const char* EVENT_APP_HICOLLIE = "APP_HICOLLIE";
constexpr const char* EVENT_APP_KILLED = "APP_KILLED";
constexpr const char* EVENT_AUDIO_JANK_FRAME = "AUDIO_JANK_FRAME";
constexpr const char* EVENT_SCROLL_ARKWEB_FLING_JANK = "SCROLL_ARKWEB_FLING_JANK";
const char DOMAIN_PROPERTY[] = "domain";
const char NAMES_PROPERTY[] = "names";
const char TYPES_PROPERTY[] = "types";
const char NAME_PROPERTY[] = "name";
const char EVENT_TYPE_PROPERTY[] = "eventType";
const char PARAM_PROPERTY[] = "params";
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_COMMON_H
