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
#include "custom_event_param_dao.h"

#include "app_event_cache_common.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "CustomEventParamDao"

namespace OHOS {
namespace HiviewDFX {
namespace CustomEventParamDao {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::CustomEventParams;
int Create(NativeRdb::RdbStore& dbStore)
{
    /**
     * table: custom_event_params
     *
     * |-------|------------|--------|------|-----------|-------------|------------|
     * |  seq  | running_id | domain | name | param_key | param_value | param_type |
     * |-------|----------- |--------|------|-----------|-------------|------------|
     * | INT64 |    TEXT    |  TEXT  | TEXT |   TEXT    |    TEXT     |    INT     |
     * |-------|------------|--------|------|-----------|-------------|------------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {FIELD_RUNNING_ID, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_DOMAIN, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_NAME, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_PARAM_KEY, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_PARAM_VALUE, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_PARAM_TYPE, SqlUtil::SQL_INT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(TABLE, fields);
    return dbStore.ExecuteSql(sql);
}

int BatchInsert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const CustomEvent& customEvent)
{
    std::vector<NativeRdb::ValuesBucket> buckets;
    for (const auto& param : customEvent.params) {
        NativeRdb::ValuesBucket bucket;
        bucket.PutString(FIELD_RUNNING_ID, customEvent.runningId);
        bucket.PutString(FIELD_DOMAIN, customEvent.domain);
        bucket.PutString(FIELD_NAME, customEvent.name);
        bucket.PutString(FIELD_PARAM_KEY, param.key);
        bucket.PutString(FIELD_PARAM_VALUE, param.value);
        bucket.PutInt(FIELD_PARAM_TYPE, param.type);
        buckets.emplace_back(bucket);
    }
    int64_t insertRows = 0;
    return dbStore->BatchInsert(insertRows, TABLE, buckets);
}

int Updates(std::shared_ptr<NativeRdb::RdbStore> dbStore, const CustomEvent& customEvent)
{
    for (const auto& param : customEvent.params) {
        NativeRdb::ValuesBucket bucket;
        bucket.PutString(FIELD_PARAM_VALUE, param.value);
        bucket.PutInt(FIELD_PARAM_TYPE, param.type);

        int changedRows = 0;
        NativeRdb::AbsRdbPredicates predicates(TABLE);
        predicates.EqualTo(FIELD_RUNNING_ID, customEvent.runningId);
        predicates.EqualTo(FIELD_DOMAIN, customEvent.domain);
        predicates.EqualTo(FIELD_NAME, customEvent.name);
        predicates.EqualTo(FIELD_PARAM_KEY, param.key);
        if (int ret = dbStore->Update(changedRows, bucket, predicates); ret != NativeRdb::E_OK) {
            return ret;
        }
    }
    return NativeRdb::E_OK;
}

int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    int deleteRows = 0;
    int ret = dbStore->Delete(deleteRows, predicates);
    HILOG_INFO(LOG_CORE, "delete %{public}d records, ret=%{public}d", deleteRows, ret);
    return ret;
}

int QueryParamkeys(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::unordered_set<std::string>& out,
    const CustomEvent& customEvent)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_RUNNING_ID, customEvent.runningId);
    predicates.EqualTo(FIELD_DOMAIN, customEvent.domain);
    predicates.EqualTo(FIELD_NAME, customEvent.name);
    auto resultSet = dbStore->Query(predicates, {FIELD_PARAM_KEY});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return NativeRdb::E_ERROR;
    }
    int ret = resultSet->GoToNextRow();
    while (ret == NativeRdb::E_OK) {
        std::string paramKey;
        if (resultSet->GetString(0, paramKey) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get value, runningId=%{public}s, domain=%{public}s, name=%{public}s",
                customEvent.runningId.c_str(), customEvent.domain.c_str(), customEvent.name.c_str());
            ret = resultSet->GoToNextRow();
            continue;
        }
        out.insert(paramKey);
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}

int Query(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::unordered_map<std::string, std::string>& params,
    const CustomEvent& customEvent)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_RUNNING_ID, customEvent.runningId);
    predicates.EqualTo(FIELD_DOMAIN, customEvent.domain);
    predicates.EqualTo(FIELD_NAME, customEvent.name);
    auto resultSet = dbStore->Query(predicates, {FIELD_PARAM_KEY, FIELD_PARAM_VALUE});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return NativeRdb::E_ERROR;
    }
    int ret = resultSet->GoToNextRow();
    while (ret == NativeRdb::E_OK) {
        std::string paramKey;
        std::string paramValue;
        if (resultSet->GetString(0, paramKey) != NativeRdb::E_OK
            || resultSet->GetString(1, paramValue) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get value, runningId=%{public}s, domain=%{public}s, name=%{public}s",
                customEvent.runningId.c_str(), customEvent.domain.c_str(), customEvent.name.c_str());
            ret = resultSet->GoToNextRow();
            continue;
        }
        params[paramKey] = paramValue;
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}
} // namespace CustomEventParamDao
} // namespace HiviewDFX
} // namespace OHOS
