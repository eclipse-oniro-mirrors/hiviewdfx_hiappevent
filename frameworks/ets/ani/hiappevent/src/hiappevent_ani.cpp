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

#include "hiappevent_ani.h"
#include "hiappevent_ani_error_code.h"
#include "hiappevent_param_builder.h"
#include "ani_app_event_holder.h"

#include "app_event_stat.h"
#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hilog/log.h"
#include "hilog/log_cpp.h"
#include "time_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_ANI"

using namespace OHOS::HiviewDFX;
namespace {
int32_t BuildEventConfig(ani_env *env, ani_object config, std::map<std::string, std::string>& eventConfigMap)
{
    std::map<std::string, ani_ref> eventConfig;
    HiAppEventAniUtil::ParseRecord(env, config, eventConfig);
    for (const auto &configPair : eventConfig) {
        eventConfigMap[configPair.first] = HiAppEventAniUtil::ConvertToString(env, configPair.second);
    }
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}
}

ani_double HiAppEventAni::AddProcessor(ani_env *env, ani_object processor)
{
    int64_t id = 0;
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.AddProcessor(env, processor, id)) {
        HILOG_ERROR(LOG_CORE, "failed to add processor");
    }
    return static_cast<ani_double>(id);
}

ani_object HiAppEventAni::Write(ani_env *env, ani_object info)
{
    std::string domain = "";
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.GetPropertyDomain(info, env, domain)) {
        HILOG_ERROR(LOG_CORE, "get property domain failed");
        return HiAppEventAniUtil::Result(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("domain")});
    }

    std::string name = "";
    if (!hiAppEventAniHelper.GetPropertyName(info, env, name)) {
        HILOG_ERROR(LOG_CORE, "get property name failed");
        return HiAppEventAniUtil::Result(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("name")});
    }

    int32_t enumValue = 0;
    if (!hiAppEventAniHelper.GeteventTypeValue(info, env, enumValue)) {
        HILOG_ERROR(LOG_CORE, "get eventType value failed");
        return HiAppEventAniUtil::Result(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("eventType")});
    }
    if (!IsValidEventType(enumValue)) {
        HILOG_ERROR(LOG_CORE, "eventType value range error");
        return HiAppEventAniUtil::Result(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("eventType")});
    }

    ani_ref paramTemp {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "params", &paramTemp)) {
        HILOG_ERROR(LOG_CORE, "get property params failed");
        return HiAppEventAniUtil::Result(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("params")});
    }

    auto appEventPack = std::make_shared<AppEventPack>(domain, name, enumValue);
    if (!hiAppEventAniHelper.ParseParamsInAppEventPack(env, paramTemp, appEventPack)) {
        HILOG_ERROR(LOG_CORE, "parse params appEventPack failed");
        return HiAppEventAniUtil::Result(env,
            HiAppEventAniUtil::BuildErrorByResult(hiAppEventAniHelper.GetResult()));
    }

    int32_t result = hiAppEventAniHelper.GetResult();
    if (result >= 0) {
        if (auto ret = VerifyAppEvent(appEventPack); ret != 0) {
            result = ret;
        }
    }
    if (result >= 0) {
        WriteEvent(appEventPack);
    }
    return HiAppEventAniUtil::Result(env, HiAppEventAniUtil::BuildErrorByResult(result));
}

void HiAppEventAni::Configure(ani_env *env, ani_object configObj)
{
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.Configure(env, configObj)) {
        HILOG_ERROR(LOG_CORE, "failed to configure HiAppEvent");
    }
}

