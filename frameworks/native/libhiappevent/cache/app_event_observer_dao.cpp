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
#include "app_event_observer_dao.h"

#include "app_event_cache_common.h"
#include "app_event_store.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventAppEventObserverDao"

namespace OHOS {
namespace HiviewDFX {
using namespace AppEventCacheCommon;

AppEventObserverDao::AppEventObserverDao(std::shared_ptr<NativeRdb::RdbStore> dbStore) : dbStore_(dbStore)
{
    if (Create() != DB_SUCC) {
        HILOG_ERROR(LOG_CORE, "failed to create table=%{public}s", Observers::TABLE.c_str());
    }
}

int AppEventObserverDao::Create()
{
    /**
     * table: observers
     *
     * |-------|------|------|
     * |  seq  | name | hash |
     * |-------|------|------|
     * | INT64 | TEXT | INT64|
     * |-------|------|------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {Observers::FIELD_NAME, SqlUtil::SQL_TEXT_TYPE},
        {Observers::FIELD_HASH, SqlUtil::SQL_INT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(Observers::TABLE, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return DB_SUCC;
}

int64_t AppEventObserverDao::Insert(const std::string& observer, int64_t hashCode)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(Observers::FIELD_NAME, observer);
    bucket.PutLong(Observers::FIELD_HASH, hashCode);
    int64_t seq = 0;
    if (dbStore_->Insert(seq, Observers::TABLE, bucket) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return seq;
}

int64_t AppEventObserverDao::QuerySeq(const std::string& observer, int64_t hashCode)
{
    NativeRdb::AbsRdbPredicates predicates(Observers::TABLE);
    predicates.EqualTo(Observers::FIELD_NAME, observer);
    predicates.EqualTo(Observers::FIELD_HASH, hashCode);
    auto resultSet = dbStore_->Query(predicates, {Observers::FIELD_SEQ});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, observer name=%{public}s, hash code=%{public}" PRId64,
            observer.c_str(), hashCode);
        return DB_FAILED;
    }

    // the hash code is unique, so get only the first
    int64_t observerSeq = 0;
    if (resultSet->GoToNextRow() == NativeRdb::E_OK && resultSet->GetLong(0, observerSeq) == NativeRdb::E_OK) {
        HILOG_INFO(LOG_CORE, "succ to query observer seq=%{public}" PRId64 ", name=%{public}s, hash=%{public}" PRId64,
            observerSeq, observer.c_str(), hashCode);
        resultSet->Close();
        return observerSeq;
    }
    resultSet->Close();
    return DB_FAILED;
}

int AppEventObserverDao::QuerySeqs(const std::string& observer, std::vector<int64_t>& observerSeqs)
{
    NativeRdb::AbsRdbPredicates predicates(Observers::TABLE);
    predicates.EqualTo(Observers::FIELD_NAME, observer);
    auto resultSet = dbStore_->Query(predicates, {Observers::FIELD_SEQ});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, observer=%{public}s", observer.c_str());
        return DB_FAILED;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        int64_t observerSeq = 0;
        if (resultSet->GetLong(0, observerSeq) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get seq value from resultSet, observer=%{public}s", observer.c_str());
            continue;
        }
        observerSeqs.emplace_back(observerSeq);
    }
    resultSet->Close();
    return DB_SUCC;
}

int AppEventObserverDao::Delete(const std::string& observer)
{
    int deleteRows = 0;
    NativeRdb::AbsRdbPredicates predicates(Observers::TABLE);
    predicates.EqualTo(Observers::FIELD_NAME, observer);
    if (dbStore_->Delete(deleteRows, predicates) != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to delete records, observer=%{public}s", observer.c_str());
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d records, observer=%{public}s", deleteRows, observer.c_str());
    return deleteRows;
}

int AppEventObserverDao::Delete(int64_t observerSeq)
{
    int deleteRows = 0;
    NativeRdb::AbsRdbPredicates predicates(Observers::TABLE);
    predicates.EqualTo(Observers::FIELD_SEQ, observerSeq);
    if (dbStore_->Delete(deleteRows, predicates) != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to delete records, observer seq=%{public}" PRId64, observerSeq);
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d records, observerSeq=%{public}" PRId64, deleteRows, observerSeq);
    return deleteRows;
}
} // namespace HiviewDFX
} // namespace OHOS
