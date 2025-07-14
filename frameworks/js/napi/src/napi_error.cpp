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
#include "napi_error.h"
 
#include <map>
namespace OHOS {
namespace HiviewDFX {
namespace NapiError {
std::string GetErrorMsg(int errCode)
{
    const std::map<int, const char *> errMap = {
        // common api error
        { ERR_PARAM, "Parameter error. Possible causes: 1. Mandatory parameters are left unspecified; 2. Incorrect "\
            "parameter types; 3.Parameter verification failed." },
        // business error of write function
        { ERR_DISABLE, "Function disabled. Possible caused by the param disable in ConfigOption is true." },
        { ERR_INVALID_DOMAIN,
            "Invalid event domain. Possible causes: 1. Contain invalid characters; 2. Length is invalid." },
        { ERR_INVALID_NAME,
            "Invalid event name. Possible causes: 1. Contain invalid characters; 2. Length is invalid." },
        { ERR_INVALID_PARAM_NUM,
            "Invalid number of event parameters. Possible caused by the number of parameters is over 32." },
        { ERR_INVALID_STR_LEN, "Invalid string length of the event parameter." },
        { ERR_INVALID_KEY,
            "Invalid event parameter name. Possible causes: 1. Contain invalid characters; 2. Length is invalid." },
        { ERR_INVALID_ARR_LEN, "Invalid array length of the event parameter." },
        { ERR_INVALID_CUSTOM_PARAM_NUM, "The number of parameter keys exceeds the limit." },
        // business error of addWatcher function
        { ERR_INVALID_WATCHER_NAME,
            "Invalid watcher name. Possible causes: 1. Contain invalid characters; 2. Length is invalid." },
        { ERR_INVALID_FILTER_DOMAIN,
            "Invalid filtering event domain. Possible causes: 1. Contain invalid characters; 2. Length is invalid." },
        { ERR_INVALID_COND_ROW, "Invalid row value. Possible caused by the row value is less than zero." },
        { ERR_INVALID_COND_SIZE, "Invalid size value. Possible caused by the size value is less than zero." },
        { ERR_INVALID_COND_TIMEOUT, "Invalid timeout value. Possible caused by the timeout value is less than zero." },
        // business error of configure function
        { ERR_INVALID_MAX_STORAGE, "Invalid max storage quota value. Possible caused by incorrectly formatted." },
        // business error of AppEventPackageHolder.setSize function
        { ERR_INVALID_SIZE, "Invalid size value. Possible caused by the size value is less than or equal to zero."},
        // business error for incorrect parameter length or format
        { ERR_INVALID_PARAM_VALUE,
            "Invalid parameter value. Possible causes: 1. Incorrect parameter length; 2. Incorrect parameter format." }
    };
    auto it = errMap.find(errCode);
    return it == errMap.end() ? "ErrorMsg undefined" : it->second;
}
} // namespace NapiError
} // namespace HiviewDFX
} // namespace OHOS