/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "event_json_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventJsonUtil {
uint32_t ParseUInt32(const Json::Value& root, const std::string& key)
{
    return (root.isMember(key) && root[key].isUInt()) ? root[key].asUInt() : 0;
}

int ParseInt(const Json::Value& root, const std::string& key)
{
    return (root.isMember(key) && root[key].isInt()) ? root[key].asInt() : 0;
}

std::string ParseString(const Json::Value& root, const std::string& key)
{
    return (root.isMember(key) && root[key].isString()) ? root[key].asString() : "";
}

void ParseStrings(const Json::Value& root, const std::string& key, std::unordered_set<std::string>& strs)
{
    if (!root.isMember(key) || !root[key].isArray()) {
        return;
    }
    for (Json::ArrayIndex i = 0; i < root[key].size(); ++i) {
        if (root[key][i].isString()) {
            strs.insert(root[key][i].asString());
        }
    }
}
} // namespace EventJsonUtil
} // namespace HiviewDFX
} // namespace OHOS
