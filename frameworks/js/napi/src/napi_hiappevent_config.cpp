/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "napi_hiappevent_config.h"

#include <map>
#include <string>

#include "hiappevent_config.h"
#include "hilog/log.h"
#include "napi_config_builder.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiHiAppEventConfig"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventConfig {
namespace {
constexpr int ERROR_SET_FAILED = -1;
constexpr const char* const APP_CRASH = "APP_CRASH";
constexpr const char* const MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
const std::map<std::string, napi_valuetype> CONFIG_OPTION_MAP = {
    { "disable", napi_boolean },
    { "maxStorage", napi_string },
};

int SetEventConfigSync(HiAppEventConfigAsyncContext* asyncContext)
{
    if (asyncContext->eventConfigPack->eventName == APP_CRASH) {
        return HiAppEventConfig::GetInstance().SetCrashConfig(asyncContext->eventConfigPack->configUintMap);
    } else if (asyncContext->eventConfigPack->eventName == MAIN_THREAD_JANK) {
        return HiAppEventConfig::GetInstance().SetEventConfig(MAIN_THREAD_JANK,
            asyncContext->eventConfigPack->configStringMap);
    }
    HILOG_ERROR(LOG_CORE, "Failed to set event config, name is invalid. name=%{public}s",
        asyncContext->eventConfigPack->eventName.c_str());
    return ERROR_SET_FAILED;
}
}
bool Configure(const napi_env env, const napi_value configObj, bool isThrow)
{
    if (!NapiUtil::IsObject(env, configObj)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("config", "ConfigOption"), isThrow);
        return false;
    }

    std::vector<std::string> keys;
    NapiUtil::GetPropertyNames(env, configObj, keys);
    for (auto key : keys) {
        if (CONFIG_OPTION_MAP.find(key) == CONFIG_OPTION_MAP.end()) {
            continue;
        }
        napi_value value = NapiUtil::GetProperty(env, configObj, key);
        if (CONFIG_OPTION_MAP.at(key) != NapiUtil::GetType(env, value)) {
            std::string errMsg = NapiUtil::CreateErrMsg(key, CONFIG_OPTION_MAP.at(key));
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isThrow);
            return false;
        }
        if (!HiAppEventConfig::GetInstance().SetConfigurationItem(key, NapiUtil::ConvertToString(env, value))) {
            std::string errMsg = "Invalid max storage quota value. Possible caused by incorrectly formatted.";
            NapiUtil::ThrowError(env, NapiError::ERR_INVALID_MAX_STORAGE, errMsg, isThrow);
            return false;
        }
    }
    return true;
}

bool IsDisable()
{
    return HiAppEventConfig::GetInstance().GetDisable();
}

std::string GetStorageDir()
{
    return HiAppEventConfig::GetInstance().GetStorageDir();
}

void SetEventConfig(const napi_env env, HiAppEventConfigAsyncContext* asyncContext)
{
    napi_value resource = NapiUtil::CreateString(env, "NapiHiAppEventSetEventConfig");
    napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventConfigAsyncContext* asyncContext = (HiAppEventConfigAsyncContext*)data;
            asyncContext->result = asyncContext->eventConfigPack->isValid ? SetEventConfigSync(asyncContext) :
                ERROR_SET_FAILED;
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventConfigAsyncContext* asyncContext = (HiAppEventConfigAsyncContext*)data;
            napi_value result = nullptr;
            if (asyncContext != nullptr && asyncContext->deferred != nullptr) { // promise
                if (asyncContext->result == 0) {
                    result = NapiUtil::CreateInt32(env, asyncContext->result);
                    napi_resolve_deferred(env, asyncContext->deferred, result);
                } else {
                    result = NapiUtil::CreateError(env, NapiError::ERR_PARAM, "Invalid param value for event config.");
                    napi_reject_deferred(env, asyncContext->deferred, result);
                }
            }
            napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->asyncWork);
    napi_queue_async_work_with_qos(env, asyncContext->asyncWork, napi_qos_default);
}
} // namespace NapiHiAppEventConfig
} // namespace HiviewDFX
} // namespace OHOS
