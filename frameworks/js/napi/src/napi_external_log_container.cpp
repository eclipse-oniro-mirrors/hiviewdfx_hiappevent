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
#include "napi_external_log_container.h"

#include <algorithm>

#include "hilog/log.h"
#include "napi_error.h"
#include "napi_external_log_wrapper.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "NapiExtLogContainer"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr char CONTAINER_CLASS_NAME[] = "ExternalLogContainer";
constexpr size_t PARAM_NUM_ONE = 1;
}

thread_local napi_ref NapiExternalLogContainer::constructor_ = nullptr;

NapiExternalLogContainer::NapiExternalLogContainer(const std::vector<ExternalLogWrapperInfo>& logInfos)
    : logInfos_(logInfos) {}

const std::vector<ExternalLogWrapperInfo>& NapiExternalLogContainer::GetLogInfos() const
{
    return logInfos_;
}

napi_value NapiExternalLogContainer::NapiConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    return thisVar;
}

napi_value NapiExternalLogContainer::NapiExport(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getAllLogs", NapiGetAllLogs),
        DECLARE_NAPI_FUNCTION("getAllLogFiles", NapiGetAllLogFiles),
        DECLARE_NAPI_FUNCTION("getLogFilesOfSysEvent", NapiGetLogFilesOfSysEvent),
        DECLARE_NAPI_FUNCTION("getLogFilesGeneratedAfter", NapiGetLogFilesGeneratedAfter),
        DECLARE_NAPI_FUNCTION("getLogFilesGeneratedBefore", NapiGetLogFilesGeneratedBefore),
        DECLARE_NAPI_FUNCTION("getLogFilesLargerThan", NapiGetLogFilesLargerThan),
        DECLARE_NAPI_FUNCTION("getLogFilesSmallerThan", NapiGetLogFilesSmallerThan),
        DECLARE_NAPI_FUNCTION("getLogNumber", NapiGetLogNumber),
        DECLARE_NAPI_FUNCTION("getFirstGeneratedLogFiles", NapiGetFirstGeneratedLogFiles)
    };
    napi_value containerClass = nullptr;
    napi_define_class(env, CONTAINER_CLASS_NAME, strlen(CONTAINER_CLASS_NAME), NapiConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &containerClass);
    NapiUtil::SetNamedProperty(env, exports, CONTAINER_CLASS_NAME, containerClass);
    constructor_ = NapiUtil::CreateReference(env, containerClass);
    return exports;
}

napi_value NapiExternalLogContainer::NapiGetAllLogs(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    napi_value result = NapiUtil::CreateArray(env);
    uint32_t index = 0;
    for (const auto& logInfo : container->logInfos_) {
        napi_value wrapperObj = nullptr;
        napi_value constructor = NapiUtil::GetReferenceValue(env, NapiExternalLogWrapper::constructor_);
        if (constructor == nullptr || napi_new_instance(env, constructor, 0, nullptr, &wrapperObj) != napi_ok) {
            continue;
        }
        auto* wrapper = new NapiExternalLogWrapper(logInfo);
        napi_wrap(env, wrapperObj, wrapper,
            [](napi_env env, void* data, void* hint) {
                delete static_cast<NapiExternalLogWrapper*>(data);
            },
            nullptr, nullptr);
        NapiUtil::SetElement(env, result, index++, wrapperObj);
    }
    return result;
}

napi_value NapiExternalLogContainer::NapiGetAllLogFiles(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    std::vector<std::string> paths;
    for (const auto& logInfo : container->logInfos_) {
        paths.emplace_back(logInfo.filePath);
    }
    return NapiUtil::CreateStrings(env, paths);
}

napi_value NapiExternalLogContainer::NapiGetLogFilesOfSysEvent(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM_ONE;
    napi_value params[PARAM_NUM_ONE] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM_ONE || !NapiUtil::IsString(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("event", "string"));
        return NapiUtil::CreateArray(env);
    }
    std::string event = NapiUtil::GetString(env, params[0]);

    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    std::vector<std::string> paths;
    for (const auto& logInfo : container->logInfos_) {
        if (logInfo.sysEvent == event) {
            paths.emplace_back(logInfo.filePath);
        }
    }
    return NapiUtil::CreateStrings(env, paths);
}

