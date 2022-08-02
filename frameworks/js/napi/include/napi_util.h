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

#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_UTIL_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_UTIL_H

#include <cstdint>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiUtil {
bool IsNull(const napi_env env, const napi_value value);
bool IsBoolean(const napi_env env, const napi_value value);
bool IsNumber(const napi_env env, const napi_value value);
bool IsString(const napi_env env, const napi_value value);
bool IsObject(const napi_env env, const napi_value value);
bool IsFunction(const napi_env env, const napi_value value);
bool IsArray(const napi_env env, const napi_value value);
bool HasProperty(const napi_env env, const napi_value object, const std::string& name);

bool GetBoolean(const napi_env env, const napi_value value);
void GetBooleans(const napi_env env, const napi_value arr, std::vector<bool>& bools);
int32_t GetInt32(const napi_env env, const napi_value value);
void GetInt32s(const napi_env env, const napi_value arr, std::vector<int32_t>& ints);
double GetDouble(const napi_env env, const napi_value value);
void GetDoubles(const napi_env env, const napi_value arr, std::vector<double>& doubles);
std::string GetString(const napi_env env, const napi_value value, size_t bufsize = 100); // 100 means default size
void GetStrings(const napi_env env, const napi_value arr, std::vector<std::string>& strs, size_t bufsize);
napi_valuetype GetType(const napi_env env, const napi_value value);
napi_valuetype GetArrayType(const napi_env env, const napi_value value);
uint32_t GetArrayLength(const napi_env env, const napi_value arr);
napi_value GetElement(const napi_env env, const napi_value arr, uint32_t index);
napi_value GetProperty(const napi_env env, const napi_value object, const std::string& name);
void GetPropertyNames(const napi_env env, const napi_value object, std::vector<std::string>& names);

napi_ref CreateReference(const napi_env env, const napi_value func);
napi_value CreateUndefined(const napi_env env);
napi_value CreateBoolean(const napi_env env, bool bValue);
napi_value CreateInt32(const napi_env env, int32_t num);
napi_value CreateString(const napi_env env, const std::string& str);
napi_value CreateObject(const napi_env env);
napi_value CreateObject(const napi_env env, const std::string& key, const napi_value value);

std::string ConvertToString(const napi_env env, const napi_value value);
} // namespace NapiUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_UTIL_H
