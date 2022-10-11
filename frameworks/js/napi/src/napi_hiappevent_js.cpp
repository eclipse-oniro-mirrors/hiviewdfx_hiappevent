/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "napi_hiappevent_builder.h"
#include "napi_hiappevent_config.h"
#include "napi_hiappevent_init.h"
#include "napi_hiappevent_write.h"
#include "napi_util.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_NAPI" };
}

static napi_value Write(napi_env env, napi_callback_info info)
{
    constexpr size_t PARAM_NUM = 4; // max param num is 4
    size_t paramNum = PARAM_NUM;
    napi_value params[PARAM_NUM] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, nullptr, nullptr));

    auto asyncContext = new(std::nothrow) NapiHiAppEventWrite::HiAppEventAsyncContext(env);
    if (asyncContext == nullptr) {
        HiLog::Error(LABEL, "failed to new asyncContext.");
        return NapiUtil::CreateUndefined(env);
    }

    // 1. build AppEventPack object
    NapiHiAppEventBuilder builder;
    asyncContext->appEventPack = builder.Build(env, params, paramNum);
    asyncContext->result = builder.GetResult();
    asyncContext->callback = builder.GetCallback();

    // 2. if the build is successful, the event verification is performed
    if (asyncContext->result == 0) {
        asyncContext->result = VerifyAppEvent(asyncContext->appEventPack);
    }

    // 3. set promise object if callback is null
    napi_value promise = NapiUtil::CreateUndefined(env);
    if (asyncContext->callback == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    }

    // 4. try to write the event to file
    NapiHiAppEventWrite::Write(env, asyncContext);
    return promise;
}

static napi_value Configure(napi_env env, napi_callback_info info)
{
    constexpr size_t PARAM_NUM = 1; // param num is 1
    size_t paramNum = PARAM_NUM;
    napi_value params[PARAM_NUM] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, nullptr, nullptr));
    if (paramNum != PARAM_NUM) {
        HiLog::Error(LABEL, "failed to check the number of configure param");
        return NapiUtil::CreateBoolean(env, false);
    }
    return NapiUtil::CreateBoolean(env, NapiHiAppEventConfig::Configure(env, params[0]));
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("configure", Configure)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));

    // init EventType class, Event class and Param class
    NapiHiAppEventInit::InitNapiClass(env, exports);
    return exports;
}
EXTERN_C_END

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "hiAppEvent",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
