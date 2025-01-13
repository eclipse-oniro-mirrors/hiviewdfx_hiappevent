/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

void UpToDbVersion2(NativeRdb::RdbStore& rdbStore)
{
    std::string sql = "ALTER TABLE " + Events::TABLE + " ADD COLUMN "
        + Events::FIELD_RUNNING_ID + " " + SqlUtil::SQL_TEXT_TYPE + " DEFAULT " + "'';";
    if (int ret = rdbStore.ExecuteSql(sql); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to upgrade db version from 1 to 2, ret=%{public}d", ret);
    }
}

void UpToDbVersion3(NativeRdb::RdbStore& rdbStore)
{
    std::string sql = "ALTER TABLE " + Observers::TABLE + " ADD COLUMN "
        + Observers::FIELD_FILTERS + " " + SqlUtil::SQL_TEXT_TYPE + " DEFAULT " + "'';";
    if (int ret = rdbStore.ExecuteSql(sql); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to upgrade db version from 2 to 3, ret=%{public}d", ret);
    }
}
}

int AppEventStoreCallback::OnCreate(NativeRdb::RdbStore& rdbStore)
{
    HILOG_DEBUG(LOG_CORE, "OnCreate start to create db");
    return NativeRdb::E_OK;
}

int AppEventStoreCallback::OnUpgrade(NativeRdb::RdbStore& rdbStore, int oldVersion, int newVersion)
{
    HILOG_DEBUG(LOG_CORE, "OnUpgrade, oldVersion=%{public}d, newVersion=%{public}d", oldVersion, newVersion);
    for (int i = oldVersion; i < newVersion; ++i) {
        if (i == 1) { // upgrade db version from 1 to 2
            UpToDbVersion2(rdbStore);
        }
        if (i == 2) { // upgrade db version from 2 to 3
            UpToDbVersion3(rdbStore);
        }
    }
    return NativeRdb::E_OK;
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
        return DB_FAILED;
    }

    dbStore_ = dbStore;
    appEventDao_ = std::make_shared<AppEventDao>(dbStore_);
    appEventObserverDao_ = std::make_shared<AppEventObserverDao>(dbStore_);
    appEventMappingDao_ = std::make_shared<AppEventMappingDao>(dbStore_);
    userIdDao_ = std::make_shared<UserIdDao>(dbStore_);
    userPropertyDao_ = std::make_shared<UserPropertyDao>(dbStore_);
    customEventParamDao_ = std::make_shared<CustomEventParamDao>(dbStore_);
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

int AppEventStore::DestroyDbStore()
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
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
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventDao_->Insert(event);
}

int64_t AppEventStore::InsertObserver(const std::string& observer, int64_t hashCode, const std::string& filters)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventObserverDao_->Insert(observer, hashCode, filters);
}

int64_t AppEventStore::InsertEventMapping(int64_t eventSeq, int64_t observerSeq)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventMappingDao_->Insert(eventSeq, observerSeq);
}

int64_t AppEventStore::InsertUserId(const std::string& name, const std::string& value)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userIdDao_->Insert(name, value);
}

int64_t AppEventStore::InsertUserProperty(const std::string& name, const std::string& value)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userPropertyDao_->Insert(name, value);
}

int64_t AppEventStore::InsertCustomEventParams(std::shared_ptr<AppEventPack> event)
{
    std::vector<CustomEventParam> newParams;
    event->GetCustomParams(newParams);
    if (newParams.empty()) {
        return DB_SUCC;
    }
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    dbStore_->BeginTransaction();
    std::unordered_set<std::string> oldParamkeys;
    customEventParamDao_->QueryParamkeys(oldParamkeys, event->GetRunningId(), event->GetDomain(), event->GetName());
    // check params num of same (runningid, domain, name)
    size_t totalNum = oldParamkeys.size();
    for (const auto& param : newParams) {
        if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
            ++totalNum;
        }
    }
    if (totalNum > MAX_NUM_OF_CUSTOM_PARAMS) {
        dbStore_->RollBack();
        return ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM;
    }
    std::vector<NativeRdb::ValuesBucket> buckets;
    for (const auto& param : newParams) {
        if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
            if (customEventParamDao_->Insert(param, event->GetRunningId(), event->GetDomain(), event->GetName())
                == DB_FAILED) {
                dbStore_->RollBack();
                return DB_FAILED;
            }
        } else {
            if (customEventParamDao_->Update(param, event->GetRunningId(), event->GetDomain(), event->GetName())
                == DB_FAILED) {
                dbStore_->RollBack();
                return DB_FAILED;
            }
        }
    }
    dbStore_->Commit();
    return DB_SUCC;
}

