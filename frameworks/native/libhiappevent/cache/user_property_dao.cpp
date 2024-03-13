/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "user_property_dao.h"

#include "app_event_cache_common.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventUserPropertyDao"

namespace OHOS {
namespace HiviewDFX {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::UserProperties;

UserPropertyDao::UserPropertyDao(std::shared_ptr<NativeRdb::RdbStore> dbStore) : dbStore_(dbStore)
{
    if (Create() != DB_SUCC) {
        HILOG_ERROR(LOG_CORE, "failed to create table=%{public}s", TABLE.c_str());
    }
}

int UserPropertyDao::Create()
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }

    /**
     * table: user_properties
     *
     * |-------|--------|-------|
     * |  seq  |  name  | value |
     * |-------|--------|-------|
     * | INT64 |  TEXT  | TEXT  |
     * |-------|--------|-------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {FIELD_NAME, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_VALUE, SqlUtil::SQL_TEXT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(TABLE, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return DB_SUCC;
}

int64_t UserPropertyDao::Insert(const std::string& name, const std::string& value)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_NAME, name);
    bucket.PutString(FIELD_VALUE, value);
    int64_t seq = 0;
    if (dbStore_->Insert(seq, TABLE, bucket) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "insert user property, name=%{public}s, value=%{public}s", name.c_str(), value.c_str());
    return seq;
}

int64_t UserPropertyDao::Update(const std::string& name, const std::string& value)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }

    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_NAME, name);
    bucket.PutString(FIELD_VALUE, value);

    int changedRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, name);
    if (dbStore_->Update(changedRows, bucket, predicates) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "update %{public}d user property, name=%{public}s", changedRows, name.c_str());
    return changedRows;
}

int UserPropertyDao::Delete(const std::string& name)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }

    int deleteRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    if (!name.empty()) {
        predicates.EqualTo(FIELD_NAME, name);
    }
    if (dbStore_->Delete(deleteRows, predicates) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d user property, name=%{public}s", deleteRows, name.c_str());
    return deleteRows;
}

int UserPropertyDao::Query(const std::string& name, std::string& out)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }

    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, name);
    auto resultSet = dbStore_->Query(predicates, {FIELD_VALUE});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, name=%{public}s", name.c_str());
        return DB_FAILED;
    }
    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        if (resultSet->GetString(0, out) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get value from resultSet, name=%{public}s", name.c_str());
            resultSet->Close();
            return DB_FAILED;
        }
    }
    resultSet->Close();
    return DB_SUCC;
}

int UserPropertyDao::QueryAll(std::unordered_map<std::string, std::string>& out)
{
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }

    NativeRdb::AbsRdbPredicates predicates(TABLE);
    auto resultSet = dbStore_->Query(predicates, {FIELD_NAME, FIELD_VALUE});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return DB_FAILED;
    }

    out.clear();
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string name;
        std::string value;
        if (resultSet->GetString(0, name) != NativeRdb::E_OK || resultSet->GetString(1, value) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get data from resultSet");
            continue;
        }
        out[name] = value;
    }

    resultSet->Close();
    return DB_SUCC;
}
} // namespace HiviewDFX
} // namespace OHOS
