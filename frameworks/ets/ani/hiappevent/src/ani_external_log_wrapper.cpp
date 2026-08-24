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

#include "ani_external_log_wrapper.h"

#include "hiappevent_ani_parameter_name.h"
#include "hiappevent_ani_util.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "AniExtLogWrapper"

namespace OHOS {
namespace HiviewDFX {
namespace {
static ani_object Wrap(ani_env *env, ani_object obj, AniExternalLogWrapper *wrapper)
{
    if (env->Object_SetFieldByName_Long(obj, "nativeHolder", reinterpret_cast<ani_long>(wrapper)) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Wrap ExternalLogWrapper failed");
        return nullptr;
    }
    return obj;
}

static AniExternalLogWrapper* Unwrap(ani_env *env, ani_object obj)
{
    ani_long context = 0;
    if (env->Object_GetFieldByName_Long(obj, "nativeHolder", &context) != ANI_OK) {
        return nullptr;
    }
    return reinterpret_cast<AniExternalLogWrapper*>(context);
}
}

AniExternalLogWrapper::AniExternalLogWrapper(const ExternalLogWrapperInfo& info) : info_(info) {}

std::string AniExternalLogWrapper::GetFilePath() const
{
    return info_.filePath;
}

int64_t AniExternalLogWrapper::GetGenerationTime() const
{
    return info_.generationTime;
}

int64_t AniExternalLogWrapper::GetSizeInKb() const
{
    return info_.sizeInKb;
}

std::string AniExternalLogWrapper::GetSysEvent() const
{
    return info_.sysEvent;
}

void AniExternalLogWrapper::AniConstructor(ani_env *env, ani_object obj)
{
    // Object is created from C++ side via CreateAniObject, nativeConstructor is a no-op
}

void AniExternalLogWrapper::AniFinalize(ani_env *env, ani_object object, ani_long nativeHolder)
{
    auto* wrapper = reinterpret_cast<AniExternalLogWrapper*>(nativeHolder);
    delete wrapper;
}

ani_string AniExternalLogWrapper::AniGetFilePath(ani_env *env, ani_object obj)
{
    auto* wrapper = Unwrap(env, obj);
    if (wrapper == nullptr) {
        return HiAppEventAniUtil::CreateAniString(env, "");
    }
    return HiAppEventAniUtil::CreateAniString(env, wrapper->GetFilePath());
}

ani_long AniExternalLogWrapper::AniGetGenerationTime(ani_env *env, ani_object obj)
{
    auto* wrapper = Unwrap(env, obj);
    if (wrapper == nullptr) {
        return 0;
    }
    return static_cast<ani_long>(wrapper->GetGenerationTime());
}

ani_long AniExternalLogWrapper::AniGetSizeInKb(ani_env *env, ani_object obj)
{
    auto* wrapper = Unwrap(env, obj);
    if (wrapper == nullptr) {
        return 0;
    }
    return static_cast<ani_long>(wrapper->GetSizeInKb());
}

ani_string AniExternalLogWrapper::AniGetSysEvent(ani_env *env, ani_object obj)
{
    auto* wrapper = Unwrap(env, obj);
    if (wrapper == nullptr) {
        return HiAppEventAniUtil::CreateAniString(env, "");
    }
    return HiAppEventAniUtil::CreateAniString(env, wrapper->GetSysEvent());
}

ani_object AniExternalLogWrapper::CreateAniObject(ani_env *env, const ExternalLogWrapperInfo& info)
{
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_EXT_LOG_WRAPPER, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_EXT_LOG_WRAPPER);
        return nullptr;
    }
    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get %{public}s ctor Failed", CLASS_NAME_EXT_LOG_WRAPPER);
        return nullptr;
    }
    ani_object obj {};
    if (env->Object_New(cls, ctor, &obj) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Create Object Failed: %{public}s", CLASS_NAME_EXT_LOG_WRAPPER);
        return nullptr;
    }
    auto* wrapper = new(std::nothrow) AniExternalLogWrapper(info);
    if (wrapper != nullptr) {
        Wrap(env, obj, wrapper);
    }
    return obj;
}

} // namespace HiviewDFX
} // namespace OHOS