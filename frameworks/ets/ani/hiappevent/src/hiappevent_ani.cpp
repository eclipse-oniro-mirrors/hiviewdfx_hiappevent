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
#include "hiappevent_ani_helper.h"
#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "hilog/log_cpp.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_ANI"

using namespace OHOS::HiviewDFX;

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
        return hiAppEventAniHelper.WriteResult(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("domain")});
    }

    std::string name = "";
    if (!hiAppEventAniHelper.GetPropertyName(info, env, name)) {
        HILOG_ERROR(LOG_CORE, "get property name failed");
        return hiAppEventAniHelper.WriteResult(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("name")});
    }

    int32_t enumValue = 0;
    if (!hiAppEventAniHelper.GeteventTypeValue(info, env, enumValue)) {
        HILOG_ERROR(LOG_CORE, "get eventType value failed");
        return hiAppEventAniHelper.WriteResult(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("eventType")});
    }
    if (!IsValidEventType(enumValue)) {
        HILOG_ERROR(LOG_CORE, "eventType value range error");
        return hiAppEventAniHelper.WriteResult(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("eventType")});
    }

    ani_ref paramTemp {};
    if (env->Object_GetPropertyByName_Ref(info, "params", &paramTemp) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get property params failed");
        return hiAppEventAniHelper.WriteResult(env, {ERR_PARAM, HiAppEventAniUtil::CreateErrMsg("params")});
    }

    auto appEventPack = std::make_shared<AppEventPack>(domain, name, enumValue);
    if (!hiAppEventAniHelper.ParseParamsInAppEventPack(env, paramTemp, appEventPack)) {
        HILOG_ERROR(LOG_CORE, "parse params appEventPack failed");
        return hiAppEventAniHelper.WriteResult(env,
            hiAppEventAniHelper.BuildErrorByResult(hiAppEventAniHelper.GetResult()));
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
    return hiAppEventAniHelper.WriteResult(env, hiAppEventAniHelper.BuildErrorByResult(result));
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env)  != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Unsupported ANI_VERSION_1");
        return ANI_OUT_OF_REF;
    }

    ani_namespace  ns {};
    if (env->FindNamespace(NAMESPACE_NAME_HIAPPEVENT, &ns)  != ANI_OK) {
        return ANI_INVALID_ARGS;
    }

    std::array methods = {
        ani_native_function {"writeSync", nullptr, reinterpret_cast<void *>(HiAppEventAni::Write) },
        ani_native_function {"addProcessor", nullptr, reinterpret_cast<void *>(HiAppEventAni::AddProcessor) },
    };

    if (env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size()) != ANI_OK) {
        return ANI_INVALID_TYPE;
    };

    *result = ANI_VERSION_1;
    return ANI_OK;
}
