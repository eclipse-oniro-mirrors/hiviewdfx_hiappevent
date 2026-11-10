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
#include "hiappevent_base.h"
#include "hiappevent/hiappevent.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr int FRAMEWORK_TYPE_COUNT = 3;
constexpr int EVENT_SIZE_FACTOR = 100;
constexpr int BOOL_VALUE_COUNT = 2;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr int STATIC_ARRAY_LEN = 2;
constexpr int EVENT_NAME_COUNT = 1;
constexpr size_t HALF_DIVISOR = 2;
}

static std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

static std::string FuzzStr2(const uint8_t* data, size_t size, size_t maxLen = FUZZ_NAME_MAX_LEN)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}

static void FuzzNdkWriteAndConfig(const uint8_t* data, size_t size)
{
    std::string domainStr = FuzzStr(data, size, DEFAULT_STR_MAX_LEN);
    std::string nameStr = FuzzStr2(data, size, FUZZ_NAME_MAX_LEN);
    int type = data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST;
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddBoolParam(list, "b", data[0] % BOOL_VALUE_COUNT == 0);
    OH_HiAppEvent_AddBoolArrayParam(list, "ba", (const bool[]) {true, false}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddInt8Param(list, "i8", static_cast<int8_t>(data[0]));
    OH_HiAppEvent_AddInt8ArrayParam(list, "i8a", (const int8_t[]) {1, 2}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddInt16Param(list, "i16", static_cast<int16_t>(data[0]));
    OH_HiAppEvent_AddInt16ArrayParam(list, "i16a", (const int16_t[]) {1, 2}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddInt32Param(list, "i32", static_cast<int32_t>(data[0]));
    OH_HiAppEvent_AddInt32ArrayParam(list, "i32a", (const int32_t[]) {1, 2}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddInt64Param(list, "i64", static_cast<int64_t>(data[0]));
    OH_HiAppEvent_AddInt64ArrayParam(list, "i64a", (const int64_t[]) {1, 2}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddFloatParam(list, "f", 1.0f);
    OH_HiAppEvent_AddFloatArrayParam(list, "fa", (const float[]) {1.0f, 2.0f}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddDoubleParam(list, "d", 1.0);
    OH_HiAppEvent_AddDoubleArrayParam(list, "da", (const double[]) {1.0, 2.0}, STATIC_ARRAY_LEN);
    OH_HiAppEvent_AddStringParam(list, "s", "v");
    const char* sa[] = {"a", "b"};
    OH_HiAppEvent_AddStringArrayParam(list, "sa", sa, STATIC_ARRAY_LEN);
    OH_HiAppEvent_Write(domainStr.c_str(), nameStr.c_str(), static_cast<EventType>(type), list);
    OH_HiAppEvent_DestroyParamList(list);
    OH_HiAppEvent_Write(nullptr, nameStr.c_str(), static_cast<EventType>(type), nullptr);
    OH_HiAppEvent_Configure("disable", "true");
    OH_HiAppEvent_Configure(nullptr, "true");
    OH_HiAppEvent_ClearData();
    HiAppEvent_Config* cfg = OH_HiAppEvent_CreateConfig();
    OH_HiAppEvent_SetConfigItem(cfg, "k", "v");
    OH_HiAppEvent_SetEventConfig("t", cfg);
    OH_HiAppEvent_DestroyConfig(cfg);
    OH_HiAppEvent_ReportFrameworkMemAnomaly(
        static_cast<OH_HiAppEvent_FrameworkType>(data[0] % FRAMEWORK_TYPE_COUNT), "1.0", "desc");
}

static void FuzzNdkWatcherProcessor(const uint8_t* data)
{
    HiAppEvent_Watcher* w = OH_HiAppEvent_CreateWatcher("fw");
    if (w) {
        const char* names[] = {"APP_CRASH"};
        OH_HiAppEvent_SetAppEventFilter(w, "OS", data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST,
            names, EVENT_NAME_COUNT);
        OH_HiAppEvent_SetTriggerCondition(w, data[0], data[0] * EVENT_SIZE_FACTOR, data[1]);
        OH_HiAppEvent_SetWatcherOnTrigger(w, nullptr);
        OH_HiAppEvent_SetWatcherOnReceive(w, nullptr);
        OH_HiAppEvent_AddWatcher(w);
        OH_HiAppEvent_TakeWatcherData(w, EVENT_SIZE_FACTOR, nullptr);
        OH_HiAppEvent_RemoveWatcher(w);
        OH_HiAppEvent_DestroyWatcher(w);
    }
    HiAppEvent_Processor* p = OH_HiAppEvent_CreateProcessor("fp");
    if (p) {
        OH_HiAppEvent_SetReportRoute(p, "app", "route");
        OH_HiAppEvent_SetReportPolicy(p, data[0], data[1], data[0] % BOOL_VALUE_COUNT, data[1] % BOOL_VALUE_COUNT);
        OH_HiAppEvent_SetReportEvent(p, "OS", "APP_CRASH", data[0] % BOOL_VALUE_COUNT);
        OH_HiAppEvent_SetCustomConfig(p, "k", "v");
        OH_HiAppEvent_SetConfigId(p, data[0]);
        OH_HiAppEvent_SetConfigName(p, "cn");
        const char* ids[] = {"id1"};
        OH_HiAppEvent_SetReportUserId(p, ids, 1);
        const char* props[] = {"p1"};
        OH_HiAppEvent_SetReportUserProperty(p, props, 1);
        OH_HiAppEvent_AddProcessor(p);
        OH_HiAppEvent_DestroyProcessor(p);
    }
    OH_HiAppEvent_RemoveProcessor(data[0]);
    OH_HiAppEvent_RegExternalLogCapacityReachedCallback(nullptr);
}

void FuzzHiappeventNdk(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    FuzzNdkWriteAndConfig(data, size);
    FuzzNdkWatcherProcessor(data);
}
