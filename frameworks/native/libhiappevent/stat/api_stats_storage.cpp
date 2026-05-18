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

#include "api_stats_storage.h"

#include <unordered_map>

#include "app_event_store.h"
#include "hilog/log.h"
#include "json/json.h"
#include "rdb_errno.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ApiStatsStorage"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
constexpr int DB_FAILED = -1;
constexpr int DB_SUCC = 0;
}

ApiStatsStorage::ApiStatsStorage()
{
}

ApiStatsStorage::~ApiStatsStorage()
{
}

ApiStatsStorage& ApiStatsStorage::GetInstance()
{
    static ApiStatsStorage instance;
    return instance;
}

int ApiStatsStorage::Backup(const ApiMetricsMap& apiMetrics)
{
    HILOG_DEBUG(LOG_CORE, "Backup start: input count=%{public}zu", apiMetrics.size());
    if (apiMetrics.empty()) {
        HILOG_DEBUG(LOG_CORE, "api metrics map is empty, nothing to backup");
        return DB_SUCC;
    }

    auto& appEventStore = AppEventStore::GetInstance();
    for (const auto& [descriptor, metrics] : apiMetrics) {
        std::string kitName = descriptor.KitName();
        std::string apiName = descriptor.ApiName();

        HILOG_DEBUG(LOG_CORE, "Backup: kitName=%{public}s, apiName=%{public}s, metrics=%{public}zu",
            kitName.c_str(), apiName.c_str(), metrics.size());

        for (const auto& metric : metrics) {
            Json::Value metricObj(Json::objectValue);
            metricObj["errCode"] = metric.errCode;
            metricObj["duration"] = metric.duration;
            metricObj["successful"] = metric.successful;
            std::string metricJson = Json::FastWriter().write(metricObj);

            int ret = appEventStore.InsertApiMetricInfo(kitName, apiName, metricJson);
            HILOG_DEBUG(LOG_CORE, "insert metric, kitName=%{public}s, apiName=%{public}s, ret=%{public}d",
                kitName.c_str(), apiName.c_str(), ret);
            if (ret != NativeRdb::E_OK) {
                HILOG_ERROR(LOG_CORE, "failed to backup api metric, kitName=%{public}s, apiName=%{public}s, \
                    ret=%{public}d", kitName.c_str(), apiName.c_str(), ret);
                return DB_FAILED;
            }
        }
    }
    HILOG_INFO(LOG_CORE, "backup api stats success, count=%{public}zu", apiMetrics.size());
    return DB_SUCC;
}

int ApiStatsStorage::QueryAll(ApiMetricsMap& apiMetrics)
{
    HILOG_DEBUG(LOG_CORE, "QueryAll start");
    auto& appEventStore = AppEventStore::GetInstance();
    apiMetrics.clear();

    std::map<std::pair<std::string, std::string>, std::vector<std::string>> results;
    int ret = appEventStore.QueryApiMetricInfoAll(results);
    if (ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to query all api stats, ret=%{public}d", ret);
        return DB_FAILED;
    }

    HILOG_DEBUG(LOG_CORE, "QueryAll: db result count=%{public}zu", results.size());
    for (const auto& [key, metricJsons] : results) {
        const auto& [kitName, apiName] = key;
        std::vector<ApiMetric> metrics;
        for (const auto& metricJson : metricJsons) {
            Json::Value jsonValue;
            Json::Reader reader(Json::Features::strictMode());
            if (!reader.parse(metricJson, jsonValue)) {
                HILOG_WARN(LOG_CORE, "failed to parse metric json, kitName=%{public}s, apiName=%{public}s",
                    kitName.c_str(), apiName.c_str());
                continue;
            }
            if (!jsonValue["errCode"].isInt() || !jsonValue["duration"].isInt() || !jsonValue["successful"].isBool()) {
                HILOG_ERROR(LOG_CORE, "query all failed to parse json");
                continue;
            }
            int errCode = jsonValue["errCode"].asInt();
            int duration = jsonValue["duration"].asInt();
            bool successful = jsonValue["successful"].asBool();
            if (duration < 0) {
                HILOG_ERROR(LOG_CORE, "query all, duration param error");
                continue;
            }
            HILOG_DEBUG(LOG_CORE, "parse metric, kitName=%{public}s, apiName=%{public}s, errCode=%{public}d, \
                duration=%{public}d, successful=%{public}d",
                kitName.c_str(), apiName.c_str(), errCode, duration, successful);
            metrics.emplace_back(ApiMetric{errCode, duration, successful});
        }

        ApiDescriptor descriptor(kitName, apiName);
        apiMetrics[descriptor] = std::move(metrics);
    }

    HILOG_INFO(LOG_CORE, "query all api stats success, count=%{public}zu", apiMetrics.size());
    return DB_SUCC;
}

int ApiStatsStorage::Clear()
{
    HILOG_DEBUG(LOG_CORE, "Clear start");
    auto& appEventStore = AppEventStore::GetInstance();
    int ret = appEventStore.ClearApiMetricInfo();
    HILOG_INFO(LOG_CORE, "clear api stats, ret=%{public}d", ret);
    return ret == NativeRdb::E_OK ? DB_SUCC : DB_FAILED;
}

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
