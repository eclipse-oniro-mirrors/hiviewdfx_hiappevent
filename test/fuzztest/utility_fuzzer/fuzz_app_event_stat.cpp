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
#include "app_event_stat.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr int BOOL_VALUE_COUNT = 2;
constexpr int API_TYPE_COUNT = 4;
constexpr int BOUNDARY_MOD = 8;
constexpr int BUCKET_COUNT = 20;
constexpr int COUNT_MIN = 0;
constexpr int COUNT_MAX = 100;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t SECOND_DATA_INDEX = 1;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

static OHOS::HiviewDFX::ApiType ToApiType(uint8_t value)
{
    return static_cast<OHOS::HiviewDFX::ApiType>(value % API_TYPE_COUNT);
}

void FuzzAppEventStat(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string a = FS(data, size, FUZZ_KEY_MAX_LEN);
    OHOS::HiviewDFX::AppEventStat::WriteApiEndEventAsync(a, data[0],
        data[0] % BOOL_VALUE_COUNT, data[SECOND_DATA_INDEX % size], ToApiType(data[0]));
    OHOS::HiviewDFX::AppEventStat::WriteApiEndMetric(a, data[0],
        data[0] % BOOL_VALUE_COUNT, data[SECOND_DATA_INDEX % size]);
}

void FuzzApiCallScopedStat(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string a = FS(data, size, FUZZ_KEY_MAX_LEN);
    {
        OHOS::HiviewDFX::ApiCallScopedStat statOnlyTime(a, ToApiType(data[0]));
        statOnlyTime.SetErrorCode(static_cast<uint32_t>(data[0]));
    }
    {
        OHOS::HiviewDFX::ApiCallScopedStat statWithErr(a, ToApiType(data[0]),
            static_cast<int32_t>(data[SECOND_DATA_INDEX % size] % BOUNDARY_MOD));
        statWithErr.SetErrorCode(static_cast<uint32_t>(data[0]));
    }
    OHOS::HiviewDFX::AppEventStat::RecordApiTimeStat(a, data[0], ToApiType(data[0]));
    OHOS::HiviewDFX::AppEventStat::RecordApiEnumStat(a, a, static_cast<int32_t>(data[0]),
        static_cast<int32_t>(data[0] % BOUNDARY_MOD), ToApiType(data[0]));
    OHOS::HiviewDFX::AppEventStat::RecordApiBoolStat(a, a, data[0] % BOOL_VALUE_COUNT == 0,
        ToApiType(data[0]));
    OHOS::HiviewDFX::AppEventStat::RecordApiCountStat(a, a, static_cast<int32_t>(data[0]),
        {COUNT_MIN, COUNT_MAX, BUCKET_COUNT}, ToApiType(data[0]));
    OHOS::HiviewDFX::AppEventStat::RecordConfigBoolStat(a, a, data[0] % BOOL_VALUE_COUNT == 0);
    OHOS::HiviewDFX::AppEventStat::RecordConfigCountStat(a, a, static_cast<int32_t>(data[0]),
        {COUNT_MIN, COUNT_MAX, BUCKET_COUNT});
    OHOS::HiviewDFX::RecordApiParamEnumStat(a, a, static_cast<int32_t>(data[0]),
        static_cast<int32_t>(data[0] % BOUNDARY_MOD), ToApiType(data[0]));
}
