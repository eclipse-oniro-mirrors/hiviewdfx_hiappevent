/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "app_event_stat.h"

#include <random>

#include "hiappevent_api_metric.h"
#include "hiappevent_base.h"
#include "hiappevent_write.h"
#include "time_util.h"
#ifdef ENABLE_API_METRICS
#include "histogram_plugin_macros.h"
#endif

namespace OHOS {
namespace HiviewDFX {
namespace {

constexpr const char* API_CALL_BOOL_NAME = "ApiCall";
constexpr const char* ERROR_CODE_ENUM_NAME = "ErrCode";
std::string ApiTypeToString(ApiType apiType)
{
    switch (apiType) {
        case ApiType::NATIVE:
            return "NDK";
        case ApiType::NAPI:
            return "NAPI";
        case ApiType::ANI:
            return "ANI";
        default:
            return "UNKNOWN";
    }
}
}

#if defined(ENABLE_API_METRICS)
ApiCallScopedStat::ApiCallScopedStat(const std::string& apiName, ApiType apiType)
    : beginTime_(static_cast<uint64_t>(TimeUtil::GetElapsedMilliSecondsSinceBoot())), apiType_(apiType),
      apiName_(apiName), errCode_(UINT32_MAX), errBoundary_(0) {}

ApiCallScopedStat::ApiCallScopedStat(const std::string& apiName, ApiType apiType, int32_t errBoundary)
    : beginTime_(static_cast<uint64_t>(TimeUtil::GetElapsedMilliSecondsSinceBoot())), apiType_(apiType),
      apiName_(apiName), errCode_(UINT32_MAX), errBoundary_(errBoundary) {}

ApiCallScopedStat::~ApiCallScopedStat()
{
    AppEventStat::RecordApiTimeStat(apiName_, beginTime_, apiType_);
    AppEventStat::RecordApiBoolStat(apiName_, API_CALL_BOOL_NAME, errCode_ == UINT32_MAX, apiType_);
    if (errCode_ != UINT32_MAX) {
        AppEventStat::RecordApiEnumStat(apiName_, ERROR_CODE_ENUM_NAME, static_cast<int32_t>(errCode_),
            errBoundary_, apiType_);
    }
}

void ApiCallScopedStat::SetErrorCode(uint32_t errCode)
{
    errCode_ = errCode;
}
#endif

void RecordApiParamEnumStat(const std::string& apiName, const std::string& enumName, int32_t sample,
    int32_t boundary, ApiType apiType)
{
    AppEventStat::RecordApiEnumStat(apiName, enumName, sample, boundary, apiType);
}

namespace AppEventStat {
namespace {
constexpr int BEHAVIOR = 4;
#ifdef ENABLE_API_METRICS
constexpr int ERR_CODE_PERMISSION_DENIED = 201; // js common error code
constexpr int ERR_CODE_NOT_SUPPORTED = 801; // js common error code
constexpr int32_t ERR_CODE_STAT_UNKNOWN = 0;
constexpr int32_t ERR_CODE_STAT_PERMISSION_DENIED = 1;
constexpr int32_t ERR_CODE_STAT_PARAM_INVALID = 2;
constexpr int32_t ERR_CODE_STAT_NOT_SUPPORTED = 3;
constexpr int32_t ERR_CODE_STAT_DISABLE = 4;
constexpr int32_t ERR_CODE_STAT_INVALID_DOMAIN = 5;
constexpr int32_t ERR_CODE_STAT_INVALID_NAME = 6;
constexpr int32_t ERR_CODE_STAT_INVALID_PARAM_NUM = 7;
constexpr int32_t ERR_CODE_STAT_INVALID_STR_LEN = 8;
constexpr int32_t ERR_CODE_STAT_INVALID_KEY = 9;
constexpr int32_t ERR_CODE_STAT_INVALID_ARR_LEN = 10;
constexpr int32_t ERR_CODE_STAT_INVALID_CUSTOM_PARAM_NUM = 11;
constexpr int32_t ERR_CODE_STAT_INVALID_WATCHER_NAME = 12;
constexpr int32_t ERR_CODE_STAT_INVALID_FILTER_DOMAIN = 13;
constexpr int32_t ERR_CODE_STAT_INVALID_COND_ROW = 14;
constexpr int32_t ERR_CODE_STAT_INVALID_COND_SIZE = 15;
constexpr int32_t ERR_CODE_STAT_INVALID_COND_TIMEOUT = 16;
constexpr int32_t ERR_CODE_STAT_INVALID_MAX_STORAGE = 17;
constexpr int32_t ERR_CODE_STAT_INVALID_SIZE = 18;
constexpr int32_t ERR_CODE_STAT_INVALID_PARAM_VALUE = 19;
constexpr int32_t MAX_ERR_CODE_STAT = ERR_CODE_STAT_INVALID_PARAM_VALUE;

int32_t MapErrCodeToStat(int errCode)
{
    switch (errCode) {
        case ERR_CODE_PERMISSION_DENIED:
            return ERR_CODE_STAT_PERMISSION_DENIED;
        case JsErrorCode::ERR_PARAM:
            return ERR_CODE_STAT_PARAM_INVALID;
        case ERR_CODE_NOT_SUPPORTED:
            return ERR_CODE_STAT_NOT_SUPPORTED;
        case JsErrorCode::ERR_DISABLE:
            return ERR_CODE_STAT_DISABLE;
        case JsErrorCode::ERR_INVALID_DOMAIN:
            return ERR_CODE_STAT_INVALID_DOMAIN;
        case JsErrorCode::ERR_INVALID_NAME:
            return ERR_CODE_STAT_INVALID_NAME;
        case JsErrorCode::ERR_INVALID_PARAM_NUM:
            return ERR_CODE_STAT_INVALID_PARAM_NUM;
        case JsErrorCode::ERR_INVALID_STR_LEN:
            return ERR_CODE_STAT_INVALID_STR_LEN;
        case JsErrorCode::ERR_INVALID_KEY:
            return ERR_CODE_STAT_INVALID_KEY;
        case JsErrorCode::ERR_INVALID_ARR_LEN:
            return ERR_CODE_STAT_INVALID_ARR_LEN;
        case JsErrorCode::ERR_INVALID_CUSTOM_PARAM_NUM:
            return ERR_CODE_STAT_INVALID_CUSTOM_PARAM_NUM;
        case JsErrorCode::ERR_INVALID_WATCHER_NAME:
            return ERR_CODE_STAT_INVALID_WATCHER_NAME;
        case JsErrorCode::ERR_INVALID_FILTER_DOMAIN:
            return ERR_CODE_STAT_INVALID_FILTER_DOMAIN;
        case JsErrorCode::ERR_INVALID_COND_ROW:
            return ERR_CODE_STAT_INVALID_COND_ROW;
        case JsErrorCode::ERR_INVALID_COND_SIZE:
            return ERR_CODE_STAT_INVALID_COND_SIZE;
        case JsErrorCode::ERR_INVALID_COND_TIMEOUT:
            return ERR_CODE_STAT_INVALID_COND_TIMEOUT;
        case JsErrorCode::ERR_INVALID_MAX_STORAGE:
            return ERR_CODE_STAT_INVALID_MAX_STORAGE;
        case JsErrorCode::ERR_INVALID_SIZE:
            return ERR_CODE_STAT_INVALID_SIZE;
        case JsErrorCode::ERR_INVALID_PARAM_VALUE:
            return ERR_CODE_STAT_INVALID_PARAM_VALUE;
        default:
            return ERR_CODE_STAT_UNKNOWN;
    }
}
#endif

uint64_t RandomNum()
{
    std::random_device seed;
    std::mt19937_64 gen(seed());
    std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());
    return dis(gen);
}
}

void WriteApiEndEventAsync(const std::string& apiName, uint64_t beginTime, int result, int errCode,
    ApiType apiType)
{
#ifdef ENABLE_API_METRICS
    RecordApiBoolStat(apiName, API_CALL_BOOL_NAME, result == AppEventStat::SUCCESS, apiType);
    if (result != AppEventStat::SUCCESS) {
        RecordApiEnumStat(apiName, ERROR_CODE_ENUM_NAME, MapErrCodeToStat(errCode), MAX_ERR_CODE_STAT,
            apiType);
    }
#endif
    if (beginTime == UINT64_MAX) { // get beginTime timestamp failed, beginTime is -1
        return;
    }
    int64_t costTime = TimeUtil::GetElapsedMilliSecondsSinceBoot() - static_cast<int64_t>(beginTime);
    int64_t realEndTime = TimeUtil::GetMilliSecondsTimestamp(CLOCK_REALTIME);
    int64_t realBeginTime = realEndTime - costTime;
    if (costTime < 0 || realBeginTime < 0 || realEndTime < 0) {
        return;
    }
#ifdef ENABLE_API_METRICS
    RecordApiTimeStat(apiName, beginTime, apiType);
#endif
    auto appEventPack = std::make_shared<AppEventPack>("api_diagnostic", "api_exec_end", BEHAVIOR);
    appEventPack->AddParam("trans_id", "transId_" + std::to_string(RandomNum()));
    appEventPack->AddParam("api_name", apiName);
    appEventPack->AddParam("sdk_name", "PerformanceAnalysisKit");
    appEventPack->AddParam("begin_time", realBeginTime);
    appEventPack->AddParam("end_time", realEndTime);
    appEventPack->AddParam("result", result);
    appEventPack->AddParam("error_code", errCode);
    SubmitWritingTask(appEventPack, "appevent_api_end");
}

int WriteApiEndMetric(const std::string& apiName, uint64_t beginTime, int result, int errCode)
{
    if (beginTime == UINT64_MAX) { // get beginTime timestamp failed, beginTime is -1
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    int64_t costTime = TimeUtil::GetElapsedMilliSecondsSinceBoot() - static_cast<int64_t>(beginTime);
    int64_t realEndTime = TimeUtil::GetMilliSecondsTimestamp(CLOCK_REALTIME);
    int64_t realBeginTime = realEndTime - costTime;
    if (costTime < 0 || realEndTime < 0 || realBeginTime < 0) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    HiAppEvent::ApiInfo apiInfo = {
        .kit = "PerformanceAnalysisKit",
        .api = apiName
    };
    HiAppEvent::ApiMetric apiMetric = {
        .errCode = errCode,
        .duration = static_cast<int>(costTime),
        .successful = (result == AppEventStat::SUCCESS)
    };
    return HiAppEvent::ApiMetricProcessor::GetInstance().ProcessApiMetric(apiInfo, apiMetric);
}

void RecordApiTimeStat(const std::string& apiName, uint64_t beginTime, ApiType apiType)
{
#ifdef ENABLE_API_METRICS
    if (beginTime == UINT64_MAX) { // get beginTime timestamp failed, beginTime is -1
        return;
    }
    int64_t costTime = TimeUtil::GetElapsedMilliSecondsSinceBoot() - static_cast<int64_t>(beginTime);
    if (costTime < 0 || costTime >= INT32_MAX) {
        return;
    }
    HISTOGRAM_TIMES("PerformanceAnalysisKit." + ApiTypeToString(apiType) + "." + apiName,
        static_cast<int32_t>(costTime));
#endif
}

void RecordApiEnumStat(const std::string& apiName, const std::string& enumName, int32_t sample, int32_t boundary,
    ApiType apiType)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_ENUMERATION("PerformanceAnalysisKit." + ApiTypeToString(apiType) + "." + apiName + "." + enumName,
        sample, boundary);
#endif
}

void RecordApiBoolStat(const std::string& apiName, const std::string& boolName, bool sample, ApiType apiType)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_BOOLEAN("PerformanceAnalysisKit." + ApiTypeToString(apiType) + "." + apiName + "." + boolName,
        sample);
#endif
}

void RecordApiCountStat(const std::string& apiName, const std::string& countName, int32_t sample,
    const CountBucket& bucket, ApiType apiType)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_CUSTOM_COUNTS("PerformanceAnalysisKit." + ApiTypeToString(apiType) + "." + apiName + "." + countName,
        sample, bucket.min, bucket.max, bucket.bucketCount);
#endif
}

void RecordConfigBoolStat(const std::string& configName, const std::string& itemName, bool sample)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_BOOLEAN("PerformanceAnalysisKit." + configName + "." + itemName, sample);
#endif
}

void RecordConfigCountStat(const std::string& configName, const std::string& itemName, int32_t sample,
    const CountBucket& bucket)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_CUSTOM_COUNTS("PerformanceAnalysisKit." + configName + "." + itemName, sample,
        bucket.min, bucket.max, bucket.bucketCount);
#endif
}
} // namespace AppEventStat
} // namespace HiviewDFX
} // namespace OHOS
