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
#include <vector>
#include <memory>
#include "hiappevent_base.h"
#include "ndk_external_log_callback.h"
#include "fuzz_cases.h"

namespace {
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

void FuzzNdkExternalLogCallback(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    auto cb = std::make_shared<OHOS::HiviewDFX::NdkExternalLogCallback>(nullptr);
    std::vector<OHOS::HiviewDFX::ExternalLogWrapperInfo> infos;
    cb->OnCapacityReached(infos);
}
