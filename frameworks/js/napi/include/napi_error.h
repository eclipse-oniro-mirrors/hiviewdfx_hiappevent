/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ERROR_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ERROR_H

#include <string>

#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiError {
constexpr int ERR_OK = JsErrorCode::ERR_OK;
// common api error
constexpr int ERR_PARAM = JsErrorCode::ERR_PARAM;

// business error of write function
constexpr int ERR_DISABLE = JsErrorCode::ERR_DISABLE;
constexpr int ERR_INVALID_DOMAIN = JsErrorCode::ERR_INVALID_DOMAIN;
constexpr int ERR_INVALID_NAME = JsErrorCode::ERR_INVALID_NAME;
constexpr int ERR_INVALID_PARAM_NUM = JsErrorCode::ERR_INVALID_PARAM_NUM;
constexpr int ERR_INVALID_STR_LEN = JsErrorCode::ERR_INVALID_STR_LEN;
constexpr int ERR_INVALID_KEY = JsErrorCode::ERR_INVALID_KEY;
constexpr int ERR_INVALID_ARR_LEN = JsErrorCode::ERR_INVALID_ARR_LEN;
constexpr int ERR_INVALID_CUSTOM_PARAM_NUM = JsErrorCode::ERR_INVALID_CUSTOM_PARAM_NUM;

// business error of addWatcher function
constexpr int ERR_INVALID_WATCHER_NAME = JsErrorCode::ERR_INVALID_WATCHER_NAME;
constexpr int ERR_INVALID_FILTER_DOMAIN = JsErrorCode::ERR_INVALID_FILTER_DOMAIN;
constexpr int ERR_INVALID_COND_ROW = JsErrorCode::ERR_INVALID_COND_ROW;
constexpr int ERR_INVALID_COND_SIZE = JsErrorCode::ERR_INVALID_COND_SIZE;
constexpr int ERR_INVALID_COND_TIMEOUT = JsErrorCode::ERR_INVALID_COND_TIMEOUT;

// business error of configure function
constexpr int ERR_INVALID_MAX_STORAGE = JsErrorCode::ERR_INVALID_MAX_STORAGE;

// business error of AppEventPackageHolder.setSize function
constexpr int ERR_INVALID_SIZE = JsErrorCode::ERR_INVALID_SIZE;

// business error for incorrect parameter length or format
constexpr int ERR_INVALID_PARAM_VALUE = JsErrorCode::ERR_INVALID_PARAM_VALUE;

// business error of registerExternalLogManager function
constexpr int ERR_LOG_MANAGER_ALREADY_REGISTERED = JsErrorCode::ERR_LOG_MANAGER_ALREADY_REGISTERED;

std::string GetErrorMsg(int errCode);
} // namespace NapiError
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ERROR_H