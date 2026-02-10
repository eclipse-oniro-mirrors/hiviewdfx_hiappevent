/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "app_event_store.h"

#include <cinttypes>
#include <utility>
#include <vector>

#include "app_event_cache_common.h"
#include "app_event_store_callback.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_common.h"
#include "hiappevent_config.h"
#include "hilog/log.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Store"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
using namespace OHOS::HiviewDFX::HiAppEvent;
namespace OHOS {
namespace HiviewDFX {
namespace {
const char* DATABASE_NAME = "appevent.db";
const char* DATABASE_DIR = "databases/";
static constexpr size_t MAX_NUM_OF_CUSTOM_PARAMS = 64;

int GetIntFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet, const std::string& colName)
{
    int value = 0;
    int colIndex = 0;
    if (resultSet->GetColumnIndex(colName, colIndex) != NativeRdb::E_OK) {
        HILOG_WARN(LOG_CORE, "failed to get column index, colName=%{public}s", colName.c_str());
        return value;
    }
    if (resultSet->GetInt(colIndex, value) != NativeRdb::E_OK) {
        HILOG_WARN(LOG_CORE, "failed to get int value, colName=%{public}s", colName.c_str());
    }
    return value;
}

int64_t GetLongFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet, const std::string& colName)
{
    int64_t value = 0;
    int colIndex = 0;
    if (resultSet->GetColumnIndex(colName, colIndex) != NativeRdb::E_OK) {
        HILOG_WARN(LOG_CORE, "failed to get column index, colName=%{public}s", colName.c_str());
        return value;
    }
    if (resultSet->GetLong(colIndex, value) != NativeRdb::E_OK) {
        HILOG_WARN(LOG_CORE, "failed to get long value, colName=%{public}s", colName.c_str());
    }
    return value;
}

std::string GetStringFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet, const std::string& colName)
{
    std::string value;
    int colIndex = 0;
    if (resultSet->GetColumnIndex(colName, colIndex) != NativeRdb::E_OK) {
        HILOG_WARN(LOG_CORE, "failed to get column index, colName=%{public}s", colName.c_str());
        return value;
    }
    if (resultSet->GetString(colIndex, value) != NativeRdb::E_OK) {
        HILOG_WARN(LOG_CORE, "failed to get string value, colName=%{public}s", colName.c_str());
    }
    return value;
}

std::shared_ptr<AppEventPack> GetEventFromResultSet(std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet)
{
    auto event = std::make_shared<AppEventPack>();
    event->SetSeq(GetLongFromResultSet(resultSet, Events::FIELD_SEQ));
    event->SetDomain(GetStringFromResultSet(resultSet, Events::FIELD_DOMAIN));
    event->SetName(GetStringFromResultSet(resultSet, Events::FIELD_NAME));
    event->SetType(GetIntFromResultSet(resultSet, Events::FIELD_TYPE));
    event->SetTime(GetLongFromResultSet(resultSet, Events::FIELD_TIME));
    event->SetTimeZone(GetStringFromResultSet(resultSet, Events::FIELD_TZ));
    event->SetPid(GetIntFromResultSet(resultSet, Events::FIELD_PID));
    event->SetTid(GetIntFromResultSet(resultSet, Events::FIELD_TID));
    event->SetTraceId(GetLongFromResultSet(resultSet, Events::FIELD_TRACE_ID));
    event->SetSpanId(GetLongFromResultSet(resultSet, Events::FIELD_SPAN_ID));
    event->SetPspanId(GetLongFromResultSet(resultSet, Events::FIELD_PSPAN_ID));
    event->SetTraceFlag(GetIntFromResultSet(resultSet, Events::FIELD_TRACE_FLAG));
    event->SetParamStr(GetStringFromResultSet(resultSet, Events::FIELD_PARAMS));
    event->SetRunningId(GetStringFromResultSet(resultSet, Events::FIELD_RUNNING_ID));
    return event;
}

int UpToDbVersion2(NativeRdb::RdbStore& rdbStore)
{
    std::string sql = "ALTER TABLE " + Events::TABLE + " ADD COLUMN "
        + Events::FIELD_RUNNING_ID + " " + SqlUtil::SQL_TEXT_TYPE + " DEFAULT " + "'';";
    return rdbStore.ExecuteSql(sql);
}

