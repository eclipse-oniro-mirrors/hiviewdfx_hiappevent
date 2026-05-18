/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
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

#include "api_stats_mgr.h"

#include <cinttypes>

#include "hiappevent_base.h"
#include "hiappevent_write.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ApiStatsMgr"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

ApiStatsManager::ApiStatsManager()
{
    ScheduleReport();
    timer_.SetBackUpCallback([this]() {
        this->ScheduleBackUp();
    });
    timer_.SetReportCallback([this]() {
        this->ScheduleReport();
    });
    timer_.Start();
}

ApiStatsManager::~ApiStatsManager()
{
    timer_.Stop();
}

void ApiStatsManager::AddRecord(ApiDescriptor descriptor, ApiMetric metric)
{
    HILOG_DEBUG(LOG_CORE, "AddRecord: kitName=%{public}s, apiName=%{public}s",
        descriptor.KitName().c_str(), descriptor.ApiName().c_str());
    std::lock_guard<std::mutex> lock(mut_);
    aggregator_.Record(descriptor, metric);
}

void ApiStatsManager::ScheduleBackUpInner()
{
    if (!aggregator_.IsUpdatedAfterLastBackup()) {
        HILOG_INFO(LOG_CORE, "ScheduleBackUpInner: no update, skip");
        return;
    }

    auto apiMetrics = aggregator_.GetApiMetrics();
    HILOG_INFO(LOG_CORE, "ScheduleBackUpInner: backup count=%{public}zu", apiMetrics.size());
    if (ApiStatsStorage::GetInstance().Backup(apiMetrics) == 0) {
        aggregator_.ClearRecord();
        HILOG_DEBUG(LOG_CORE, "ScheduleBackUpInner success");
    } else {
        HILOG_ERROR(LOG_CORE, "ScheduleBackUpInner failed to backup api stats");
    }
}

void ApiStatsManager::ScheduleBackUp()
{
    HILOG_DEBUG(LOG_CORE, "ScheduleBackUp start");
    std::lock_guard<std::mutex> lock(mut_);
    ApiStatsManager::ScheduleBackUpInner();
}

void ApiStatsManager::ScheduleReport()
{
    HILOG_DEBUG(LOG_CORE, "ScheduleReport start");
    std::lock_guard<std::mutex> lock(mut_);
    ApiStatsManager::ScheduleBackUpInner();
    
    ApiMetricsMap apiMetrics;
    if (ApiStatsStorage::GetInstance().QueryAll(apiMetrics) != 0) {
        HILOG_ERROR(LOG_CORE, "failed to query api stats for report");
        return;
    }
    
    HILOG_DEBUG(LOG_CORE, "ScheduleReport: query count=%{public}zu", apiMetrics.size());
    auto reports = ApiStatsAggregator::AggregateStats(apiMetrics);
    
    ReportStats(reports);
    
    if (ApiStatsStorage::GetInstance().Clear() != 0) {
        HILOG_ERROR(LOG_CORE, "failed to clear api stats after report");
    } else {
        HILOG_DEBUG(LOG_CORE, "ScheduleReport: clear success");
    }
}

std::shared_ptr<AppEventPack> ApiStatsManager::ConvertReportToEventPack(const ApiStatsReport& report)
{
    constexpr int BEHAVIOR = 4;
    auto appEventPack = std::make_shared<AppEventPack>("api_diagnostic", "api_called_stat", BEHAVIOR);
    
    appEventPack->AddParam("api_name", report.apiName);
    appEventPack->AddParam("sdk_name", report.kitName);
    appEventPack->AddParam("begin_time", report.beginTime);
    appEventPack->AddParam("call_times", report.callTimes);
    appEventPack->AddParam("success_times", report.successTimes);
    appEventPack->AddParam("max_cost_time", report.maxCostTime);
    appEventPack->AddParam("min_cost_time", report.minCostTime);
    appEventPack->AddParam("total_cost_time", report.totalCostTime);
    appEventPack->AddParam("error_code_types", report.errorCodeTypes);
    appEventPack->AddParam("error_code_num", report.errorCodeNum);

    return appEventPack;
}

void ApiStatsManager::ReportStats(const std::vector<ApiStatsReport>& reports)
{
    if (reports.empty()) {
        HILOG_DEBUG(LOG_CORE, "no api stats to report");
        return;
    }
    
    HILOG_INFO(LOG_CORE, "start to report api stats, count=%{public}zu", reports.size());
    
    for (const auto& report : reports) {
        auto eventPack = ConvertReportToEventPack(report);
        SubmitWritingTask(eventPack, "api_stats_report");
        HILOG_INFO(LOG_CORE, "report api stats: kitName=%{public}s, apiName=%{public}s, callTimes=%{public}d, "
            "successTimes=%{public}d, maxCostTime=%{public}" PRId64 ", minCostTime=%{public}" PRId64 ", "
            "totalCostTime=%{public}" PRId64 ", errorCodeCount=%{public}zu",
            report.kitName.c_str(), report.apiName.c_str(), report.callTimes, report.successTimes,
            report.maxCostTime, report.minCostTime, report.totalCostTime, report.errorCodeTypes.size());
    }
    
    HILOG_INFO(LOG_CORE, "report api stats success");
}

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS