/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "UserPropertyDao"

namespace OHOS {
namespace HiviewDFX {
namespace UserPropertyDao {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::UserProperties;

int Create(NativeRdb::RdbStore& dbStore)
{
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
    return dbStore.ExecuteSql(sql);
}

int Insert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name, const std::string& value)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_NAME, name);
    bucket.PutString(FIELD_VALUE, value);
    int64_t seq = 0;
    int ret = dbStore->Insert(seq, TABLE, bucket);
    HILOG_INFO(LOG_CORE, "insert user property, name=%{public}s, ret=%{public}d", name.c_str(), ret);
    return ret;
}

int Update(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name, const std::string& value)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_NAME, name);
    bucket.PutString(FIELD_VALUE, value);

    int changedRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, name);
    int ret = dbStore->Update(changedRows, bucket, predicates);
    HILOG_INFO(LOG_CORE, "update %{public}d user property, name=%{public}s, ret=%{public}d",
        changedRows, name.c_str(), ret);
    return ret;
}

int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name)
{
    int deleteRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    if (!name.empty()) {
        predicates.EqualTo(FIELD_NAME, name);
    }
    int ret = dbStore->Delete(deleteRows, predicates);
    HILOG_INFO(LOG_CORE, "delete %{public}d user property, name=%{public}s, ret=%{public}d",
        deleteRows, name.c_str(), ret);
    return ret;
}

int Query(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name, std::string& out)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, name);
    auto resultSet = dbStore->Query(predicates, {FIELD_VALUE});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, name=%{public}s", name.c_str());
        return NativeRdb::E_ERROR;
    }
    int ret = resultSet->GoToNextRow();
    if (ret == NativeRdb::E_OK && resultSet->GetString(0, out) != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get value from resultSet, name=%{public}s", name.c_str());
        resultSet->Close();
        return NativeRdb::E_ERROR;
    }
    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}

int QueryAll(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::unordered_map<std::string, std::string>& out)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    auto resultSet = dbStore->Query(predicates, {FIELD_NAME, FIELD_VALUE});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table");
        return NativeRdb::E_ERROR;
    }

    out.clear();
    int ret = resultSet->GoToNextRow();
    while (ret == NativeRdb::E_OK) {
        std::string name;
        std::string value;
        if (resultSet->GetString(0, name) != NativeRdb::E_OK || resultSet->GetString(1, value) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get data from resultSet");
            ret = resultSet->GoToNextRow();
            continue;
        }
        out[name] = value;
        ret = resultSet->GoToNextRow();
    }

    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}
} // namespace UserPropertyDao
} // namespace HiviewDFX
} // namespace OHOS
