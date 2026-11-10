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
#include <vector>

#include "app_event.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t HALF_DIVISOR = 2;
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t MAX_LEN_OF_DOMAIN = 32;
constexpr size_t MAX_LEN_OF_EVENT_NAME = 48;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int FIRST_EVENT_TYPE = 1;
}

void FuzzAppEvent(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    size_t half = size / HALF_DIVISOR;
    std::string domain(reinterpret_cast<const char*>(data), std::min(half, MAX_LEN_OF_DOMAIN));
    std::string name(reinterpret_cast<const char*>(data + half),
        std::min(size - half, MAX_LEN_OF_EVENT_NAME));
    auto eventType = static_cast<OHOS::HiviewDFX::HiAppEvent::EventType>(
        data[0] % EVENT_TYPE_COUNT + FIRST_EVENT_TYPE);
    OHOS::HiviewDFX::HiAppEvent::Event event(domain, name, eventType);
    event.AddParam("k_bool", static_cast<bool>(data[0]));
    event.AddParam("k_int32", static_cast<int32_t>(data[0]));
    event.AddParam("k_int64", static_cast<int64_t>(data[0]));
    event.AddParam("k_double", static_cast<double>(data[0]));
    event.AddParam("k_str", std::string("v"));
    std::vector<bool> boolValues = { static_cast<bool>(data[0]) };
    event.AddParam("k_bool_arr", boolValues);
    std::vector<int32_t> int32Values = { static_cast<int32_t>(data[0]) };
    event.AddParam("k_int32_arr", int32Values);
    std::vector<int64_t> int64Values = { static_cast<int64_t>(data[0]) };
    event.AddParam("k_int64_arr", int64Values);
    std::vector<double> doubleValues = { static_cast<double>(data[0]) };
    event.AddParam("k_double_arr", doubleValues);
    std::vector<std::string> strValues = { "a", "b" };
    event.AddParam("k_str_arr", strValues);
    OHOS::HiviewDFX::HiAppEvent::Write(event);
}
