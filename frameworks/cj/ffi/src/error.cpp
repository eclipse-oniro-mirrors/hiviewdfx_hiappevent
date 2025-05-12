/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <unordered_map>

#include "error.h"

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
using namespace OHOS::HiviewDFX::ErrorCode;

const std::unordered_map<int, int> ERR_MAP = {
    { ERROR_INVALID_EVENT_NAME, ERR_INVALID_NAME },
    { ERROR_INVALID_EVENT_DOMAIN, ERR_INVALID_DOMAIN },
    { ERROR_HIAPPEVENT_DISABLE, ERR_DISABLE },
    { ERROR_INVALID_PARAM_NAME, ERR_INVALID_KEY },
    { ERROR_INVALID_PARAM_VALUE_LENGTH, ERR_INVALID_STR_LEN },
    { ERROR_INVALID_PARAM_NUM, ERR_INVALID_PARAM_NUM },
    { ERROR_INVALID_LIST_PARAM_SIZE, ERR_INVALID_ARR_LEN },
    { ERROR_INVALID_CUSTOM_PARAM_NUM, ERR_INVALID_CUSTOM_PARAM_NUM },
    { ERROR_INVALID_PARAM_VALUE_LENGTH, ERR_INVALID_PARAM_VALUE_LENGTH}
};

int GetErrorCode(int errCode)
{
    if (ERR_MAP.find(errCode) != ERR_MAP.end()) {
        return ERR_MAP.at(errCode);
    } else {
        return ERR_PARAM;
    }
}
} // HiAppEvent
} // CJSystemapi
} // OHOS