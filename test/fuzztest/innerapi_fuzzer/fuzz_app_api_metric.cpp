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

#include "app_api_metric.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t HALF_DIVISOR = 2;
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t KIT_API_STR_MAX_LEN = 32;
constexpr int EVEN_NUMBER_BASE = 2;

std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

std::string FuzzStr2(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}
}

void FuzzAppApiMetric(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string kit = FuzzStr(data, size, KIT_API_STR_MAX_LEN);
    std::string api = FuzzStr2(data, size, KIT_API_STR_MAX_LEN);
    OHOS::HiviewDFX::HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = kit;
    apiInfo.api = api;
    OHOS::HiviewDFX::HiAppEvent::ApiMetric metric;
    metric.errCode = data[0];
    metric.duration = data[1];
    metric.successful = data[0] % EVEN_NUMBER_BASE == 0;
    OHOS::HiviewDFX::HiAppEvent::ReportApiMetric(apiInfo, metric);
}