napi_value NapiExternalLogContainer::NapiGetLogFilesGeneratedAfter(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM_ONE;
    napi_value params[PARAM_NUM_ONE] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM_ONE || !NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("timePoint", "number"));
        return NapiUtil::CreateArray(env);
    }
    int64_t timePoint = NapiUtil::GetInt64(env, params[0]);

    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    std::vector<std::string> paths;
    for (const auto& logInfo : container->logInfos_) {
        if (logInfo.generationTime > timePoint) {
            paths.emplace_back(logInfo.filePath);
        }
    }
    return NapiUtil::CreateStrings(env, paths);
}

napi_value NapiExternalLogContainer::NapiGetLogFilesGeneratedBefore(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM_ONE;
    napi_value params[PARAM_NUM_ONE] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM_ONE || !NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("timePoint", "number"));
        return NapiUtil::CreateArray(env);
    }
    int64_t timePoint = NapiUtil::GetInt64(env, params[0]);

    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    std::vector<std::string> paths;
    for (const auto& logInfo : container->logInfos_) {
        if (logInfo.generationTime < timePoint) {
            paths.emplace_back(logInfo.filePath);
        }
    }
    return NapiUtil::CreateStrings(env, paths);
}

napi_value NapiExternalLogContainer::NapiGetLogFilesLargerThan(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM_ONE;
    napi_value params[PARAM_NUM_ONE] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM_ONE || !NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("sizeKb", "number"));
        return NapiUtil::CreateArray(env);
    }
    int64_t sizeKb = NapiUtil::GetInt64(env, params[0]);

    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    std::vector<std::string> paths;
    for (const auto& logInfo : container->logInfos_) {
        if (logInfo.sizeInKb > sizeKb) {
            paths.emplace_back(logInfo.filePath);
        }
    }
    return NapiUtil::CreateStrings(env, paths);
}

napi_value NapiExternalLogContainer::NapiGetLogFilesSmallerThan(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM_ONE;
    napi_value params[PARAM_NUM_ONE] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM_ONE || !NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("sizeKb", "number"));
        return NapiUtil::CreateArray(env);
    }
    int64_t sizeKb = NapiUtil::GetInt64(env, params[0]);

    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    std::vector<std::string> paths;
    for (const auto& logInfo : container->logInfos_) {
        if (logInfo.sizeInKb < sizeKb) {
            paths.emplace_back(logInfo.filePath);
        }
    }
    return NapiUtil::CreateStrings(env, paths);
}

napi_value NapiExternalLogContainer::NapiGetLogNumber(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateInt32(env, 0);
    }
    return NapiUtil::CreateInt32(env, static_cast<int32_t>(container->logInfos_.size()));
}

napi_value NapiExternalLogContainer::NapiGetFirstGeneratedLogFiles(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM_ONE;
    napi_value params[PARAM_NUM_ONE] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM_ONE || !NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("num", "number"));
        return NapiUtil::CreateArray(env);
    }
    int32_t num = NapiUtil::GetInt32(env, params[0]);

    NapiExternalLogContainer* container = nullptr;
    if (napi_unwrap(env, thisVar, (void**)&container) != napi_ok || container == nullptr) {
        return NapiUtil::CreateArray(env);
    }

    // Sort by generation time ascending
    auto sorted = container->logInfos_;
    std::sort(sorted.begin(), sorted.end(),
        [](const ExternalLogWrapperInfo& a, const ExternalLogWrapperInfo& b) {
            return a.generationTime < b.generationTime;
        });

    std::vector<std::string> paths;
    for (int32_t i = 0; i < num && i < static_cast<int32_t>(sorted.size()); ++i) {
        paths.emplace_back(sorted[i].filePath);
    }
    return NapiUtil::CreateStrings(env, paths);
}

} // namespace HiviewDFX
} // namespace OHOS