ani_object HiAppEventAni::SetEventParamSync(ani_env *env, ani_object params, ani_string domain, ani_object name)
{
    std::string domainTemp = HiAppEventAniUtil::ParseStringValue(env, domain);
    std::string nameTemp = "";
    if (!HiAppEventAniUtil::IsRefUndefined(env, name)) {
        nameTemp = HiAppEventAniUtil::ParseStringValue(env, name);
    }

    auto appEventPack = std::make_shared<AppEventPack>(domainTemp, nameTemp);
    HiAppEventParamBuilder hiAppEventParamBuilder;
    if (!hiAppEventParamBuilder.ParseParamsInAppEventPack(env, params, appEventPack)) {
        HILOG_ERROR(LOG_CORE, "parse params appEventPack failed");
        return HiAppEventAniUtil::Result(env,
            HiAppEventAniUtil::BuildErrorByResult(hiAppEventParamBuilder.GetResult()));
    }

    int32_t result = hiAppEventParamBuilder.GetResult();
    if (result >= 0) {
        if (auto retTemp = VerifyCustomEventParams(appEventPack); retTemp != 0) {
            result = retTemp;
        }
    }
    if (result == 0) {
        if (auto ret = SetEventParam(appEventPack); ret > 0) {
            result = ret;
        }
    }
    return HiAppEventAniUtil::Result(env, HiAppEventAniUtil::BuildErrorByResult(result));
}

ani_object HiAppEventAni::SetEventConfigSync(ani_env *env, ani_string name, ani_object config)
{
    std::string nameString = HiAppEventAniUtil::ParseStringValue(env, name);
    std::map<std::string, std::string> eventConfigMap;
    int32_t result = BuildEventConfig(env, config, eventConfigMap);
    if (result != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL || eventConfigMap.empty()) {
        HILOG_ERROR(LOG_CORE, "the param type is invalid or the config is empty.");
        return HiAppEventAniUtil::Result(env, {result, "the param type is invalid or the config is empty."});
    }
    result = HiviewDFX::SetEventConfig(nameString, eventConfigMap);
    if (result == 0) {
        return HiAppEventAniUtil::Result(env, HiAppEventAniUtil::BuildErrorByResult(result));
    } else {
        return HiAppEventAniUtil::Result(env, {ERR_PARAM, "Invalid param value for event config."});
    }
}

void HiAppEventAni::ClearData([[maybe_unused]] ani_env *env)
{
    uint64_t beginTime = static_cast<uint64_t>(TimeUtil::GetElapsedMilliSecondsSinceBoot());
    HiAppEventClean::ClearData(HiAppEventConfig::GetInstance().GetStorageDir());
    AppEventStat::WriteApiEndEventAsync("clearData", beginTime, AppEventStat::SUCCESS, ERR_CODE_SUCC);
}

void HiAppEventAni::SetUserId(ani_env *env, ani_string name, ani_string value)
{
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.SetUserId(env, name, value)) {
        HILOG_ERROR(LOG_CORE, "failed to set userId");
    }
}

ani_string HiAppEventAni::GetUserId(ani_env *env, ani_string name)
{
    ani_string userId {};
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.GetUserId(env, name, userId)) {
        HILOG_ERROR(LOG_CORE, "failed to get userId");
    }
    return userId;
}

void HiAppEventAni::SetUserProperty(ani_env *env, ani_string name, ani_string value)
{
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.SetUserProperty(env, name, value)) {
        HILOG_ERROR(LOG_CORE, "failed to set userProperty");
    }
}

ani_string HiAppEventAni::GetUserProperty(ani_env *env, ani_string name)
{
    ani_string userProperty {};
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.GetUserProperty(env, name, userProperty)) {
        HILOG_ERROR(LOG_CORE, "failed to get userProperty");
    }
    return userProperty;
}

void HiAppEventAni::RemoveProcessor(ani_env *env, ani_double id)
{
    HiAppEventAniHelper hiAppEventAniHelper;
    if (!hiAppEventAniHelper.RemoveProcessor(env, id)) {
        HILOG_ERROR(LOG_CORE, "failed to remove processor");
    }
}

ani_object HiAppEventAni::AddWatcher(ani_env *env, ani_object watcher)
{
    uint64_t beginTime = static_cast<uint64_t>(TimeUtil::GetElapsedMilliSecondsSinceBoot());
    HiAppEventAniHelper hiAppEventAniHelper;
    return hiAppEventAniHelper.AddWatcher(env, watcher, beginTime);
}

