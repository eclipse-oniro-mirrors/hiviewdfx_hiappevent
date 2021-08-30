/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <string>

#include "hiappevent_base.h"
#include "hiappevent_pack.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_hiappevent_helper.h"
#include "native_hiappevent_init.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::ErrorCode;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_NAPI" };
constexpr int CONFIGURE_PARAM_NUM = 2;
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    size_t paramNum = WRITE_FUNC_MAX_PARAM_NUM;
    napi_value params[WRITE_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    HiAppEventAsyncContext* asyncContext = new HiAppEventAsyncContext {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };

    // set event file dirtory
    SetStorageDir(env, info);

    // check the number and type of parameters
    int32_t result = CheckWriteParamsType(env, params, (int)paramNum);

    // set callback function
    int paramEndIndex = paramNum;
    if (paramNum > MIN_PARAM_NUM) {
        napi_valuetype lastParamType;
        napi_typeof(env, params[paramNum - MIN_PARAM_NUM], &lastParamType);
        if (lastParamType == napi_valuetype::napi_function) {
            napi_create_reference(env, params[paramNum - MIN_PARAM_NUM], 1, &asyncContext->callback);
            paramEndIndex -= MIN_PARAM_NUM;
        }
    }

    // set promise object
    napi_value promise = nullptr;
    napi_get_undefined(env, &promise);
    if (asyncContext->callback == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    }

    if (result == SUCCESS_FLAG) {
        std::shared_ptr<AppEventPack> appEventPack = CreateEventPackFromNapiValue(env, params[EVENT_NAME_INDEX],
            params[EVENT_TYPE_INDEX]);
        if (paramEndIndex > WRITE_FUNC_MIN_PARAM_NUM) {
            int buildRes = BuildAppEventPack(env, params, paramEndIndex, appEventPack);
            result = buildRes == SUCCESS_FLAG ? result : buildRes;
        }
        asyncContext->appEventPack = appEventPack;

        int verifyResult = VerifyAppEvent(appEventPack);
        if (verifyResult != SUCCESS_FLAG) {
            HiLog::Warn(LABEL, "event verify failed.");
            result = verifyResult;
        }
    }

    asyncContext->result = result;
    AsyncWriteEvent(env, asyncContext);
    return promise;
}

static napi_value WriteJson(napi_env env, napi_callback_info info)
{
    size_t paramNum = WRITE_JSON_FUNC_MAX_PARAM_NUM;
    napi_value params[WRITE_JSON_FUNC_MAX_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    HiAppEventAsyncContext* asyncContext = new HiAppEventAsyncContext {
        .env = env,
        .asyncWork = nullptr,
        .deferred = nullptr,
    };

    // set event file dirtory
    SetStorageDir(env, info);

    // check the number and type of parameters
    int32_t result = CheckWriteJsonParamsType(env, params, (int)paramNum);

    // set callback function
    if (paramNum > MIN_PARAM_NUM) {
        napi_valuetype lastParamType;
        napi_typeof(env, params[paramNum - MIN_PARAM_NUM], &lastParamType);
        if (lastParamType == napi_valuetype::napi_function) {
            napi_create_reference(env, params[paramNum - MIN_PARAM_NUM], 1, &asyncContext->callback);
        }
    }

    // set promise object
    napi_value promise = nullptr;
    napi_get_undefined(env, &promise);
    if (asyncContext->callback == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    }

    if (result == SUCCESS_FLAG) {
        std::shared_ptr<AppEventPack> appEventPack = CreateEventPackFromNapiValue(env, params[EVENT_NAME_INDEX],
            params[EVENT_TYPE_INDEX]);
        int buildRes = BuildAppEventPackFromObject(env, params[JSON_OBJECT_INDEX], appEventPack);
        result = buildRes == SUCCESS_FLAG ? result : buildRes;
        asyncContext->appEventPack = appEventPack;

        int verifyResult = VerifyAppEvent(appEventPack);
        if (verifyResult != SUCCESS_FLAG) {
            HiLog::Warn(LABEL, "event verify failed.");
            result = verifyResult;
        }
    }

    asyncContext->result = result;
    AsyncWriteEvent(env, asyncContext);
    return promise;
}

static napi_value Configure(napi_env env, napi_callback_info info)
{
    size_t paramNum = CONFIGURE_PARAM_NUM;
    napi_value params[CONFIGURE_PARAM_NUM] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisArg, &data));

    napi_value result = nullptr;
    napi_get_boolean(env, false, &result);
    if (paramNum != CONFIGURE_PARAM_NUM) {
        HiLog::Error(LABEL, "failed to configure event manager, invalid number of params.");
        return result;
    }

    napi_get_boolean(env, ConfigureFromNapiValue(env, params[0], params[1]), &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_value disable = nullptr;
    napi_create_string_utf8(env, "disable", NAPI_AUTO_LENGTH, &disable);
    napi_value maxStorage = nullptr;
    napi_create_string_utf8(env, "max_storage", NAPI_AUTO_LENGTH, &maxStorage);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("writeJson", WriteJson),
        DECLARE_NAPI_FUNCTION("configure", Configure),
        DECLARE_NAPI_STATIC_PROPERTY("DISABLE", disable),
        DECLARE_NAPI_STATIC_PROPERTY("MAX_STORAGE", maxStorage)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));

    // init EventType class, Event class and Param class
    InitNapiClass(env, exports);

    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hiappevent",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}