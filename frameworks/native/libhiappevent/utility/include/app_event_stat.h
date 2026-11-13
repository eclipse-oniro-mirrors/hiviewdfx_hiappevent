/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_APP_EVENT_STAT_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_APP_EVENT_STAT_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace HiviewDFX {
enum class ApiType {
    UNKNOWN = 0,
    NATIVE = 1,
    NAPI = 2,
    ANI = 3,
};

#if defined(ENABLE_API_METRICS)
class ApiCallScopedStat {
public:
    ApiCallScopedStat(const std::string& apiName, ApiType apiType);
    ApiCallScopedStat(const std::string& apiName, ApiType apiType, int32_t errBoundary);
    ~ApiCallScopedStat();

    ApiCallScopedStat(const ApiCallScopedStat&) = delete;
    ApiCallScopedStat& operator=(const ApiCallScopedStat&) = delete;

    void SetErrorCode(uint32_t errCode);

private:
    uint64_t beginTime_;
    ApiType apiType_;
    std::string apiName_;
    uint32_t errCode_;
    int32_t errBoundary_;
};
#else
class ApiCallScopedStat {
public:
    ApiCallScopedStat(const std::string&, ApiType) {}
    ApiCallScopedStat(const std::string&, ApiType, int32_t) {}
    ~ApiCallScopedStat() = default;

    ApiCallScopedStat(const ApiCallScopedStat&) = delete;
    ApiCallScopedStat& operator=(const ApiCallScopedStat&) = delete;

    void SetErrorCode(uint32_t) {}
};
#endif

void RecordApiParamEnumStat(const std::string& apiName, const std::string& enumName, int32_t sample,
    int32_t boundary, ApiType apiType);

namespace AppEventStat {
struct CountBucket {
    int32_t min;
    int32_t max;
    int32_t bucketCount;
};

constexpr int SUCCESS = 0;
constexpr int FAILED = 1;

void WriteApiEndEventAsync(const std::string& apiName, uint64_t beginTime, int result, int errCode,
    ApiType apiType);
int WriteApiEndMetric(const std::string& apiName, uint64_t beginTime, int result, int errCode);
void RecordApiTimeStat(const std::string& apiName, uint64_t beginTime, ApiType apiType);
void RecordApiEnumStat(const std::string& apiName, const std::string& enumName, int32_t sample, int32_t boundary,
    ApiType apiType);
void RecordApiBoolStat(const std::string& apiName, const std::string& boolName, bool sample, ApiType apiType);
void RecordApiCountStat(const std::string& apiName, const std::string& countName, int32_t sample,
    const CountBucket& bucket, ApiType apiType);
void RecordConfigBoolStat(const std::string& configName, const std::string& itemName, bool sample);
void RecordConfigCountStat(const std::string& configName, const std::string& itemName, int32_t sample,
    const CountBucket& bucket);
} // namespace AppEventStat
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_UTILITY_APP_EVENT_STAT_H
