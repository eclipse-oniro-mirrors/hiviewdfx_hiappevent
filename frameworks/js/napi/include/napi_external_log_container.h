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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_CONTAINER_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_CONTAINER_H

#include <string>
#include <vector>

#include "hiappevent_base.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace HiviewDFX {

class NapiExternalLogContainer {
public:
    NapiExternalLogContainer() = default;
    explicit NapiExternalLogContainer(const std::vector<ExternalLogWrapperInfo>& logInfos);
    ~NapiExternalLogContainer() = default;

    const std::vector<ExternalLogWrapperInfo>& GetLogInfos() const;

    static napi_value NapiConstructor(napi_env env, napi_callback_info info);
    static napi_value NapiExport(napi_env env, napi_value exports);
    static napi_value NapiGetAllLogs(napi_env env, napi_callback_info info);
    static napi_value NapiGetAllLogFiles(napi_env env, napi_callback_info info);
    static napi_value NapiGetLogFilesOfSysEvent(napi_env env, napi_callback_info info);
    static napi_value NapiGetLogFilesGeneratedAfter(napi_env env, napi_callback_info info);
    static napi_value NapiGetLogFilesGeneratedBefore(napi_env env, napi_callback_info info);
    static napi_value NapiGetLogFilesLargerThan(napi_env env, napi_callback_info info);
    static napi_value NapiGetLogFilesSmallerThan(napi_env env, napi_callback_info info);
    static napi_value NapiGetLogNumber(napi_env env, napi_callback_info info);
    static napi_value NapiGetFirstGeneratedLogFiles(napi_env env, napi_callback_info info);

public:
    static thread_local napi_ref constructor_;

private:
    std::vector<ExternalLogWrapperInfo> logInfos_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_CONTAINER_H