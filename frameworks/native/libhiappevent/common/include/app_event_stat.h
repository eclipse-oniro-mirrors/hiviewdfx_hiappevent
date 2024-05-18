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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_COMMON_APP_EVENT_STAT_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_COMMON_APP_EVENT_STAT_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace AppEventStat {
constexpr int SUCCESS = 0;
constexpr int FAILED = 1;

void SyncWriteApiEndEvent(const std::string& apiName, uint64_t beginTime, int result, int errCode);
} // namespace AppEventStat
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_COMMON_APP_EVENT_STAT_H
