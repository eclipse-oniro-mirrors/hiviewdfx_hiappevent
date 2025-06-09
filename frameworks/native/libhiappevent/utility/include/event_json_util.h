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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_EVENT_JSON_UTIL_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_EVENT_JSON_UTIL_H

#include <string>
#include <unordered_set>

#include "cJSON.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventJsonUtil {
bool CJsonIsInt(const cJSON* item);
bool CJsonIsUint(const cJSON *item);
bool CJsonIsInt64(const cJSON *item);
bool CJsonIsBool(const cJSON *item);
uint32_t ParseUInt32(const cJSON *root, const std::string& key);
int ParseInt(const cJSON *root, const std::string& key);
std::string ParseString(const cJSON *root, const std::string& key);
void ParseStrings(const cJSON *root, const std::string& key, std::unordered_set<std::string>& strs);
cJSON *GetJsonObjectFromJsonString(const std::string& paramString);
std::vector<std::string> CJsonGetMemberNames(const cJSON *json);
} // namespace EventJsonUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_EVENT_JSON_UTIL_H
