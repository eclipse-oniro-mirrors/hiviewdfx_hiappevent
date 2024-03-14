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
#include "app_event_mapping_dao.h"

#include <algorithm>
#include <cinttypes>

#include "app_event_cache_common.h"
#include "app_event_store.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventAppEvenMappingDao"

namespace OHOS {
namespace HiviewDFX {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::AppEventMapping;
AppEventMappingDao::AppEventMappingDao(std::shared_ptr<NativeRdb::RdbStore> dbStore) : dbStore_(dbStore)
{
    if (Create() != DB_SUCC) {
        HILOG_ERROR(LOG_CORE, "failed to create table=%{public}s", TABLE.c_str());
    }
}

int AppEventMappingDao::Create()
{
    /**
     * table: event_observer_mapping
     *
     * |-------|-----------|--------------|
     * |  seq  | event_seq | observer_seq |
     * |-------|-----------|--------------|
     * | INT64 |   INT64   |    INT64     |
     * |-------|-----------|--------------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {FIELD_EVENT_SEQ, SqlUtil::SQL_INT_TYPE},
        {FIELD_OBSERVER_SEQ, SqlUtil::SQL_INT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(TABLE, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return DB_SUCC;
}

int64_t AppEventMappingDao::Insert(int64_t eventSeq, int64_t observerSeq)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutLong(FIELD_EVENT_SEQ, eventSeq);
    bucket.PutLong(FIELD_OBSERVER_SEQ, observerSeq);
    int64_t seq = 0;
    if (dbStore_->Insert(seq, TABLE, bucket) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return seq;
}

int AppEventMappingDao::Delete(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    if (observerSeq > 0) {
        predicates.EqualTo(FIELD_OBSERVER_SEQ, observerSeq);
    }
    if (!eventSeqs.empty()) {
        std::vector<std::string> eventSeqStrs(eventSeqs.size());
        std::transform(eventSeqs.begin(), eventSeqs.end(), eventSeqStrs.begin(), [](int64_t eventSeq) {
            return std::to_string(eventSeq);
        });
        predicates.In(FIELD_EVENT_SEQ, eventSeqStrs);
    }

    int deleteRows = 0;
    if (dbStore_->Delete(deleteRows, predicates) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d records, observerSeq=%{public}" PRId64, deleteRows, observerSeq);
    return deleteRows;
}
} // namespace HiviewDFX
} // namespace OHOS
