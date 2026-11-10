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
#include "processor_config_loader.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr size_t HELPER_MIN_STR_SIZE = 2;
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
    if (size < HELPER_MIN_STR_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}

void FuzzProcessorConfigLoader(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string p = FuzzStr(data, size, DEFAULT_STR_MAX_LEN);
    std::string c = FuzzStr2(data, size, FUZZ_KEY_MAX_LEN);
    {
        FILE* cf = fopen("/data/test/fuzz/hiappevent/processor.json", "w");
        if (cf != nullptr) {
            fputs("{\"fuzz_proc\":{\"route_info\":{\"app_id\":\"a1\",\"uri\":\"u1\"}}}", cf);
            fclose(cf);
        }
    }
    OHOS::HiviewDFX::HiAppEvent::ProcessorConfigLoader loader;
    loader.LoadProcessorConfig("/data/test/fuzz/hiappevent/processor.json", c);
    loader.LoadProcessorConfig(p, c);
    loader.GetReportConfig();
}
