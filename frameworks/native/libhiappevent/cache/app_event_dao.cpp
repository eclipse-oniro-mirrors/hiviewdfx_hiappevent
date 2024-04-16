/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "app_event_dao.h"

#include "app_event_cache_common.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventAppEventDao"

namespace OHOS {
namespace HiviewDFX {
using namespace AppEventCacheCommon;

AppEventDao::AppEventDao(std::shared_ptr<NativeRdb::RdbStore> dbStore) : dbStore_(dbStore)
{
    if (Create() != DB_SUCC) {
        HILOG_ERROR(LOG_CORE, "failed to create table=%{public}s", Events::TABLE.c_str());
    }
}

int AppEventDao::Create()
{
    /**
     * table: events
     *
     * |-------|--------|------|------|------|-----|-----|----------|---------|----------|------------|--------|
     * ------------|
     * |  seq  | domain | name | type |  tz  | pid | tid | trace_id | span_id | pspan_id | trace_flag | params |
     *  running_id |
     * |-------|--------|------|------|------|-----|-----|----------|---------|----------|------------|--------|
     * ------------|
     * | INT64 |  TEXT  | TEXT |  INT | TEXT | INT | INT |  INT64   |  INT64  |   INT64  |    INT     |  TEXT  |
     *     TEXT    |
     * |-------|--------|------|------|------|-----|-----|----------|---------|----------|------------|--------|
     * ------------|
     */
    const std::vector<std::pair<std::string, std::string>> fields = {
        {Events::FIELD_DOMAIN, SqlUtil::SQL_TEXT_TYPE},
        {Events::FIELD_NAME, SqlUtil::SQL_TEXT_TYPE},
        {Events::FIELD_TYPE, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_TIME, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_TZ, SqlUtil::SQL_TEXT_TYPE},
        {Events::FIELD_PID, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_TID, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_TRACE_ID, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_SPAN_ID, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_PSPAN_ID, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_TRACE_FLAG, SqlUtil::SQL_INT_TYPE},
        {Events::FIELD_PARAMS, SqlUtil::SQL_TEXT_TYPE},
        {Events::FIELD_RUNNING_ID, SqlUtil::SQL_TEXT_TYPE},
    };
    std::string sql = SqlUtil::CreateTable(Events::TABLE, fields);
    if (dbStore_->ExecuteSql(sql) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return DB_SUCC;
}

int64_t AppEventDao::Insert(std::shared_ptr<AppEventPack> event)
{
    NativeRdb::ValuesBucket bucket;
    bucket.PutString(Events::FIELD_DOMAIN, event->GetDomain());
    bucket.PutString(Events::FIELD_NAME, event->GetName());
    bucket.PutInt(Events::FIELD_TYPE, event->GetType());
    bucket.PutLong(Events::FIELD_TIME, event->GetTime());
    bucket.PutString(Events::FIELD_TZ, event->GetTimeZone());
    bucket.PutInt(Events::FIELD_PID, event->GetPid());
    bucket.PutInt(Events::FIELD_TID, event->GetTid());
    bucket.PutLong(Events::FIELD_TRACE_ID, event->GetTraceId());
    bucket.PutLong(Events::FIELD_SPAN_ID, event->GetSpanId());
    bucket.PutLong(Events::FIELD_PSPAN_ID, event->GetPspanId());
    bucket.PutInt(Events::FIELD_TRACE_FLAG, event->GetTraceFlag());
    bucket.PutString(Events::FIELD_PARAMS, event->GetParamStr());
    bucket.PutString(Events::FIELD_RUNNING_ID, event->GetRunningId());
    int64_t seq = 0;
    if (dbStore_->Insert(seq, Events::TABLE, bucket) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    return seq;
}

int AppEventDao::Delete(int64_t eventSeq)
{
    NativeRdb::AbsRdbPredicates predicates(Events::TABLE);
    if (eventSeq > 0) {
        predicates.EqualTo(Events::FIELD_SEQ, eventSeq);
    }
    int deleteRows = 0;
    if (dbStore_->Delete(deleteRows, predicates) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d records, eventSeq=%{public}" PRId64, deleteRows, eventSeq);
    return deleteRows;
}
} // namespace HiviewDFX
} // namespace OHOS