int64_t AppEventStore::UpdateUserId(const std::string& name, const std::string& value)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userIdDao_->Update(name, value);
}

int64_t AppEventStore::UpdateUserProperty(const std::string& name, const std::string& value)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userPropertyDao_->Update(name, value);
}

int64_t AppEventStore::UpdateObserver(int64_t seq, const std::string& filters)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventObserverDao_->Update(seq, filters);
}

int AppEventStore::DeleteUserId(const std::string& name)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userIdDao_->Delete(name);
}

int AppEventStore::DeleteUserProperty(const std::string& name)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userPropertyDao_->Delete(name);
}

int AppEventStore::QueryUserIds(std::unordered_map<std::string, std::string>& out)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userIdDao_->QueryAll(out);
}

int AppEventStore::QueryUserId(const std::string& name, std::string& out)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userIdDao_->Query(name, out);
}

int AppEventStore::QueryUserProperties(std::unordered_map<std::string, std::string>& out)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userPropertyDao_->QueryAll(out);
}

int AppEventStore::QueryUserProperty(const std::string& name, std::string& out)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return userPropertyDao_->Query(name, out);
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
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    std::vector<int64_t> eventSeqs;
    for (const auto &event : events) {
        eventSeqs.emplace_back(event->GetSeq());
    }
    if (appEventMappingDao_->Delete(observerSeq, eventSeqs) < 0) {
        HILOG_WARN(LOG_CORE, "failed to delete the events mapping data, observer=%{public}" PRId64, observerSeq);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int AppEventStore::QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t size)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

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
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        auto event = GetEventFromResultSet(resultSet);
        // query custom event params, and add to AppEventPack
        std::unordered_map<std::string, std::string> params;
        customEventParamDao_->Query(params, event->GetRunningId(), event->GetDomain());
        customEventParamDao_->Query(params, event->GetRunningId(), event->GetDomain(), event->GetName());
        event->AddCustomParams(params);
        events.emplace_back(event);
    }
    resultSet->Close();
    return DB_SUCC;
}

int AppEventStore::QueryCustomParamsAdd2EventPack(std::shared_ptr<AppEventPack> event)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    std::unordered_map<std::string, std::string> params;
    customEventParamDao_->Query(params, event->GetRunningId(), event->GetDomain());
    customEventParamDao_->Query(params, event->GetRunningId(), event->GetDomain(), event->GetName());
    event->AddCustomParams(params);
    return DB_SUCC;
}

int64_t AppEventStore::QueryObserverSeq(const std::string& observer, int64_t hashCode)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    std::string filters;
    return appEventObserverDao_->QuerySeq(observer, hashCode, filters);
}

int64_t AppEventStore::QueryObserverSeq(const std::string& observer, int64_t hashCode, std::string& filters)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventObserverDao_->QuerySeq(observer, hashCode, filters);
}

int AppEventStore::QueryObserverSeqs(const std::string& observer, std::vector<int64_t>& observerSeqs, ObserverType type)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventObserverDao_->QuerySeqs(observer, observerSeqs, type);
}

int AppEventStore::QueryWatchers(std::vector<Observer>& observers)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventObserverDao_->QueryWatchers(observers);
}

int AppEventStore::DeleteObserver(int64_t observerSeq, ObserverType type)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    if (int ret = appEventMappingDao_->Delete(observerSeq, {}); ret < 0) {
        return ret;
    }
    return appEventObserverDao_->Delete(observerSeq, type);
}

