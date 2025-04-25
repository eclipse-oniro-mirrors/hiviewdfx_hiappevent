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

#include "json/json.h"

namespace OHOS {
namespace HiviewDFX {
namespace EventJsonUtil {
uint32_t ParseUInt32(const Json::Value& root, const std::string& key);
int ParseInt(const Json::Value& root, const std::string& key);
std::string ParseString(const Json::Value& root, const std::string& key);
void ParseStrings(const Json::Value& root, const std::string& key, std::unordered_set<std::string>& strs);
bool GetJsonObjectFromJsonString(Json::Value& eventJson, const std::string& paramString);
} // namespace EventJsonUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_EVENT_JSON_UTIL_H
