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

#ifndef HIAPPEVENT_ANI_UTIL_H
#define HIAPPEVENT_ANI_UTIL_H

#include <ani.h>
#include <map>
#include <unordered_set>
#include "hilog/log.h"
#include "hilog/log_cpp.h"
#include "hiappevent_ani_error_code.h"
#include "hiappevent_ani_parameter_name.h"

namespace OHOS {
namespace HiviewDFX {
enum AniArgsType {
    ANI_UNKNOWN = -1,
    ANI_INT = 0,
    ANI_BOOLEAN = 1,
    ANI_NUMBER = 2,
    ANI_STRING = 3,
    ANI_NULL = 4,
    ANI_UNDEFINED = 5,
};
class HiAppEventAniUtil {
public:
    static std::string CreateErrMsg(const std::string &name);
    static bool IsArray(ani_env *env, ani_object object);
    static bool IsRefUndefined(ani_env *env, ani_ref ref);
    static AniArgsType GetArgType(ani_env *env, ani_object elementObj);
    static AniArgsType GetArrayType(ani_env *env, ani_ref arrayRef);
    static std::string ParseStringValue(ani_env *env, ani_ref aniStrRef);
    static int32_t ParseIntValue(ani_env *env, ani_ref elementRef);
    static bool ParseBoolValue(ani_env *env, ani_ref elementRef);
    static double ParseNumberValue(ani_env *env, ani_ref elementRef);
    static void GetStringsToSet(ani_env *env, ani_ref Ref, std::unordered_set<std::string> &arr);
    static void ParseRecord(ani_env *env, ani_ref recordRef, std::map<std::string, ani_ref> &recordResult);
    static ani_ref GetProperty(ani_env *env, ani_object object, const std::string &name);
    static void ThrowAniError(ani_env *env, int32_t code, const std::string &message);
    static std::vector<bool> GetBooleans(ani_env *env, ani_ref arrayRef);
    static std::vector<double> GetDoubles(ani_env *env, ani_ref arrayRef);
    static std::vector<std::string> GetStrings(ani_env *env, ani_ref arrayRef);
    static std::vector<int> GetInts(ani_env *env, ani_ref arrayRef);
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_ANI_UTIL_H
