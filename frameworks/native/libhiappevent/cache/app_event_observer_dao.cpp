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
#include "app_event_observer_dao.h"

#include "app_event_store.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ObserverDao"

namespace OHOS {
namespace HiviewDFX {
namespace AppEventObserverDao {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::Observers;
int Create(NativeRdb::RdbStore& dbStore)
{
    /**
     * table: observers
     *
     * |-------|------|------|---------|
     * |  seq  | name | hash | filters |
     * |-------|------|------|---------|
     * | INT64 | TEXT | INT64|   TEXT  |
     * |-------|------|------|---------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {FIELD_NAME, SqlUtil::SQL_TEXT_TYPE},
        {FIELD_HASH, SqlUtil::SQL_INT_TYPE},
        {FIELD_FILTERS, SqlUtil::SQL_TEXT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(TABLE, fields);
    return dbStore.ExecuteSql(sql);
}

int Insert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const Observer& observer, int64_t& seq)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_NAME, observer.name);
    bucket.PutLong(FIELD_HASH, observer.hashCode);
    bucket.PutString(FIELD_FILTERS, observer.filters);
    return dbStore->Insert(seq, TABLE, bucket);
}

int Update(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t seq, const std::string& filters)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(FIELD_FILTERS, filters);

    int changedRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_SEQ, seq);
    int ret = dbStore->Update(changedRows, bucket, predicates);
    HILOG_INFO(LOG_CORE, "update %{public}d observer seq=%{public}" PRId64 ", filters=%{public}s, ret=%{public}d",
        changedRows, seq, filters.c_str(), ret);
    return ret;
}

int QuerySeqAndFilters(std::shared_ptr<NativeRdb::RdbStore> dbStore, const Observer& observer,
    int64_t& seq, std::string& filters)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, observer.name);
    predicates.EqualTo(FIELD_HASH, observer.hashCode);
    auto resultSet = dbStore->Query(predicates, {FIELD_SEQ, FIELD_FILTERS});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, observer name=%{public}s, hash code=%{public}" PRId64,
            observer.name.c_str(), observer.hashCode);
        return NativeRdb::E_ERROR;
    }

    // the hash code is unique, so get only the first
    int ret = resultSet->GoToNextRow();
    if (ret == NativeRdb::E_OK && resultSet->GetLong(0, seq) == NativeRdb::E_OK
        && resultSet->GetString(1, filters) == NativeRdb::E_OK) {
        HILOG_INFO(LOG_CORE, "succ to query observer seq=%{public}" PRId64 ", name=%{public}s, hash=%{public}" PRId64,
            seq, observer.name.c_str(), observer.hashCode);
    }
    resultSet->Close();
    return ret;
}

int QuerySeqs(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name,
    std::vector<int64_t>& observerSeqs)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, name);
    auto resultSet = dbStore->Query(predicates, {FIELD_SEQ});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, observer=%{public}s", name.c_str());
        return NativeRdb::E_ERROR;
    }
    int ret = resultSet->GoToNextRow();
    while (ret == NativeRdb::E_OK) {
        int64_t observerSeq = 0;
        if (resultSet->GetLong(0, observerSeq) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get seq value from resultSet, observer=%{public}s", name.c_str());
            ret = resultSet->GoToNextRow();
            continue;
        }
        observerSeqs.emplace_back(observerSeq);
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}

int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name)
{
    int deleteRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_NAME, name);
    int ret = dbStore->Delete(deleteRows, predicates);
    HILOG_INFO(LOG_CORE, "delete %{public}d records, observer=%{public}s, ret=%{public}d",
        deleteRows, name.c_str(), ret);
    return ret;
}

int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t observerSeq)
{
    int deleteRows = 0;
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_SEQ, observerSeq);
    int ret = dbStore->Delete(deleteRows, predicates);
    HILOG_INFO(LOG_CORE, "delete %{public}d records, observerSeq=%{public}" PRId64 ", ret=%{public}d",
        deleteRows, observerSeq, ret);
    return ret;
}

int QueryWatchers(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::vector<Observer>& observers)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    predicates.EqualTo(FIELD_HASH, 0); // hash = 0 means watcher
    auto resultSet = dbStore->Query(predicates, {FIELD_SEQ, FIELD_NAME, FIELD_FILTERS});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query watcher from observers");
        return NativeRdb::E_ERROR;
    }
    int ret = resultSet->GoToNextRow();
    while (ret == NativeRdb::E_OK) {
        int64_t seq = 0;
        std::string name;
        std::string filters;
        if (resultSet->GetLong(0, seq) != NativeRdb::E_OK // 0 means index of seq
            || resultSet->GetString(1, name) != NativeRdb::E_OK // 1 means index of name
            || resultSet->GetString(2, filters) != NativeRdb::E_OK) { // 2 means index of filters
            HILOG_ERROR(LOG_CORE, "failed to get value from resultSet");
            ret = resultSet->GoToNextRow();
            continue;
        }
        observers.emplace_back(Observer(seq, name, filters));
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}
} // namespace AppEventObserverDao
} // namespace HiviewDFX
} // namespace OHOS