int AppEventStore::DeleteEventMapping(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventMappingDao_->Delete(observerSeq, eventSeqs);
}

int AppEventStore::DeleteEvent(int64_t eventSeq)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return appEventDao_->Delete(eventSeq);
}

int AppEventStore::DeleteCustomEventParams()
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    return customEventParamDao_->Delete();
}

int AppEventStore::DeleteEvent(const std::vector<int64_t>& eventSeqs)
{
    if (eventSeqs.empty()) {
        return DB_SUCC;
    }
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    std::unordered_set<int64_t> existEventSeqs;
    // query seqs in event_observer_mapping
    if (appEventMappingDao_->QueryExistEvent(eventSeqs, existEventSeqs) == DB_FAILED) {
        return DB_FAILED;
    }
    // delete events if seqs not in event_observer_mapping
    if (existEventSeqs.empty()) {
        return appEventDao_->Delete(eventSeqs);
    }
    std::vector<int64_t> delEventSeqs;
    for (const auto& seq : eventSeqs) {
        if (existEventSeqs.find(seq) == existEventSeqs.end()) {
            delEventSeqs.emplace_back(seq);
        }
    }
    return appEventDao_->Delete(delEventSeqs);
}

int AppEventStore::DeleteUnusedParamsExceptCurId(const std::string& curRunningId)
{
    if (curRunningId.empty()) {
        return DB_SUCC;
    }
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    int deleteRows = 0;
    // delete custom_event_params if running_id not in events, and running_id isn't current runningId
    std::string whereClause = "(" + CustomEventParams::TABLE + "." + CustomEventParams::FIELD_RUNNING_ID
        + " NOT IN (SELECT " + Events::FIELD_RUNNING_ID + " FROM " + Events::TABLE + ")) AND "
        + CustomEventParams::FIELD_RUNNING_ID + " != ?";
    if (dbStore_->Delete(deleteRows, CustomEventParams::TABLE, whereClause, std::vector<std::string>{curRunningId})
        != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d params unused", deleteRows);
    return deleteRows;
}

int AppEventStore::DeleteUnusedEventMapping()
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    int deleteRows = 0;
    // delete event_observer_mapping if event_seq not in events
    std::string whereClause = AppEventMapping::TABLE + "." + AppEventMapping::FIELD_EVENT_SEQ + " NOT IN (SELECT "
        + Events::FIELD_SEQ + " FROM " + Events::TABLE + ")";
    if (dbStore_->Delete(deleteRows, AppEventMapping::TABLE, whereClause) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d event map unused", deleteRows);
    return deleteRows;
}

int AppEventStore::DeleteHistoryEvent(int reservedNum, int reservedNumOs)
{
    std::lock_guard<ffrt::mutex> lockGuard(dbMutex_);
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    int deleteRows = 0;
    std::vector<std::string> whereArgs = {
        DOMAIN_OS, std::to_string(reservedNum), DOMAIN_OS, std::to_string(reservedNumOs)
    };
    // delete history events, keep the latest reservedNum events, and keep the latest reservedNumOs events of OS domain
    std::string whereClause
        = Events::FIELD_SEQ + " NOT IN (SELECT " + Events::FIELD_SEQ + " FROM " + Events::TABLE
        + " WHERE " + Events::FIELD_DOMAIN + " != ? ORDER BY "+ Events::FIELD_SEQ + " DESC LIMIT 0,?) AND "
        + Events::FIELD_SEQ + " NOT IN (SELECT " + Events::FIELD_SEQ + " FROM " + Events::TABLE
        + " WHERE " + Events::FIELD_DOMAIN + " = ? ORDER BY " + Events::FIELD_SEQ + " DESC LIMIT 0,?)";
    if (dbStore_->Delete(deleteRows, Events::TABLE, whereClause, whereArgs) != NativeRdb::E_OK) {
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d events over limit", deleteRows);
    return deleteRows;
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
