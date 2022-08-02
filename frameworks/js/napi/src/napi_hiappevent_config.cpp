/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <string>

#include "hiappevent_config.h"
#include "napi_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventConfig {
bool Configure(const napi_env env, const napi_value configObj)
{
    if (NapiUtil::GetType(env, configObj) != napi_object) {
        return false;
    }

    std::vector<std::string> keys;
    NapiUtil::GetPropertyNames(env, configObj, keys);
    bool result = true;
    for (auto key : keys) {
        napi_value value = NapiUtil::GetProperty(env, configObj, key);
        if (value == nullptr) {
            result = false;
            continue;
        }
        std::string strValue = NapiUtil::ConvertToString(env, value);
        if (strValue.empty()) {
            result = false;
            continue;
        }
        if (!HiAppEventConfig::GetInstance().SetConfigurationItem(key, strValue)) {
            result = false;
        }
    }
    return result;
}
} // namespace NapiHiAppEventConfig
} // namespace HiviewDFX
} // namespace OHOS
