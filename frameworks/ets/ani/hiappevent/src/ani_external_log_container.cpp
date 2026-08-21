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

#include "ani_external_log_container.h"

#include <algorithm>

#include "ani_external_log_wrapper.h"
#include "hiappevent_ani_parameter_name.h"
#include "hiappevent_ani_util.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "AniExtLogContainer"

namespace OHOS {
namespace HiviewDFX {
namespace {
static ani_object Wrap(ani_env *env, ani_object obj, AniExternalLogContainer *container)
{
    if (env->Object_SetFieldByName_Long(obj, "nativeHolder", reinterpret_cast<ani_long>(container)) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Wrap ExternalLogContainer failed");
        return nullptr;
    }
    return obj;
}

static AniExternalLogContainer* Unwrap(ani_env *env, ani_object obj)
{
    ani_long context = 0;
    if (env->Object_GetFieldByName_Long(obj, "nativeHolder", &context) != ANI_OK) {
        return nullptr;
    }
    return reinterpret_cast<AniExternalLogContainer*>(context);
}

static ani_ref CreateStringArray(ani_env *env, const std::vector<std::string>& strs)
{
    return HiAppEventAniUtil::CreateStrings(env, strs);
}
}

AniExternalLogContainer::AniExternalLogContainer(const std::vector<ExternalLogWrapperInfo>& logInfos)
    : logInfos_(logInfos) {}

const std::vector<ExternalLogWrapperInfo>& AniExternalLogContainer::GetLogInfos() const
{
    return logInfos_;
}

void AniExternalLogContainer::AniConstructor(ani_env *env, ani_object obj)
{
    // Object is created from C++ side via CreateAniObject, nativeConstructor is a no-op
}

void AniExternalLogContainer::AniFinalize(ani_env *env, ani_object object, ani_long nativeHolder)
{
    auto* container = reinterpret_cast<AniExternalLogContainer*>(nativeHolder);
    delete container;
}

ani_object AniExternalLogContainer::AniGetAllLogs(ani_env *env, ani_object obj)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    const auto& logInfos = container->GetLogInfos();
    // Create Array<ExternalLogWrapper>
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_ARRAY, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_ARRAY);
        return nullptr;
    }
    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", "i:", &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Array ctor find Failed");
        return nullptr;
    }
    ani_object array {};
    if (env->Object_New(cls, ctor, &array, static_cast<ani_int>(logInfos.size())) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Array Object_New Failed");
        return nullptr;
    }
    ani_method setMethod {};
    if (env->Class_FindMethod(cls, "$_set", "iY:", &setMethod) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Array $_set find Failed");
        return nullptr;
    }
    for (size_t i = 0; i < logInfos.size(); ++i) {
        ani_object wrapperObj = AniExternalLogWrapper::CreateAniObject(env, logInfos[i]);
        if (wrapperObj == nullptr) {
            continue;
        }
        env->Object_CallMethod_Void(array, setMethod, static_cast<ani_int>(i), wrapperObj);
    }
    return array;
}

ani_object AniExternalLogContainer::AniGetAllLogFiles(ani_env *env, ani_object obj)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    const auto& logInfos = container->GetLogInfos();
    std::vector<std::string> files;
    files.reserve(logInfos.size());
    for (const auto& info : logInfos) {
        files.push_back(info.filePath);
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_object AniExternalLogContainer::AniGetLogFilesOfSysEvent(ani_env *env, ani_object obj, ani_string event)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    std::string sysEvent = HiAppEventAniUtil::ParseStringValue(env, event);
    const auto& logInfos = container->GetLogInfos();
    std::vector<std::string> files;
    for (const auto& info : logInfos) {
        if (info.sysEvent == sysEvent) {
            files.push_back(info.filePath);
        }
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_object AniExternalLogContainer::AniGetLogFilesGeneratedAfter(ani_env *env, ani_object obj, ani_long timePoint)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    int64_t time = static_cast<int64_t>(timePoint);
    const auto& logInfos = container->GetLogInfos();
    std::vector<std::string> files;
    for (const auto& info : logInfos) {
        if (info.generationTime > time) {
            files.push_back(info.filePath);
        }
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_object AniExternalLogContainer::AniGetLogFilesGeneratedBefore(ani_env *env, ani_object obj, ani_long timePoint)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    int64_t time = static_cast<int64_t>(timePoint);
    const auto& logInfos = container->GetLogInfos();
    std::vector<std::string> files;
    for (const auto& info : logInfos) {
        if (info.generationTime < time) {
            files.push_back(info.filePath);
        }
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_object AniExternalLogContainer::AniGetLogFilesLargerThan(ani_env *env, ani_object obj, ani_long sizeKb)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    int64_t size = static_cast<int64_t>(sizeKb);
    const auto& logInfos = container->GetLogInfos();
    std::vector<std::string> files;
    for (const auto& info : logInfos) {
        if (info.sizeInKb > size) {
            files.push_back(info.filePath);
        }
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_object AniExternalLogContainer::AniGetLogFilesSmallerThan(ani_env *env, ani_object obj, ani_long sizeKb)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    int64_t size = static_cast<int64_t>(sizeKb);
    const auto& logInfos = container->GetLogInfos();
    std::vector<std::string> files;
    for (const auto& info : logInfos) {
        if (info.sizeInKb < size) {
            files.push_back(info.filePath);
        }
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_int AniExternalLogContainer::AniGetLogNumber(ani_env *env, ani_object obj)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return 0;
    }
    return static_cast<ani_int>(container->GetLogInfos().size());
}

ani_object AniExternalLogContainer::AniGetFirstGeneratedLogFiles(ani_env *env, ani_object obj, ani_int num)
{
    auto* container = Unwrap(env, obj);
    if (container == nullptr) {
        return nullptr;
    }
    int n = static_cast<int>(num);
    if (n <= 0) {
        ani_ref arr = CreateStringArray(env, std::vector<std::string>());
        return static_cast<ani_object>(arr);
    }
    auto logInfos = container->GetLogInfos();
    std::sort(logInfos.begin(), logInfos.end(),
        [](const ExternalLogWrapperInfo& a, const ExternalLogWrapperInfo& b) {
            return a.generationTime < b.generationTime;
        });
    std::vector<std::string> files;
    for (int i = 0; i < n && i < static_cast<int>(logInfos.size()); ++i) {
        files.push_back(logInfos[i].filePath);
    }
    ani_ref arr = CreateStringArray(env, files);
    return static_cast<ani_object>(arr);
}

ani_object AniExternalLogContainer::CreateAniObject(ani_env *env,
    const std::vector<ExternalLogWrapperInfo>& logInfos)
{
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_EXT_LOG_CONTAINER, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_EXT_LOG_CONTAINER);
        return nullptr;
    }
    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get %{public}s ctor Failed", CLASS_NAME_EXT_LOG_CONTAINER);
        return nullptr;
    }
    ani_object obj {};
    if (env->Object_New(cls, ctor, &obj) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Create Object Failed: %{public}s", CLASS_NAME_EXT_LOG_CONTAINER);
        return nullptr;
    }
    auto* container = new(std::nothrow) AniExternalLogContainer(logInfos);
    if (container != nullptr) {
        Wrap(env, obj, container);
    }
    return obj;
}

} // namespace HiviewDFX
} // namespace OHOS