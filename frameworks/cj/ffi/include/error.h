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

#ifndef ERROR_H
#define ERROR_H

#include "hiappevent_base.h"

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
// CJ error
constexpr int ERR_PARAM = 401;
constexpr int ERR_DISABLE = 11100001;
constexpr int ERR_INVALID_DOMAIN = 11101001;
constexpr int ERR_INVALID_NAME = 11101002;
constexpr int ERR_INVALID_PARAM_NUM = 11101003;
constexpr int ERR_INVALID_STR_LEN = 11101004;
constexpr int ERR_INVALID_KEY = 11101005;
constexpr int ERR_INVALID_ARR_LEN = 11101006;
constexpr int ERR_INVALID_WATCHER_NAME = 11102001;
constexpr int ERR_INVALID_FILTER_DOMAIN = 11102002;
constexpr int ERR_INVALID_COND_ROW = 11102003;
constexpr int ERR_INVALID_COND_SIZE = 11102004;
constexpr int ERR_INVALID_COND_TIMEOUT = 11102005;
constexpr int ERR_INVALID_MAX_STORAGE = 11103001;
constexpr int ERR_INVALID_SIZE = 11104001;
constexpr int ERR_INVALID_CUSTOM_PARAM_NUM = 11101007;
constexpr int ERR_INVALID_PARAM_VALUE_LENGTH = 11101004;

int GetErrorCode(int errCode);
} // HiAppEvent
} // CJSystemapi
} // OHOS

#endif