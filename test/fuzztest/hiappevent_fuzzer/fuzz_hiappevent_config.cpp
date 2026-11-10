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

#include "hiappevent_config.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t HALF_DIVISOR = 2;
constexpr size_t FUZZ_NAME_MAX_LEN = 32;
constexpr size_t FUZZ_VALUE_MAX_LEN = 256;
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

void FuzzHiappeventConfig(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FuzzStr(data, size, FUZZ_NAME_MAX_LEN);
    std::string value = FuzzStr2(data, size, FUZZ_VALUE_MAX_LEN);
    auto& config = OHOS::HiviewDFX::HiAppEventConfig::GetInstance();
    config.SetStorageDir("/data/test/fuzz/hiappevent/");
    config.SetConfigurationItem(name, value);
    config.SetConfigurationItem("disable", "true");
    config.SetConfigurationItem("disable", "false");
    config.SetConfigurationItem("max_storage", "10m");
    config.SetConfigurationItem("max_storage", "1g");
    config.SetConfigurationItem("", value);
    config.SetConfigurationItem(name, "");
    config.GetDisable();
    config.GetMaxStorageSize();
    config.GetStorageDir();
    config.GetRunningId();
    config.IsFreeSizeOverLimit();
    config.RefreshFreeSize();
}