int UpToDbVersion3(NativeRdb::RdbStore& rdbStore)
{
    std::string sql = "ALTER TABLE " + Observers::TABLE + " ADD COLUMN "
        + Observers::FIELD_FILTERS + " " + SqlUtil::SQL_TEXT_TYPE + " DEFAULT " + "'';";
    return rdbStore.ExecuteSql(sql);
}
}

int AppEventStoreCallback::OnCreate(NativeRdb::RdbStore& rdbStore)
{
    HILOG_DEBUG(LOG_CORE, "OnCreate start to create db");
    if (int ret = AppEventDao::Create(rdbStore); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create table events, ret=%{public}d", ret);
        return ret;
    }
    if (int ret = AppEventObserverDao::Create(rdbStore); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create table observers, ret=%{public}d", ret);
        return ret;
    }
    if (int ret = AppEventMappingDao::Create(rdbStore); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create table event_observer_mapping, ret=%{public}d", ret);
        return ret;
    }
    if (int ret = UserIdDao::Create(rdbStore); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create table user_ids, ret=%{public}d", ret);
        return ret;
    }
    if (int ret = UserPropertyDao::Create(rdbStore); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create table user_properties, ret=%{public}d", ret);
        return ret;
    }
    if (int ret = CustomEventParamDao::Create(rdbStore); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create table custom_event_params, ret=%{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int AppEventStoreCallback::OnUpgrade(NativeRdb::RdbStore& rdbStore, int oldVersion, int newVersion)
{
    HILOG_DEBUG(LOG_CORE, "OnUpgrade, oldVersion=%{public}d, newVersion=%{public}d", oldVersion, newVersion);
    for (int i = oldVersion; i < newVersion; ++i) {
        switch (i) {
            case 1: // upgrade db version from 1 to 2
                if (int ret = UpToDbVersion2(rdbStore); ret != NativeRdb::E_OK) {
                    HILOG_ERROR(LOG_CORE, "failed to upgrade db version from 1 to 2, ret=%{public}d", ret);
                    return ret;
                }
                break;
            case 2: // upgrade db version from 2 to 3
                if (int ret = UpToDbVersion3(rdbStore); ret != NativeRdb::E_OK) {
                    HILOG_ERROR(LOG_CORE, "failed to upgrade db version from 2 to 3, ret=%{public}d", ret);
                    return ret;
                }
                break;
            default:
                break;
        }
    }
    return NativeRdb::E_OK;
}

AppEventStore::AppEventStore()
{
    (void)InitDbStore();
}

AppEventStore::~AppEventStore()
{
    dbStore_ = nullptr;
}

AppEventStore& AppEventStore::GetInstance()
{
    static AppEventStore instance;
    return instance;
}

int AppEventStore::InitDbStore()
{
    if (!InitDbStoreDir()) {
        return DB_FAILED;
    }

    int ret = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dirPath_ + DATABASE_NAME);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    const int dbVersion = 3; // 3 means new db version
    AppEventStoreCallback callback;
    auto dbStore = NativeRdb::RdbHelper::GetRdbStore(config, dbVersion, callback, ret);
    if (ret != NativeRdb::E_OK || dbStore == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to create db store, ret=%{public}d", ret);
        CheckAndRepairDbStore(ret);
        return DB_FAILED;
    }

    dbStore_ = dbStore;
    HILOG_INFO(LOG_CORE, "create db store successfully");
    return DB_SUCC;
}

bool AppEventStore::InitDbStoreDir()
{
    std::string dir = HiAppEventConfig::GetInstance().GetStorageDir();
    if (dir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to init db dir, path is empty");
        return false;
    }
    dirPath_ = FileUtil::GetFilePathByDir(dir, DATABASE_DIR);
    if (!FileUtil::IsFileExists(dirPath_) && !FileUtil::ForceCreateDirectory(dirPath_)) {
        HILOG_ERROR(LOG_CORE, "failed to create database dir, errno=%{public}d", errno);
        return false;
    }
    return true;
}

int AppEventStore::ExecuteDbOperation(const std::function<int()>& func)
{
    bool isExecuted = false;
    int OperationRes = ExecuteReadOperation(func, isExecuted);
    if (OperationRes == DB_SUCC) {
        return DB_SUCC;
    }
    return ExecuteWriteOperation(func, isExecuted, OperationRes);
}

int AppEventStore::ExecuteReadOperation(const std::function<int()>& func, bool& isExecuted)
{
    std::shared_lock<std::shared_mutex> lock(dbMutex_);
    if (dbStore_ == nullptr) {
        return DB_FAILED;
    }
    isExecuted = true;
    return func();
}

int AppEventStore::ExecuteWriteOperation(const std::function<int()>& func, const bool& isExecuted, int& OperationRes)
{
    std::unique_lock<std::shared_mutex> lock(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    if (!isExecuted) {  // opt not execute yet
        OperationRes = func();
        if (OperationRes == NativeRdb::E_OK) {
            return DB_SUCC;
        }
    }
    CheckAndRepairDbStore(OperationRes);
    return DB_FAILED;
}

void AppEventStore::CheckAndRepairDbStore(int errCode)
{
    if (errCode != NativeRdb::E_SQLITE_CORRUPT) {
        return;
    }
    dbStore_ = nullptr;
    if (int ret = NativeRdb::RdbHelper::DeleteRdbStore(dirPath_ + DATABASE_NAME); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "errCode=%{public}d failed to delete db file, ret=%{public}d", errCode, ret);
        return;
    }
    HILOG_INFO(LOG_CORE, "errCode=%{public}d delete db file successfully", errCode);
    return;
}

int AppEventStore::DestroyDbStore()
{
    std::unique_lock<std::shared_mutex> lock(dbMutex_);
    if (dbStore_ == nullptr) {
        return DB_SUCC;
    }
    dbStore_ = nullptr;
    if (int ret = NativeRdb::RdbHelper::DeleteRdbStore(dirPath_ + DATABASE_NAME); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to destroy db store, ret=%{public}d", ret);
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "destroy db store successfully");
    return DB_SUCC;
}

int64_t AppEventStore::InsertEvent(std::shared_ptr<AppEventPack> event)
{
    int64_t seq = 0;
    auto func = [this, &event, &seq] () {
        return AppEventDao::Insert(dbStore_, event, seq);
    };
    if (ExecuteDbOperation(func) == DB_FAILED) {
        return DB_FAILED;
    }
    return seq;
}

int64_t AppEventStore::InsertObserver(const Observer& observer)
{
    int64_t seq = 0;
    auto func = [this, &observer, &seq] () {
        return AppEventObserverDao::Insert(dbStore_, observer, seq);
    };
    if (ExecuteDbOperation(func) == DB_FAILED) {
        return DB_FAILED;
    }
    return seq;
}

int AppEventStore::InsertEventMapping(const std::vector<EventObserverInfo>& eventObservers)
{
    auto func = [this, &eventObservers] () {
        return AppEventMappingDao::Insert(dbStore_, eventObservers);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::InsertUserId(const std::string& name, const std::string& value)
{
    auto func = [this, &name, &value] () {
        return UserIdDao::Insert(dbStore_, name, value);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::InsertUserProperty(const std::string& name, const std::string& value)
{
    auto func = [this, &name, &value] () {
        return UserPropertyDao::Insert(dbStore_, name, value);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::InsertCustomEventParams(std::shared_ptr<AppEventPack> event)
{
    std::vector<CustomEventParam> newParams;
    event->GetCustomParams(newParams);
    if (newParams.empty()) {
        return DB_SUCC;
    }
    int errCode = DB_SUCC;
    auto func = [this, &event, &newParams, &errCode] () {
        dbStore_->BeginTransaction();
        std::unordered_set<std::string> oldParamkeys;
        CustomEvent customEvent(event->GetRunningId(), event->GetDomain(), event->GetName());
        CustomEventParamDao::QueryParamkeys(dbStore_, oldParamkeys, customEvent);
        // check params num of same (runningid, domain, name)
        size_t totalNum = oldParamkeys.size();
        for (const auto& param : newParams) {
            if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
                ++totalNum;
            }
        }
        if (totalNum > MAX_NUM_OF_CUSTOM_PARAMS) {
            dbStore_->RollBack();
            errCode = ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM;
            return DB_FAILED;
        }
        std::vector<CustomEventParam> insertParams;
        std::vector<CustomEventParam> updateParams;
        for (const auto& param : newParams) {
            if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
                insertParams.emplace_back(param);
            } else {
                updateParams.emplace_back(param);
            }
        }
        customEvent.params = insertParams;
        if (int ret = CustomEventParamDao::BatchInsert(dbStore_, customEvent); ret != NativeRdb::E_OK) {
            dbStore_->RollBack();
            return ret;
        }
        customEvent.params = updateParams;
        if (int ret = CustomEventParamDao::Updates(dbStore_, customEvent); ret != NativeRdb::E_OK) {
            dbStore_->RollBack();
            return ret;
        }
        dbStore_->Commit();
        return DB_SUCC;
    };
    int res = ExecuteDbOperation(func);
    return errCode != DB_SUCC ? errCode : res;
}

int AppEventStore::UpdateUserId(const std::string& name, const std::string& value)
{
    auto func = [this, &name, &value] () {
        return UserIdDao::Update(dbStore_, name, value);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::UpdateUserProperty(const std::string& name, const std::string& value)
{
    auto func = [this, &name, &value] () {
        return UserPropertyDao::Update(dbStore_, name, value);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::UpdateObserver(int64_t seq, const std::string& filters)
{
    auto func = [this, &seq, &filters] () {
        return AppEventObserverDao::Update(dbStore_, seq, filters);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteUserId(const std::string& name)
{
    auto func = [this, &name] () {
        return UserIdDao::Delete(dbStore_, name);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteUserProperty(const std::string& name)
{
    auto func = [this, &name] () {
        return UserPropertyDao::Delete(dbStore_, name);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryUserIds(std::unordered_map<std::string, std::string>& out)
{
    auto func = [this, &out] () {
        return UserIdDao::QueryAll(dbStore_, out);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryUserId(const std::string& name, std::string& out)
{
    auto func = [this, &name, &out] () {
        return UserIdDao::Query(dbStore_, name, out);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryUserProperties(std::unordered_map<std::string, std::string>& out)
{
    auto func = [this, &out] () {
        return UserPropertyDao::QueryAll(dbStore_, out);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryUserProperty(const std::string& name, std::string& out)
{
    auto func = [this, &name, &out] () {
        return UserPropertyDao::Query(dbStore_, name, out);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::TakeEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t size)
{
    // query the events of the observer
    if (int ret = QueryEvents(events, observerSeq, size); ret != DB_SUCC) {
        return ret;
    }
    if (events.empty()) {
        return DB_SUCC;
    }
    // delete the events mapping of the observer
    std::vector<int64_t> eventSeqs;
    for (const auto &event : events) {
        eventSeqs.emplace_back(event->GetSeq());
    }

    auto func = [this, &observerSeq, &eventSeqs] () {
        int ret = AppEventMappingDao::Delete(dbStore_, observerSeq, eventSeqs);
        if (ret != NativeRdb::E_OK) {
            HILOG_WARN(LOG_CORE, "failed to delete the events mapping data ret=%{public}d, observer=%{public}" PRId64,
                ret, observerSeq);
        }
        return ret;
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t size)
{
    auto func = [this, &events, &observerSeq, &size] () {
        std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
        std::string sql = "SELECT " + Events::TABLE + ".* FROM " + AppEventMapping::TABLE + " INNER JOIN "
            + Events::TABLE + " ON " + AppEventMapping::TABLE + "." + AppEventMapping::FIELD_EVENT_SEQ + "="
            + Events::TABLE + "." + Events::FIELD_SEQ + " WHERE " + AppEventMapping::FIELD_OBSERVER_SEQ + "=?"
            + " ORDER BY " + AppEventMapping::TABLE + "." + AppEventMapping::FIELD_EVENT_SEQ + " DESC ";
        if (size > 0) {
            sql += " LIMIT " + std::to_string(size);
        }
        resultSet = dbStore_->QuerySql(sql, std::vector<std::string>{std::to_string(observerSeq)});
        if (resultSet == nullptr) {
            HILOG_WARN(LOG_CORE, "result set is null, observer=%{public}" PRId64, observerSeq);
            return DB_FAILED;
        }
        int ret = resultSet->GoToNextRow();
        while (ret == NativeRdb::E_OK) {
            auto event = GetEventFromResultSet(resultSet);
            // query custom event params, and add to AppEventPack
            std::unordered_map<std::string, std::string> params;
            // event name is not mandatory, query custom event params with event name is "" first
            CustomEventParamDao::Query(dbStore_, params, CustomEvent(event->GetRunningId(), event->GetDomain(), ""));
            CustomEventParamDao::Query(dbStore_, params,
                CustomEvent(event->GetRunningId(), event->GetDomain(), event->GetName()));
            event->AddCustomParams(params);
            events.emplace_back(event);
            ret = resultSet->GoToNextRow();
        }
        resultSet->Close();
        if (ret == NativeRdb::E_SQLITE_CORRUPT) {
            return ret;
        }
        return DB_SUCC;
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryCustomParamsAdd2EventPack(std::shared_ptr<AppEventPack> event)
{
    auto func = [this, &event] () {
        std::unordered_map<std::string, std::string> params;
        // event name is not mandatory, query custom event params with event name is "" first
        CustomEventParamDao::Query(dbStore_, params, CustomEvent(event->GetRunningId(), event->GetDomain(), ""));
        CustomEventParamDao::Query(dbStore_, params,
            CustomEvent(event->GetRunningId(), event->GetDomain(), event->GetName()));
        event->AddCustomParams(params);
        return DB_SUCC;
    };
    return ExecuteDbOperation(func);
}

int64_t AppEventStore::QueryObserverSeq(const std::string& name, int64_t hashCode)
{
    std::string filters;
    return QueryObserverSeqAndFilters(name, hashCode, filters);
}

int64_t AppEventStore::QueryObserverSeqAndFilters(const std::string& name, int64_t hashCode, std::string& filters)
{
    int64_t seq = 0;
    auto func = [this, &name, &hashCode, &seq, &filters] () {
        return AppEventObserverDao::QuerySeqAndFilters(dbStore_, Observer(name, hashCode), seq, filters);
    };
    if (ExecuteDbOperation(func) == DB_FAILED) {
        return DB_FAILED;
    }
    return seq;
}

int AppEventStore::QueryObserverSeqs(const std::string& name, std::vector<int64_t>& observerSeqs)
{
    auto func = [this, &name, &observerSeqs] () {
        return AppEventObserverDao::QuerySeqs(dbStore_, name, observerSeqs);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::QueryWatchers(std::vector<Observer>& observers)
{
    auto func = [this, &observers] () {
        return AppEventObserverDao::QueryWatchers(dbStore_, observers);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteObserver(int64_t observerSeq)
{
    auto func = [this, &observerSeq] () {
        int retM = AppEventMappingDao::Delete(dbStore_, observerSeq, {});
        if (retM != NativeRdb::E_OK) {
            return retM;
        }
        return AppEventObserverDao::Delete(dbStore_, observerSeq);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteEventMapping(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    auto func = [this, &observerSeq, &eventSeqs] () {
        return AppEventMappingDao::Delete(dbStore_, observerSeq, eventSeqs);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteEvent(int64_t eventSeq)
{
    auto func = [this, &eventSeq] () {
        return AppEventDao::Delete(dbStore_, eventSeq);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteCustomEventParams()
{
    auto func = [this] () {
        return CustomEventParamDao::Delete(dbStore_);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteEvent(const std::vector<int64_t>& eventSeqs)
{
    if (eventSeqs.empty()) {
        return DB_SUCC;
    }
    auto func = [this, &eventSeqs] () {
        std::unordered_set<int64_t> existEventSeqs;
        // query seqs in event_observer_mapping
        int retQuery = AppEventMappingDao::QueryExistEvent(dbStore_, eventSeqs, existEventSeqs);
        if (retQuery != NativeRdb::E_OK) {
            return retQuery;
        }
        // delete events if seqs not in event_observer_mapping
        std::vector<int64_t> delEventSeqs;
        for (const auto& seq : eventSeqs) {
            if (existEventSeqs.find(seq) == existEventSeqs.end()) {
                delEventSeqs.emplace_back(seq);
            }
        }
        return AppEventDao::Delete(dbStore_, delEventSeqs);
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteUnusedParamsExceptCurId(const std::string& curRunningId)
{
    if (curRunningId.empty()) {
        return DB_SUCC;
    }
    auto func = [this, &curRunningId] () {
        int deleteRows = 0;
        // delete custom_event_params if running_id isn't current runningId, and running_id not in events
        std::string whereClause = CustomEventParams::FIELD_RUNNING_ID + " != ?" + " AND ("
            + CustomEventParams::TABLE + "." + CustomEventParams::FIELD_RUNNING_ID + " NOT IN (SELECT "
            + Events::FIELD_RUNNING_ID + " FROM " + Events::TABLE + "))";
        int ret = dbStore_->Delete(deleteRows, CustomEventParams::TABLE, whereClause,
            std::vector<std::string>{curRunningId});
        if (ret != NativeRdb::E_OK) {
            return ret;
        }
        HILOG_INFO(LOG_CORE, "delete %{public}d params unused", deleteRows);
        return DB_SUCC;
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteUnusedEventMapping()
{
    auto func = [this] () {
        int deleteRows = 0;
        // delete event_observer_mapping if event_seq not in events
        std::string whereClause = AppEventMapping::TABLE + "." + AppEventMapping::FIELD_EVENT_SEQ + " NOT IN (SELECT "
            + Events::FIELD_SEQ + " FROM " + Events::TABLE + ")";
        int ret = dbStore_->Delete(deleteRows, AppEventMapping::TABLE, whereClause);
        if (ret != NativeRdb::E_OK) {
            return ret;
        }
        HILOG_INFO(LOG_CORE, "delete %{public}d event map unused", deleteRows);
        return DB_SUCC;
    };
    return ExecuteDbOperation(func);
}

int AppEventStore::DeleteHistoryEvent(int reservedNum, int reservedNumOs)
{
    auto func = [this, &reservedNum, &reservedNumOs] () {
        int deleteRows = 0;
        std::vector<std::string> whereArgs = {
            DOMAIN_OS, std::to_string(reservedNum), DOMAIN_OS, std::to_string(reservedNumOs)
        };
        // delete history events, keep the latest reservedNum events,
        // and keep the latest reservedNumOs events of OS domain
        std::string whereClause
            = Events::FIELD_SEQ + " NOT IN (SELECT " + Events::FIELD_SEQ + " FROM " + Events::TABLE
            + " WHERE " + Events::FIELD_DOMAIN + " != ? ORDER BY "+ Events::FIELD_SEQ + " DESC LIMIT 0,?) AND "
            + Events::FIELD_SEQ + " NOT IN (SELECT " + Events::FIELD_SEQ + " FROM " + Events::TABLE
            + " WHERE " + Events::FIELD_DOMAIN + " = ? ORDER BY " + Events::FIELD_SEQ + " DESC LIMIT 0,?)";
        int ret = dbStore_->Delete(deleteRows, Events::TABLE, whereClause, whereArgs);
        if (ret != NativeRdb::E_OK) {
            return ret;
        }
        HILOG_INFO(LOG_CORE, "delete %{public}d events over limit", deleteRows);
        return DB_SUCC;
    };
    return ExecuteDbOperation(func);
}

bool AppEventStore::DeleteData(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    if (DeleteEventMapping(observerSeq, eventSeqs) < 0) {
        return false;
    }
    if (DeleteEvent(eventSeqs) < 0) {
        HILOG_WARN(LOG_CORE, "failed to delete unused event");
    }
    std::string runningId = HiAppEventConfig::GetInstance().GetRunningId();
    if (!runningId.empty() && DeleteUnusedParamsExceptCurId(runningId) < 0) {
        HILOG_WARN(LOG_CORE, "failed to delete unused params");
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
