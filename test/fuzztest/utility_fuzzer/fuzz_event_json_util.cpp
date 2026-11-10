/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <string>
#include <unordered_set>
#include "hiappevent_base.h"
#include "json/json.h"
#include "event_json_util.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t FUZZ_VALUE_MAX_LEN = 256;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

void FuzzEventJsonUtil(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string s = FS(data, size, FUZZ_VALUE_MAX_LEN);
    Json::Value v;
    Json::Reader r;
    r.parse(s, v);
    if (!v.isObject() && !v.isNull()) {
        return;
    }
    Json::Value validObj;
    validObj["u64"] = static_cast<uint64_t>(data[0]);
    validObj["u32"] = static_cast<uint32_t>(data[0]);
    validObj["i"] = static_cast<int>(data[0]);
    validObj["s"] = "str";
    Json::Value arr(Json::arrayValue);
    arr.append("a");
    arr.append("b");
    validObj["arr"] = arr;
    OHOS::HiviewDFX::EventJsonUtil::ParseUInt64(validObj, "u64");
    OHOS::HiviewDFX::EventJsonUtil::ParseUInt32(validObj, "u32");
    OHOS::HiviewDFX::EventJsonUtil::ParseInt(validObj, "i");
    OHOS::HiviewDFX::EventJsonUtil::ParseString(validObj, "s");
    std::unordered_set<std::string> validNs;
    OHOS::HiviewDFX::EventJsonUtil::ParseStrings(validObj, "arr", validNs);
    OHOS::HiviewDFX::EventJsonUtil::ParseUInt64(v, "key");
    OHOS::HiviewDFX::EventJsonUtil::ParseUInt32(v, "key");
    OHOS::HiviewDFX::EventJsonUtil::ParseInt(v, "key");
    OHOS::HiviewDFX::EventJsonUtil::ParseString(v, "key");
    std::unordered_set<std::string> ns;
    OHOS::HiviewDFX::EventJsonUtil::ParseStrings(v, "key", ns);
    Json::Value ej;
    OHOS::HiviewDFX::EventJsonUtil::GetJsonObjectFromJsonString(ej, s);
}
