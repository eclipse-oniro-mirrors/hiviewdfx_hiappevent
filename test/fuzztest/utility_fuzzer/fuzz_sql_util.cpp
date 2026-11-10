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
#include <utility>
#include <vector>
#include "hiappevent_base.h"
#include "sql_util.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

void FuzzSqlUtil(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string t = FS(data, size, FUZZ_KEY_MAX_LEN);
    std::vector<std::pair<std::string, std::string>> f = {
        {"c1", OHOS::HiviewDFX::SqlUtil::SQL_TEXT_TYPE},
        {"c2", OHOS::HiviewDFX::SqlUtil::SQL_INT_TYPE},
    };
    OHOS::HiviewDFX::SqlUtil::CreateTable(t, f);
    OHOS::HiviewDFX::SqlUtil::CreateTable("", f);
    OHOS::HiviewDFX::SqlUtil::CreateTable(t, {});
}
