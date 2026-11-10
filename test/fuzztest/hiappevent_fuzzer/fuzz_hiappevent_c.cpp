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
#include <string>

#include "hiappevent_c.h"
#include "fuzz_cases.h"

namespace {
constexpr int32_t TEST_INT_PARAM_VALUE = 42;
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t HALF_DIVISOR = 2;
constexpr int ARR_SIZE = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr int FRAMEWORK_TYPE_COUNT = 3;
constexpr size_t FUZZ_DOMAIN_MAX_LEN = 32;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
}

static std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

static std::string FuzzStr2(const uint8_t* data, size_t size, size_t maxLen)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}

static void FuzzCAddParamValues(ParamList list, const uint8_t* data)
{
    AddBoolParamValue(list, "b", static_cast<bool>(data[0]));
    AddInt8ParamValue(list, "i8", static_cast<int8_t>(data[0]));
    AddInt16ParamValue(list, "i16", static_cast<int16_t>(data[0]));
    AddInt32ParamValue(list, "i32", static_cast<int32_t>(data[0]));
    AddInt64ParamValue(list, "i64", static_cast<int64_t>(data[0]));
    AddFloatParamValue(list, "f", 1.0f);
    AddDoubleParamValue(list, "d", 1.0);
    AddStringParamValue(list, "s", "v");
    const bool boolArr[ARR_SIZE] = { true, false };
    AddBoolArrayParamValue(list, "ba", boolArr, ARR_SIZE);
    const int8_t int8Arr[ARR_SIZE] = { 1, 2 };
    AddInt8ArrayParamValue(list, "i8a", int8Arr, ARR_SIZE);
    const int16_t int16Arr[ARR_SIZE] = { 1, 2 };
    AddInt16ArrayParamValue(list, "i16a", int16Arr, ARR_SIZE);
    const int32_t int32Arr[ARR_SIZE] = { 1, 2 };
    AddInt32ArrayParamValue(list, "i32a", int32Arr, ARR_SIZE);
    const int64_t int64Arr[ARR_SIZE] = { 1, 2 };
    AddInt64ArrayParamValue(list, "i64a", int64Arr, ARR_SIZE);
    const float floatArr[ARR_SIZE] = { 1.0f, 2.0f };
    AddFloatArrayParamValue(list, "fa", floatArr, ARR_SIZE);
    const double doubleArr[ARR_SIZE] = { 1.0, 2.0 };
    AddDoubleArrayParamValue(list, "da", doubleArr, ARR_SIZE);
    const char* strArr[ARR_SIZE] = { "a", "b" };
    AddStringArrayParamValue(list, "sa", strArr, ARR_SIZE);
}

void FuzzHiappeventC(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string domainStr = FuzzStr(data, size, FUZZ_DOMAIN_MAX_LEN);
    std::string nameStr = FuzzStr2(data, size, FUZZ_NAME_MAX_LEN);
    int type = data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST;
    ParamList list = HiAppEventCreateParamList();
    FuzzCAddParamValues(list, data);
    std::string validDomain = domainStr.empty() ? "testDm" : domainStr;
    std::string validName = nameStr.empty() ? "testNm" : nameStr;
    HiAppEventInnerWrite(validDomain.c_str(), validName.c_str(), static_cast<EventType>(type), list);
    HiAppEventInnerWrite(validDomain.c_str(), validName.c_str(), static_cast<EventType>(type), nullptr);
    ParamList nullList = HiAppEventCreateParamList();
    AddBoolParamValue(nullList, nullptr, true);
    AddInt32ParamValue(nullList, nullptr, TEST_INT_PARAM_VALUE);
    AddStringParamValue(nullList, nullptr, "v");
    AddBoolParamValue(nullptr, "k", true);
    HiAppEventDestroyParamList(nullList);
    HiAppEventDestroyParamList(list);
    HiAppEventInnerConfigure("disable", "true");
    HiAppEventInnerConfigure(nullptr, "true");
    HiAppEvent_Config* config = HiAppEventCreateConfig();
    HiAppEventSetConfigItem(config, "k", "v");
    HiAppEventSetEventConfig("t", config);
    auto frameworkType = static_cast<OH_HiAppEvent_FrameworkType>(data[0] % FRAMEWORK_TYPE_COUNT);
    HiAppEventReportFrameworkMemAnomaly(frameworkType, "1.0", "desc");
    HiAppEventDestroyConfig(config);
    ClearData();
}
