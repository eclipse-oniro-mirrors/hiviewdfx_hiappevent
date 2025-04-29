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
#define LOG_TAG "EventMappingDao"

namespace OHOS {
namespace HiviewDFX {
namespace AppEventMappingDao {
using namespace AppEventCacheCommon;
using namespace AppEventCacheCommon::AppEventMapping;
int Create(NativeRdb::RdbStore& dbStore)
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
    return dbStore.ExecuteSql(sql);
}

int Insert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::vector<EventObserverInfo>& eventObservers)
{
    std::vector<NativeRdb::ValuesBucket> buckets;
    for (const auto& eventObserver : eventObservers) {
        NativeRdb::ValuesBucket bucket;
        bucket.PutLong(FIELD_EVENT_SEQ, eventObserver.eventSeq);
        bucket.PutLong(FIELD_OBSERVER_SEQ, eventObserver.observerSeq);
        buckets.emplace_back(bucket);
    }
    int64_t insertRows = 0;
    return dbStore->BatchInsert(insertRows, TABLE, buckets);
}

int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
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
    int ret = dbStore->Delete(deleteRows, predicates);
    HILOG_INFO(LOG_CORE, "delete %{public}d records, observerSeq=%{public}" PRId64 ", ret=%{public}d",
        deleteRows, observerSeq, ret);
    return ret;
}

int QueryExistEvent(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::vector<int64_t>& eventSeqs,
    std::unordered_set<int64_t>& existEventSeqs)
{
    if (eventSeqs.empty()) {
        return NativeRdb::E_OK;
    }
    NativeRdb::AbsRdbPredicates predicates(TABLE);
    std::vector<std::string> eventSeqStrs(eventSeqs.size());
    std::transform(eventSeqs.begin(), eventSeqs.end(), eventSeqStrs.begin(), [](int64_t eventSeq) {
        return std::to_string(eventSeq);
    });
    predicates.In(FIELD_EVENT_SEQ, eventSeqStrs);

    auto resultSet = dbStore->Query(predicates, {FIELD_EVENT_SEQ});
    if (resultSet == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to query table, event size=%{public}zu", eventSeqs.size());
        return NativeRdb::E_ERROR;
    }
    int ret = resultSet->GoToNextRow();
    while (ret == NativeRdb::E_OK) {
        int64_t existEventSeq = 0;
        if (resultSet->GetLong(0, existEventSeq) != NativeRdb::E_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get event seq value from resultSet");
            resultSet->Close();
            return NativeRdb::E_ERROR;
        }
        existEventSeqs.insert(existEventSeq);
        ret = resultSet->GoToNextRow();
    }
    resultSet->Close();
    return ret == NativeRdb::E_SQLITE_CORRUPT ? ret : NativeRdb::E_OK;
}
} // namespace AppEventMappingDao
} // namespace HiviewDFX
} // namespace OHOS
