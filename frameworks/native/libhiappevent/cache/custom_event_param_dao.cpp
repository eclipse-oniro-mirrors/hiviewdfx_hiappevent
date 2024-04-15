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
#define LOG_TAG "HiAppEventCustomEventParamDao"

namespace OHOS {
namespace HiviewDFX {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::CustomEventParams;

CustomEventParamDao::CustomEventParamDao(std::shared_ptr<NativeRdb::RdbStore> dbStore) : dbStore_(dbStore)
{
    if (Create() != DB_SUCC) {
        HILOG_ERROR(LOG_CORE, "failed to create table=%{public}s", TABLE.c_str());
    }
}

int CustomEventParamDao::Create()
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }
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
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return DB_SUCC;
}

int64_t CustomEventParamDao::Insert(const CustomEventParam& param,
    const std::string& runningId, const std::string& domain, const std::string& name)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_RUNNING_ID, runningId);
    bucket.PutString(FIELD_DOMAIN, domain);
    bucket.PutString(FIELD_NAME, name);
    bucket.PutString(FIELD_PARAM_KEY, param.key);
    bucket.PutString(FIELD_PARAM_VALUE, param.value);
    bucket.PutInt(FIELD_PARAM_TYPE, param.type);
    int64_t seq = 0;
    if (dbStore_->Insert(seq, TABLE, bucket) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return seq;
}

int64_t CustomEventParamDao::Update(const CustomEventParam& param,
    const std::string& runningId, const std::string& domain, const std::string& name)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }

    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_PARAM_VALUE, param.value);
    bucket.PutInt(FIELD_PARAM_TYPE, param.type);

    int changedRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_RUNNING_ID, runningId);
    predicates.EqualTo(FIELD_DOMAIN, domain);
    predicates.EqualTo(FIELD_NAME, name);
    predicates.EqualTo(FIELD_PARAM_KEY, param.key);
    if (dbStore_->Update(changedRows, bucket, predicates) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return changedRows;
}

int CustomEventParamDao::Delete(const std::string& runningId)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    if (runningId.empty()) {
        return DB_SUCC;
    }
    predicates.EqualTo(FIELD_RUNNING_ID, runningId);
    int deleteRows = 0;
    if (dbStore_->Delete(deleteRows, predicates) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d records, runningId=%{public}s", deleteRows, runningId.c_str());
    return deleteRows;
}

int CustomEventParamDao::QueryParamkeys(std::unordered_set<std::string>& out,
    const std::string& runningId, const std::string& domain, const std::string& name)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_RUNNING_ID, runningId);
    predicates.EqualTo(FIELD_DOMAIN, domain);
    predicates.EqualTo(FIELD_NAME, name);
    auto resultSet = dbStore_->Query(predicates, {FIELD_PARAM_KEY});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return DB_FAILED;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string paramKey;
        if (resultSet->GetString(0, paramKey) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get value, runningId=%{public}s, domain=%{public}s, name=%{public}s",
                runningId.c_str(), domain.c_str(), name.c_str());
            continue;
        }
        out.insert(paramKey);
    }
    resultSet->Close();
    return DB_SUCC;
}

int CustomEventParamDao::Query(std::unordered_map<std::string, std::string>& params,
    const std::string& runningId, const std::string& domain, const std::string& name)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_RUNNING_ID, runningId);
    predicates.EqualTo(FIELD_DOMAIN, domain);
    predicates.EqualTo(FIELD_NAME, name);
    auto resultSet = dbStore_->Query(predicates, {FIELD_PARAM_KEY, FIELD_PARAM_VALUE});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return DB_FAILED;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string paramKey;
        std::string paramValue;
        if (resultSet->GetString(0, paramKey) != NativeRdb::E_OK
            || resultSet->GetString(1, paramValue) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get value, runningId=%{public}s, domain=%{public}s, name=%{public}s",
                runningId.c_str(), domain.c_str(), name.c_str());
            continue;
        }
        params[paramKey] = paramValue;
    }
    resultSet->Close();
    return DB_SUCC;
}
} // namespace HiviewDFX
} // namespace OHOS
