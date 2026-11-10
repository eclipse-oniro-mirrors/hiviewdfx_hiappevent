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

#include "appevent_store_init.h"
#include <cstdint>
#include <cstddef>
#include "fuzz_cases.h"


namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;

using FuzzCaseFunc = void (*)(const uint8_t*, size_t);

constexpr FuzzCaseFunc FUZZ_CASES[] = {
    FuzzAppEventDbCleaner,
    FuzzAppEventLogCleaner,
};
constexpr size_t FUZZ_CASE_COUNT = sizeof(FUZZ_CASES) / sizeof(FUZZ_CASES[0]);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::HiviewDFX::AppEventStoreInit::GetInstance(); // init db for all db-dependent cases
    if (data == nullptr || size < MIN_FUZZ_DATA_SIZE) {
        return 0;
    }
    uint8_t selector = data[0] % FUZZ_CASE_COUNT;
    data++;
    size--;
    FUZZ_CASES[selector](data, size);
    return 0;
}
