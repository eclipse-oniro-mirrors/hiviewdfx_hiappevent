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
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include "app_crash_policy.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t FUZZ_VALUE_MAX_LEN = 256;
constexpr int FIRST_MAP_KEY = 0;
constexpr int SECOND_MAP_KEY = 1;
constexpr size_t HALF_DIVISOR = 2;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

static std::string FS2(const uint8_t* d, size_t s, size_t m = FUZZ_NAME_MAX_LEN)
{
    if (s < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t h = s / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(d + h), std::min(s - h, m));
}

void FuzzAppCrashPolicy(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string k = FS(data, size, FUZZ_KEY_MAX_LEN);
    std::string v = FS2(data, size, FUZZ_VALUE_MAX_LEN);
    std::map<std::string, std::string> cm = {{"pageSwitchLogEnable", "true"},
        {"publishOnNextLaunch", "true"}, {"extendPcLrPrinting", "true"},
        {"logFileCutoffSzBytes", "1024"}, {"simplifyVmaPrinting", "false"},
        {"collectMinidump", "true"}, {"useRefinedLogFileName", "true"}, {k, v}};
    OHOS::HiviewDFX::AppCrashPolicy p;
    p.SetEventPolicy(cm);
    std::map<uint8_t, uint32_t> im = {{FIRST_MAP_KEY, data[0]}, {SECOND_MAP_KEY, data[1]}};
    p.SetEventPolicy(im);
}
