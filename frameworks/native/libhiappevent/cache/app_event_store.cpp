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
#include "hiappevent_config.h"
#include "hilog/log.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "sql_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventStore"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
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
        HILOG_ERROR(LOG_CORE, "failed to upgrade db version from 1 to 2, ret=%{pulic}d", ret);
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

int AppEventStore::InitDbStore()
{
    if (!InitDbStoreDir()) {
        return DB_FAILED;
    }

    int ret = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dirPath_ + DATABASE_NAME);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    const int dbVersion = 2; // 2 means new db version
    AppEventStoreCallback callback;
    auto dbStore = NativeRdb::RdbHelper::GetRdbStore(config, dbVersion, callback, ret);
    if (ret != NativeRdb::E_OK || dbStore == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to create db store, ret=%{pulic}d", ret);
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
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
    if (dbStore_ == nullptr) {
        return DB_SUCC;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    dbStore_ = nullptr;
    if (int ret = NativeRdb::RdbHelper::DeleteRdbStore(dirPath_ + DATABASE_NAME); ret != NativeRdb::E_OK) {
        HILOG_ERROR(LOG_CORE, "failed to destroy db store, ret=%{pulic}d", ret);
        return DB_FAILED;
    }
    HILOG_INFO(LOG_CORE, "destroy db store successfully");
    return DB_SUCC;
}

int64_t AppEventStore::InsertEvent(std::shared_ptr<AppEventPack> event)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventDao_->Insert(event);
}

int64_t AppEventStore::InsertObserver(const std::string& observer, int64_t hashCode)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventObserverDao_->Insert(observer, hashCode);
}

int64_t AppEventStore::InsertEventMapping(int64_t eventSeq, int64_t observerSeq)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventMappingDao_->Insert(eventSeq, observerSeq);
}

int64_t AppEventStore::InsertUserId(const std::string& name, const std::string& value)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userIdDao_->Insert(name, value);
}

int64_t AppEventStore::InsertUserProperty(const std::string& name, const std::string& value)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userPropertyDao_->Insert(name, value);
}

int64_t AppEventStore::InsertCustomEventParams(std::shared_ptr<AppEventPack> event)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    dbStore_->BeginTransaction();
    std::unordered_set<std::string> oldParamkeys;
    customEventParamDao_->QueryParamkeys(oldParamkeys, event->GetRunningId(), event->GetDomain(), event->GetName());
    std::vector<CustomEventParam> newParams;
    event->GetCustomParams(newParams);
    if (newParams.empty()) {
        return DB_SUCC;
    }
    // check params num of same (runningid, domain, name)
    size_t totalNum = oldParamkeys.size();
    for (const auto& param : newParams) {
        if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
            ++totalNum;
        }
    }
    if (totalNum > MAX_NUM_OF_CUSTOM_PARAMS) {
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
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userIdDao_->Update(name, value);
}

int64_t AppEventStore::UpdateUserProperty(const std::string& name, const std::string& value)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userPropertyDao_->Update(name, value);
}

int AppEventStore::DeleteUserId(const std::string& name)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userIdDao_->Delete(name);
}

int AppEventStore::DeleteUserProperty(const std::string& name)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userPropertyDao_->Delete(name);
}

int AppEventStore::QueryUserIds(std::unordered_map<std::string, std::string>& out)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userIdDao_->QueryAll(out);
}

int AppEventStore::QueryUserId(const std::string& name, std::string& out)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userIdDao_->Query(name, out);
}

int AppEventStore::QueryUserProperties(std::unordered_map<std::string, std::string>& out)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return userPropertyDao_->QueryAll(out);
}

int AppEventStore::QueryUserProperty(const std::string& name, std::string& out)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
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
    std::lock_guard<std::mutex> lockGuard(dbMutex_);
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
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
    {
        std::lock_guard<std::mutex> lockGuard(dbMutex_);
        std::string sql = "SELECT " + Events::TABLE + ".* FROM " + AppEventMapping::TABLE + " INNER JOIN "
            + Events::TABLE + " ON " + AppEventMapping::TABLE + "." + AppEventMapping::FIELD_EVENT_SEQ + "="
            + Events::TABLE + "." + Events::FIELD_SEQ + " WHERE " + AppEventMapping::FIELD_OBSERVER_SEQ + "=?"
            + " ORDER BY " + AppEventMapping::TABLE + "." + AppEventMapping::FIELD_EVENT_SEQ + " DESC ";
        if (size > 0) {
            sql += " LIMIT " + std::to_string(size);
        }
        resultSet = dbStore_->QuerySql(sql, std::vector<std::string>{std::to_string(observerSeq)});
    }
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
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    std::unordered_map<std::string, std::string> params;
    customEventParamDao_->Query(params, event->GetRunningId(), event->GetDomain());
    customEventParamDao_->Query(params, event->GetRunningId(), event->GetDomain(), event->GetName());
    event->AddCustomParams(params);
    return DB_SUCC;
}

int64_t AppEventStore::QueryObserverSeq(const std::string& observer, int64_t hashCode)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventObserverDao_->QuerySeq(observer, hashCode);
}

int AppEventStore::QueryObserverSeqs(const std::string& observer, std::vector<int64_t>& observerSeqs)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventObserverDao_->QuerySeqs(observer, observerSeqs);
}

int AppEventStore::DeleteObserver(int64_t observerSeq)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    if (int ret = appEventMappingDao_->Delete(observerSeq, {}); ret < 0) {
        return ret;
    }
    return appEventObserverDao_->Delete(observerSeq);
}

int AppEventStore::DeleteEventMapping(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventMappingDao_->Delete(observerSeq, eventSeqs);
}

int AppEventStore::DeleteEvent(int64_t eventSeq)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return appEventDao_->Delete(eventSeq);
}

int AppEventStore::DeleteCustomEventParams(const std::string& runningId)
{
    if (dbStore_ == nullptr && InitDbStore() != DB_SUCC) {
        return DB_FAILED;
    }
    std::lock_guard<std::mutex> lockGuard(dbMutex_);
    return customEventParamDao_->Delete(runningId);
}
} // namespace HiviewDFX
} // namespace OHOS
