/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_APP_EVENT_UTIL_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_APP_EVENT_UTIL_H

#include <string>
#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {
namespace AppEventUtil {
bool IsBetaVersion();
void ReportAppEventReceive(const std::vector<std::shared_ptr<AppEventPack>>& appEventInfos,
                           const std::string& watcherName, const std::string& callback);
} // namespace AppEventUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_APP_EVENT_UTIL_H
