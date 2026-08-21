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
#include "napi_external_log_wrapper.h"

#include "hilog/log.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "NapiExtLogWrapper"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char WRAPPER_CLASS_NAME[] = "ExternalLogWrapper";
}

thread_local napi_ref NapiExternalLogWrapper::constructor_ = nullptr;

NapiExternalLogWrapper::NapiExternalLogWrapper(const ExternalLogWrapperInfo& info) : info_(info) {}

std::string NapiExternalLogWrapper::GetFilePath() const
{
    return info_.filePath;
}

int64_t NapiExternalLogWrapper::GetGenerationTime() const
{
    return info_.generationTime;
}

int64_t NapiExternalLogWrapper::GetSizeInKb() const
{
    return info_.sizeInKb;
}

std::string NapiExternalLogWrapper::GetSysEvent() const
{
    return info_.sysEvent;
}

napi_value NapiExternalLogWrapper::NapiConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    // C++ object is created externally and set via napi_wrap by the caller
    return thisVar;
}

napi_value NapiExternalLogWrapper::NapiExport(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getFilePath", NapiGetFilePath),
        DECLARE_NAPI_FUNCTION("getGenerationTime", NapiGetGenerationTime),
        DECLARE_NAPI_FUNCTION("getSizeInKb", NapiGetSizeInKb),
        DECLARE_NAPI_FUNCTION("getSysEvent", NapiGetSysEvent)
    };
    napi_value wrapperClass = nullptr;
    napi_define_class(env, WRAPPER_CLASS_NAME, strlen(WRAPPER_CLASS_NAME), NapiConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &wrapperClass);
    NapiUtil::SetNamedProperty(env, exports, WRAPPER_CLASS_NAME, wrapperClass);
    constructor_ = NapiUtil::CreateReference(env, wrapperClass);
    return exports;
}

napi_value NapiExternalLogWrapper::NapiGetFilePath(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogWrapper* wrapper = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&wrapper) != napi_ok || wrapper == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    return NapiUtil::CreateString(env, wrapper->GetFilePath());
}

napi_value NapiExternalLogWrapper::NapiGetGenerationTime(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogWrapper* wrapper = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&wrapper) != napi_ok || wrapper == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    return NapiUtil::CreateInt64(env, wrapper->GetGenerationTime());
}

napi_value NapiExternalLogWrapper::NapiGetSizeInKb(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogWrapper* wrapper = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&wrapper) != napi_ok || wrapper == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    return NapiUtil::CreateInt64(env, wrapper->GetSizeInKb());
}

napi_value NapiExternalLogWrapper::NapiGetSysEvent(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogWrapper* wrapper = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&wrapper) != napi_ok || wrapper == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    return NapiUtil::CreateString(env, wrapper->GetSysEvent());
}

} // namespace HiviewDFX
} // namespace OHOS