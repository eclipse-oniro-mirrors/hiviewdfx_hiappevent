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

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include "main_thread_jank_policy.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr int FIRST_MAP_KEY = 0;
}

void FuzzMainThreadJankPolicy(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::map<std::string, std::string> cm = {{"pageSwitchLogEnable", "true"}};
    OHOS::HiviewDFX::MainThreadJankPolicy p;
    p.SetEventPolicy(cm);
    std::map<uint8_t, uint32_t> im = {{FIRST_MAP_KEY, data[0]}};
    p.SetEventPolicy(im);
    OHOS::HiviewDFX::MainThreadJankConfig c;
    c.SetEventPolicy(cm);
    c.SetEventPolicy(im);
}
