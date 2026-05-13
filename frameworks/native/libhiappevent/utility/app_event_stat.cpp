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

#include "hiappevent_base.h"
#include "hiappevent_write.h"
#include "hiappevent_api_metric.h"
#include "time_util.h"
#ifdef ENABLE_API_METRICS
#include "histogram_plugin_macros.h"
#endif

namespace OHOS {
namespace HiviewDFX {
namespace AppEventStat {
namespace {
constexpr int BEHAVIOR = 4;
#ifdef ENABLE_API_METRICS
constexpr int MAX_ERR = 11105001;
#endif

uint64_t RandomNum()
{
    std::random_device seed;
    std::mt19937_64 gen(seed());
    std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<uint64_t>::max());
    return dis(gen);
}
}

void WriteApiEndEventAsync(const std::string& apiName, uint64_t beginTime, int result, int errCode)
{
#ifdef ENABLE_API_METRICS
    HISTOGRAM_BOOLEAN("PerformanceAnalysisKit.ApiCall." + apiName, result == AppEventStat::SUCCESS);
    if (result != AppEventStat::SUCCESS) {
        HISTOGRAM_ENUMERATION("PerformanceAnalysisKit.ErrCode." + apiName, errCode, MAX_ERR);
    }
#endif
    int64_t costTime = TimeUtil::GetElapsedMilliSecondsSinceBoot() - static_cast<int64_t>(beginTime);
    int64_t realEndTime = TimeUtil::GetMilliSecondsTimestamp(CLOCK_REALTIME);
    int64_t realBeginTime = realEndTime - costTime;
    if (realBeginTime < 0 || realEndTime < 0) {
        return;
    }
#ifdef ENABLE_API_METRICS
    HISTOGRAM_TIMES("PerformanceAnalysisKit.Time." + apiName, costTime);
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
    int64_t costTime = TimeUtil::GetElapsedMilliSecondsSinceBoot() - static_cast<int64_t>(beginTime);
    int64_t realEndTime = TimeUtil::GetMilliSecondsTimestamp(CLOCK_REALTIME);
    int64_t realBeginTime = realEndTime - costTime;
    if (realBeginTime < 0) {
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

} // namespace AppEventStat
} // namespace HiviewDFX
} // namespace OHOS
