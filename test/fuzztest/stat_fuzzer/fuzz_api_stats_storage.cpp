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
#include "api_stats_storage.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
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

void FuzzApiStatsStorage(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string k = FS(data, size, FUZZ_KEY_MAX_LEN);
    std::string a = FS2(data, size, FUZZ_KEY_MAX_LEN);
    OHOS::HiviewDFX::HiAppEvent::ApiMetricsMap mm;
    OHOS::HiviewDFX::HiAppEvent::ApiDescriptor d(k, a);
    OHOS::HiviewDFX::HiAppEvent::ApiMetric m;
    m.errCode = data[0];
    m.duration = data[1];
    m.successful = true;
    mm[d].push_back(m);
    auto& st = OHOS::HiviewDFX::HiAppEvent::ApiStatsStorage::GetInstance();
    st.Backup(mm);
    OHOS::HiviewDFX::HiAppEvent::ApiMetricsMap o;
    st.QueryAll(o);
    st.Clear();
    st.Backup({});
}
