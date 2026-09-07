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

#include "api_stats_dao.h"

#include "app_event_cache_common.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ApiStatsDao"

namespace OHOS {
namespace HiviewDFX {
namespace ApiStatsDao {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::ApiStats;
int Create(NativeRdb::RdbStore& dbStore)
{
    /**
     * table: api_stats
     *
     * |----------------|-------------|---------|
     * |    kit_name    |  api_name   | metric  |
     * |----------------|-------------|---------|
     * |     TEXT       |    TEXT     |  TEXT   |
     * |----------------|-------------|---------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {FIELD_KITNAME, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_APINAME, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_METRIC, SqlUtil::SQL_TEXT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(TABLE, fields);
    return dbStore.ExecuteSql(sql);
}

int MetricInsert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& kitName, const std::string& apiName,
    const std::string& metric)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_KITNAME, kitName);
    bucket.PutString(FIELD_APINAME, apiName);
    bucket.PutString(FIELD_METRIC, metric);

    int64_t seq = 0;
    int ret = dbStore->Insert(seq, TABLE, bucket);
    HILOG_DEBUG(LOG_CORE, "insert api stats, kitName=%{public}s, apiName=%{public}s, ret=%{public}d",
        kitName.c_str(), apiName.c_str(), ret);
    return ret;
}

int MetricQueryAll(std::shared_ptr<NativeRdb::RdbStore> dbStore,
    std::map<std::pair<std::string, std::string>, std::vector<std::string>>& out)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    auto resultSet = dbStore->Query(predicates, {FIELD_KITNAME, FIELD_APINAME, FIELD_METRIC});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return NativeRdb::E_ERROR;
    }

    out.clear();
    int ret = NativeRdb::E_OK;
    const int COLUMN_INDEX_KIT = 0;
    const int COLUMN_INDEX_API = 1;
    const int COLUMN_INDEX_METRIC = 2;
    do {
        ret = resultSet->GoToNextRow();
        if (ret == NativeRdb::E_OK) {
            std::string kitName;
            std::string apiName;
            std::string metric;
            if (resultSet->GetString(COLUMN_INDEX_KIT, kitName) == NativeRdb::E_OK &&
                resultSet->GetString(COLUMN_INDEX_API, apiName) == NativeRdb::E_OK &&
                resultSet->GetString(COLUMN_INDEX_METRIC, metric) == NativeRdb::E_OK) {
                out[std::make_pair(kitName, apiName)].emplace_back(metric);
            }
        }
    } while (ret == NativeRdb::E_OK);

    resultSet->Close();
    HILOG_DEBUG(LOG_CORE, "query all api stats, group count=%{public}zu, ret=%{public}d", out.size(), ret);
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}

int MetricClear(std::shared_ptr<NativeRdb::RdbStore> dbStore)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    int deleteRows = 0;
    int ret = dbStore->Delete(deleteRows, predicates);
    HILOG_DEBUG(LOG_CORE, "clear all api stats(%{public}d records), ret=%{public}d", deleteRows, ret);
    return ret;
}
} // namespace ApiStatsDao
} // namespace HiviewDFX
} // namespace OHOS
