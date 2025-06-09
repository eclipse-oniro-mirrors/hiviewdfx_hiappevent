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
bool CJsonIsInt(const cJSON* item)
{
    if (!cJSON_IsNumber(item)) {
        return false;
    }
    double value = item->valuedouble;
    return value == static_cast<int32_t>(value);
}

bool CJsonIsUint(const cJSON *item)
{
    if (!cJSON_IsNumber(item)) {
        return false;
    }
    double value = item->valuedouble;
    return value == static_cast<uint32_t>(value);
}

bool CJsonIsInt64(const cJSON *item)
{
    if (!cJSON_IsNumber(item)) {
        return false;
    }
    double value = item->valuedouble;
    return value == static_cast<int64_t>(value);
}

bool CJsonIsBool(const cJSON* item)
{
    return item && cJSON_IsBool(item);
}

uint32_t ParseUInt32(const cJSON *root, const std::string& key)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(root, key.c_str());
    if (item && CJsonIsUint(item)) {
        return static_cast<uint32_t>(item->valuedouble);
    }
    return 0;
}

int ParseInt(const cJSON *root, const std::string& key)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(root, key.c_str());
    if (item && CJsonIsInt(item)) {
        return static_cast<int32_t>(item->valuedouble);
    }
    return 0;
}

std::string ParseString(const cJSON *root, const std::string& key)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(root, key.c_str());
    if (item && cJSON_IsString(item)) {
        return item->valuestring;
    }
    return "";
}

void ParseStrings(const cJSON *root, const std::string& key, std::unordered_set<std::string>& strs)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(root, key.c_str());
    if (!item || !cJSON_IsArray(item)) {
        return;
    }
    size_t itemSize = cJSON_GetArraySize(item);
    for (size_t i = 0; i < itemSize; ++i) {
        cJSON *elem = cJSON_GetArrayItem(item, i);
        if (elem && cJSON_IsString(elem)) {
            strs.insert(elem->valuestring);
        }
    }
}

cJSON *GetJsonObjectFromJsonString(const std::string& paramString)
{
    cJSON *eventJson = cJSON_Parse(paramString.c_str());
    if (!eventJson) {
        return nullptr;
    }
    if (!cJSON_IsObject(eventJson)) {
        cJSON_Delete(eventJson);
        return nullptr;
    }
    return eventJson;
}

std::vector<std::string> CJsonGetMemberNames(const cJSON *json)
{
    if (cJSON_IsObject(json)) {
        cJSON *current = json->child;
        std::vector<std::string> memberNames;
        while (current) {
            memberNames.push_back(current->string);
            current = current->next;
        }
        return memberNames;
    }
    return {};
}
} // namespace EventJsonUtil
} // namespace HiviewDFX
} // namespace OHOS
