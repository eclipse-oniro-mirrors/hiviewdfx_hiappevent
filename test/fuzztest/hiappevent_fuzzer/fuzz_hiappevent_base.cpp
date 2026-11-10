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

#include <algorithm>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "hiappevent_base.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 4;
constexpr size_t HELPER_MIN_STR_SIZE = 2;
constexpr int BOOL_VALUE_COUNT = 2;
constexpr size_t HALF_DIVISOR = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr size_t FUZZ_DOMAIN_MAX_LEN = 32;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
}

static std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

static std::string FuzzStr2(const uint8_t* data, size_t size, size_t maxLen)
{
    if (size < HELPER_MIN_STR_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}

static void FuzzBaseAddParams(OHOS::HiviewDFX::AppEventPack& pack, const uint8_t* data, size_t size)
{
    pack.AddParam("k");
    pack.AddParam("kb", data[0] % BOOL_VALUE_COUNT == 0);
    pack.AddParam("kc", static_cast<char>(data[0]));
    pack.AddParam("ki8", static_cast<int8_t>(data[0]));
    pack.AddParam("ki16", static_cast<int16_t>(data[0]));
    pack.AddParam("ki", static_cast<int>(data[0]));
    pack.AddParam("ki64", static_cast<int64_t>(data[0]));
    pack.AddParam("kf", static_cast<float>(data[0]));
    pack.AddParam("kd", static_cast<double>(data[0]));
    pack.AddParam("kcp", "cv");
    std::string fuzzStr = FuzzStr(data, size, FUZZ_DOMAIN_MAX_LEN);
    std::string escStr = "a\tb\nc\rd\"e\\f\bg\fh";
    pack.AddParam("esc", escStr);
    pack.AddParam("ks", fuzzStr);
    pack.AddParam("ks", fuzzStr);
    std::vector<bool> boolVec = { true, false };
    std::vector<int8_t> int8Vec = { 1, 2 };
    std::vector<char> charVec = { 'a', 'b' };
    std::vector<int16_t> int16Vec = { 1, 2 };
    std::vector<int> intVec = { 1, 2, 3 };
    std::vector<int64_t> int64Vec = { 1, 2 };
    std::vector<float> floatVec = { 1.0f, 2.0f };
    std::vector<double> doubleVec = { 1.0, 2.0 };
    std::vector<const char*> cStrVec = { "a", "b" };
    std::vector<std::string> strVec = { "a", "b" };
    pack.AddParam("kb_arr", boolVec);
    pack.AddParam("ki8_arr", int8Vec);
    pack.AddParam("kc_arr", charVec);
    pack.AddParam("ki16_arr", int16Vec);
    pack.AddParam("ki_arr", intVec);
    pack.AddParam("ki64_arr", int64Vec);
    pack.AddParam("kf_arr", floatVec);
    pack.AddParam("kd_arr", doubleVec);
    pack.AddParam("kcp_arr", cStrVec);
    pack.AddParam("ks_arr", strVec);
    std::unordered_map<std::string, std::string> customParams = { { "ck", "cv" } };
    pack.AddCustomParams(customParams);
}

static void FuzzBaseGetSetInfo(OHOS::HiviewDFX::AppEventPack& pack, const uint8_t* data,
    const std::string& domain, const std::string& name)
{
    pack.GetEventStr();
    pack.GetParamStr();
    pack.GetSeq();
    pack.GetDomain();
    pack.GetName();
    pack.GetType();
    pack.GetTime();
    pack.GetTimeZone();
    pack.GetPid();
    pack.GetTid();
    pack.GetTraceId();
    pack.GetSpanId();
    pack.GetPspanId();
    pack.GetTraceFlag();
    pack.GetRunningId();
    std::list<OHOS::HiviewDFX::AppEventParam> baseParams = pack.GetBaseParams();
    std::vector<OHOS::HiviewDFX::CustomEventParam> customParams;
    pack.GetCustomParams(customParams);
    pack.GetExternalLogManager();
    pack.SetSeq(data[0]);
    pack.SetDomain(name);
    pack.SetName(domain);
    pack.SetType(data[0]);
    pack.SetTime(data[0]);
    pack.SetTimeZone("UTC+8");
    pack.SetPid(data[0]);
    pack.SetTid(data[0]);
    pack.SetTraceId(data[0]);
    pack.SetSpanId(data[0]);
    pack.SetPspanId(data[0]);
    pack.SetTraceFlag(data[0]);
    pack.SetRunningId("rid");
    pack.SetBaseParams(baseParams);
    pack.SetParamStr("{}");
    OHOS::HiviewDFX::ExternalLogManager externalLogManager;
    pack.SetExternalLogManager(externalLogManager);
}

void FuzzHiappeventBase(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string domain = FuzzStr(data, size, FUZZ_DOMAIN_MAX_LEN);
    std::string name = FuzzStr2(data, size, FUZZ_NAME_MAX_LEN);
    OHOS::HiviewDFX::AppEventPack pack(domain, name, data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST);
    FuzzBaseAddParams(pack, data, size);
    FuzzBaseGetSetInfo(pack, data, domain, name);
}
