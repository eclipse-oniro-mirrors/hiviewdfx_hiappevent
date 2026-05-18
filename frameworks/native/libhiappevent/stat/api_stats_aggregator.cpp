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

#include "api_stats_aggregator.h"
#include "hilog/log.h"
#include "time_util.h"

#include <limits>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ApiStatsAggregator"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

void ApiStatsAggregator::Record(ApiDescriptor descriptor, ApiMetric metric)
{
    HILOG_DEBUG(LOG_CORE, "Record: kitName=%{public}s, apiName=%{public}s",
        descriptor.KitName().c_str(), descriptor.ApiName().c_str());
    apiMetricsMap_[descriptor].emplace_back(metric);
    MarkAsUpdated();
}

void ApiStatsAggregator::ClearRecord()
{
    HILOG_DEBUG(LOG_CORE, "ClearRecord");
    apiMetricsMap_.clear();
    MarkAsBackuped();
}

void ApiStatsAggregator::MarkAsUpdated()
{
    updatedAfterLastBackup_ = true;
}

void ApiStatsAggregator::MarkAsBackuped()
{
    updatedAfterLastBackup_ = false;
}

bool ApiStatsAggregator::IsUpdatedAfterLastBackup()
{
    return updatedAfterLastBackup_;
}

ApiMetricsMap ApiStatsAggregator::GetApiMetrics() const
{
    return apiMetricsMap_;
}

std::vector<ApiStatsReport> ApiStatsAggregator::AggregateStats(const ApiMetricsMap& apiMetrics)
{
    HILOG_DEBUG(LOG_CORE, "AggregateStats: input count=%{public}zu", apiMetrics.size());
    static int64_t SIXTY_SECONDS_MS = 60 * 1000;
    int64_t beginTime = TimeUtil::GetMilliSecondsTimestamp(CLOCK_REALTIME) - SIXTY_SECONDS_MS;
    std::vector<ApiStatsReport> reports;
    
    for (const auto& [descriptor, metrics] : apiMetrics) {
        if (metrics.empty()) {
            continue;
        }
        
        ApiStatsReport report;
        report.kitName = descriptor.KitName();
        report.apiName = descriptor.ApiName();
        report.beginTime = beginTime;
        report.callTimes = static_cast<int>(metrics.size());
        report.successTimes = 0;
        report.maxCostTime = 0;
        report.minCostTime = std::numeric_limits<int64_t>::max();
        report.totalCostTime = 0;
        std::map<int, int> errorCodeMap;
        
        for (const auto& metric : metrics) {
            if (metric.successful) {
                report.successTimes++;
            }
            report.totalCostTime += metric.duration;
            if (metric.duration > report.maxCostTime) {
                report.maxCostTime = metric.duration;
            }
            if (metric.duration < report.minCostTime) {
                report.minCostTime = metric.duration;
            }
            errorCodeMap[metric.errCode]++;
        }
        
        for (const auto& [errCode, count] : errorCodeMap) {
            report.errorCodeTypes.push_back(std::to_string(errCode));
            report.errorCodeNum.push_back(count);
        }
        
        reports.push_back(report);
    }
    
    HILOG_DEBUG(LOG_CORE, "AggregateStats: output count=%{public}zu", reports.size());
    return reports;
}

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
