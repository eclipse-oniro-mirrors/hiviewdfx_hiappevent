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

#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_AGGREGATOR_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_AGGREGATOR_H

#include <string>
#include <vector>

#include "api_stats_types.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

struct ApiStatsReport {
    std::string kitName;
    std::string apiName;
    int64_t beginTime;
    int callTimes;
    int successTimes;
    int64_t maxCostTime;
    int64_t minCostTime;
    int64_t totalCostTime;
    std::vector<std::string> errorCodeTypes;
    std::vector<int> errorCodeNum;
};

class ApiStatsAggregator {
public:
    void Record(ApiDescriptor descriptor, ApiMetric metric);
    void ClearRecord();
    void MarkAsBackuped();
    bool IsUpdatedAfterLastBackup();
    ApiMetricsMap GetApiMetrics() const;
    static std::vector<ApiStatsReport> AggregateStats(const ApiMetricsMap& apiMetrics);
 
private:
    ApiMetricsMap apiMetricsMap_;
    bool updatedAfterLastBackup_ = false;
 
    void MarkAsUpdated();
};

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_AGGREGATOR_H