void HiAppEventAni::RemoveWatcher(ani_env *env, ani_object watcher)
{
    uint64_t beginTime = static_cast<uint64_t>(TimeUtil::GetElapsedMilliSecondsSinceBoot());
    HiAppEventAniHelper hiAppEventAniHelper;
    hiAppEventAniHelper.RemoveWatcher(env, watcher, beginTime);
}

static ani_status BindEventFunction(ani_env *env)
{
    ani_namespace  ns {};
    if (ANI_OK != env->FindNamespace(NAMESPACE_NAME_HIAPPEVENT, &ns)) {
        return ANI_INVALID_ARGS;
    }
    std::array methods = {
        ani_native_function {"writeSync", nullptr, reinterpret_cast<void *>(HiAppEventAni::Write)},
        ani_native_function {"addProcessor", nullptr, reinterpret_cast<void *>(HiAppEventAni::AddProcessor)},
        ani_native_function {"configure", nullptr, reinterpret_cast<void *>(HiAppEventAni::Configure)},
        ani_native_function {"setEventParamSync",
            nullptr, reinterpret_cast<void *>(HiAppEventAni::SetEventParamSync)},
        ani_native_function {"setEventConfigSync",
            nullptr, reinterpret_cast<void *>(HiAppEventAni::SetEventConfigSync)},
        ani_native_function {"clearData", nullptr, reinterpret_cast<void *>(HiAppEventAni::ClearData)},
        ani_native_function {"setUserId", nullptr, reinterpret_cast<void *>(HiAppEventAni::SetUserId)},
        ani_native_function {"getUserId", nullptr, reinterpret_cast<void *>(HiAppEventAni::GetUserId)},
        ani_native_function {"setUserProperty", nullptr, reinterpret_cast<void *>(HiAppEventAni::SetUserProperty)},
        ani_native_function {"getUserProperty", nullptr, reinterpret_cast<void *>(HiAppEventAni::GetUserProperty)},
        ani_native_function {"removeProcessor", nullptr, reinterpret_cast<void *>(HiAppEventAni::RemoveProcessor)},
        ani_native_function {"addWatcher", nullptr, reinterpret_cast<void *>(HiAppEventAni::AddWatcher)},
        ani_native_function {"removeWatcher", nullptr, reinterpret_cast<void *>(HiAppEventAni::RemoveWatcher)},
    };
    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size())) {
        return ANI_INVALID_TYPE;
    };
    return ANI_OK;
}

static ani_status BindHolderFunction(ani_env *env)
{
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_EVENT_PACKAGE_HOLDER, &cls)) {
        return ANI_INVALID_ARGS;
    }
    std::array methods = {
        ani_native_function {"nativeConstructor", nullptr,
            reinterpret_cast<void *>(AniAppEventHolder::AniConstructor)},
        ani_native_function {"_finalize", nullptr, reinterpret_cast<void *>(AniAppEventHolder::AniFinalize)},
        ani_native_function {"setSize", nullptr, reinterpret_cast<void *>(AniAppEventHolder::AniSetSize)},
        ani_native_function {"setRow", nullptr, reinterpret_cast<void *>(AniAppEventHolder::AniSetRow)},
        ani_native_function {"takeNext", nullptr, reinterpret_cast<void *>(AniAppEventHolder::AniTakeNext)},
    };
    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        return ANI_INVALID_TYPE;
    };
    return ANI_OK;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env = nullptr;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        HILOG_ERROR(LOG_CORE, "Unsupported ANI_VERSION_1");
        return ANI_OUT_OF_REF;
    }

    if (ANI_OK != BindEventFunction(env)) {
        return BindEventFunction(env);
    }

    if (ANI_OK != BindHolderFunction(env)) {
        return BindHolderFunction(env);
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}
