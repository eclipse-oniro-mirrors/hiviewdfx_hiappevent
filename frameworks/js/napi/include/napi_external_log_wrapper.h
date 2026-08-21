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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_WRAPPER_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_WRAPPER_H

#include <string>

#include "hiappevent_base.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace HiviewDFX {

class NapiExternalLogWrapper {
public:
    NapiExternalLogWrapper() = default;
    explicit NapiExternalLogWrapper(const ExternalLogWrapperInfo& info);
    ~NapiExternalLogWrapper() = default;

    std::string GetFilePath() const;
    int64_t GetGenerationTime() const;
    int64_t GetSizeInKb() const;
    std::string GetSysEvent() const;

    static napi_value NapiConstructor(napi_env env, napi_callback_info info);
    static napi_value NapiExport(napi_env env, napi_value exports);
    static napi_value NapiGetFilePath(napi_env env, napi_callback_info info);
    static napi_value NapiGetGenerationTime(napi_env env, napi_callback_info info);
    static napi_value NapiGetSizeInKb(napi_env env, napi_callback_info info);
    static napi_value NapiGetSysEvent(napi_env env, napi_callback_info info);

public:
    static thread_local napi_ref constructor_;

private:
    ExternalLogWrapperInfo info_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_WRAPPER_H