/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace HiviewDFX {
namespace NapiError {
// common api error
constexpr int ERR_PARAM = 401;

// business error of write function
constexpr int ERR_DISABLE = 11100001;
constexpr int ERR_INVALID_DOMAIN = 11101001;
constexpr int ERR_INVALID_NAME = 11101002;
constexpr int ERR_INVALID_PARAM_NUM = 11101003;
constexpr int ERR_INVALID_STR_LEN = 11101004;
constexpr int ERR_INVALID_KEY = 11101005;
constexpr int ERR_INVALID_ARR_LEN = 11101006;

// business error of addWatcher function
constexpr int ERR_INVALID_WATCHER_NAME = 11102001;
constexpr int ERR_INVALID_FILTER_DOMAIN = 11102002;
constexpr int ERR_INVALID_COND_ROW = 11102003;
constexpr int ERR_INVALID_COND_SIZE = 11102004;
constexpr int ERR_INVALID_COND_TIMEOUT = 11102005;

// business error of configure function
constexpr int ERR_INVALID_MAX_STORAGE = 11103001;

// business error of AppEventPackageHolder.setSize function
constexpr int ERR_INVALID_SIZE = 11104001;

// business error of addProcessor function
constexpr int ERR_INVALID_PROCESSOR_NAME = 11105001;
constexpr int ERR_INVALID_PROCESSOR_DEBUG_MODE = 11105002;
constexpr int ERR_INVALID_PROCESSOR_ROUTE_INFO = 11105003;
constexpr int ERR_INVALID_PROCESSOR_START_REPORT = 11105004;
constexpr int ERR_INVALID_PROCESSOR_BACKGROUND_REPORT = 11105005;
constexpr int ERR_INVALID_PROCESSOR_PERIOD_REPORT = 11105006;
constexpr int ERR_INVALID_PROCESSOR_BATCH_REPORT = 11105007;
constexpr int ERR_INVALID_PROCESSOR_USER_IDS = 11105008;
constexpr int ERR_INVALID_PROCESSOR_USER_PROPERTIES = 11105009;
constexpr int ERR_INVALID_PROCESSOR_EVENT_CONFIGS = 111050010;

} // namespace NapiError
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ERROR_H