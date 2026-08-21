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

#ifndef ANI_EXTERNAL_LOG_WRAPPER_H
#define ANI_EXTERNAL_LOG_WRAPPER_H

#include <ani.h>

#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {

class AniExternalLogWrapper {
public:
    AniExternalLogWrapper() = default;
    explicit AniExternalLogWrapper(const ExternalLogWrapperInfo& info);
    ~AniExternalLogWrapper() = default;

    std::string GetFilePath() const;
    int64_t GetGenerationTime() const;
    int64_t GetSizeInKb() const;
    std::string GetSysEvent() const;

    static void AniConstructor(ani_env *env, ani_object obj);
    static void AniFinalize(ani_env *env, ani_object object, ani_long nativeHolder);
    static ani_string AniGetFilePath(ani_env *env, ani_object obj);
    static ani_long AniGetGenerationTime(ani_env *env, ani_object obj);
    static ani_long AniGetSizeInKb(ani_env *env, ani_object obj);
    static ani_string AniGetSysEvent(ani_env *env, ani_object obj);

    static ani_object CreateAniObject(ani_env *env, const ExternalLogWrapperInfo& info);

private:
    ExternalLogWrapperInfo info_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_EXTERNAL_LOG_WRAPPER_H