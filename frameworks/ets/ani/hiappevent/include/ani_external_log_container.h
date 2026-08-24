/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ANI_EXTERNAL_LOG_CONTAINER_H
#define ANI_EXTERNAL_LOG_CONTAINER_H

#include <ani.h>

#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {

class AniExternalLogContainer {
public:
    AniExternalLogContainer() = default;
    explicit AniExternalLogContainer(const std::vector<ExternalLogWrapperInfo>& logInfos);
    ~AniExternalLogContainer() = default;

    const std::vector<ExternalLogWrapperInfo>& GetLogInfos() const;

    static void AniConstructor(ani_env *env, ani_object obj);
    static void AniFinalize(ani_env *env, ani_object object, ani_long nativeHolder);
    static ani_object AniGetAllLogs(ani_env *env, ani_object obj);
    static ani_object AniGetAllLogFiles(ani_env *env, ani_object obj);
    static ani_object AniGetLogFilesOfSysEvent(ani_env *env, ani_object obj, ani_string event);
    static ani_object AniGetLogFilesGeneratedAfter(ani_env *env, ani_object obj, ani_long timePoint);
    static ani_object AniGetLogFilesGeneratedBefore(ani_env *env, ani_object obj, ani_long timePoint);
    static ani_object AniGetLogFilesLargerThan(ani_env *env, ani_object obj, ani_long sizeKb);
    static ani_object AniGetLogFilesSmallerThan(ani_env *env, ani_object obj, ani_long sizeKb);
    static ani_int AniGetLogNumber(ani_env *env, ani_object obj);
    static ani_object AniGetFirstGeneratedLogFiles(ani_env *env, ani_object obj, ani_int num);

    static ani_object CreateAniObject(ani_env *env, const std::vector<ExternalLogWrapperInfo>& logInfos);

private:
    std::vector<ExternalLogWrapperInfo> logInfos_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_EXTERNAL_LOG_CONTAINER_H