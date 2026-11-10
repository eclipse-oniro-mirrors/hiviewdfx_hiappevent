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
#include "ndk_app_event_processor_service.h"
#include "fuzz_cases.h"
namespace {
    constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
    constexpr int STATIC_ARRAY_LEN = 2;
    constexpr int BOOL_VALUE_COUNT = 2;
    constexpr size_t FUZZ_NAME_MAX_LEN = 48;
    constexpr size_t DEFAULT_STR_MAX_LEN = 64;
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

void FuzzNdkAppEventProcessorService(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string n = FuzzStr(data, size, DEFAULT_STR_MAX_LEN);
    HiAppEvent_Processor* p = CreateProcessor(n.c_str());
    if (p) {
        SetReportRoute(p, "app", "route");
        SetReportPolicy(p, data[0], data[1], data[0] % BOOL_VALUE_COUNT, data[1] % BOOL_VALUE_COUNT);
        SetReportEvent(p, "OS", "APP_CRASH", data[0] % BOOL_VALUE_COUNT);
        SetCustomConfig(p, "k", "v");
        SetConfigId(p, data[0]);
        SetConfigName(p, n.c_str());
        const char* ids[] = {"id1", "id2"};
        SetReportUserId(p, ids, STATIC_ARRAY_LEN);
        const char* props[] = {"p1", "p2"};
        SetReportUserProperty(p, props, STATIC_ARRAY_LEN);
        AddProcessor(p);
        DestroyProcessor(p);
    }
    RemoveProcessor(data[0]);
}
