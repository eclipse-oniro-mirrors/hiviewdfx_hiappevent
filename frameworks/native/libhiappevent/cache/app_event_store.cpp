/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <utility>
#include <vector>

#include "app_event_cache_common.h"
#include "app_event_store_callback.h"
#include "file_util.h"
#include "hilog/log.h"
#include "rdb_errno.h"
#include "rdb_helper.h"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_Store" };
const char* DATABASE_NAME = "appevent.db";
const char* DATABASE_DIR = "databases";
const char* SQL_TEXT_TYPE = "TEXT NOT NULL";
const char* SQL_INT_TYPE = "INTEGER";
const char PATH_DELIMITER = '/';

std::string GenerateCreateSql(const std::string& table, std::vector<std::pair<std::string, std::string>> fields)
{
    std::string sql;
    sql += "CREATE TABLE IF NOT EXISTS ";
    sql += table;
    sql += "(";
    sql += "seq INTEGER PRIMARY KEY AUTOINCREMENT"; // default field: seq
    for (auto field : fields) {
        sql += ", ";
        sql += field.first;
        sql += " ";
        sql += field.second;
    }
    sql += ")";
    return sql;
}

std::string GenerateDropSql(const std::string& table)
{
    std::string sql;
    sql += "DROP TABLE IF EXISTS ";
    sql += table;
    return sql;
}
}
AppEventStore::AppEventStore(const std::string& dir) : dbStore_(nullptr), dirPath_("")
{
    InitDbStoreDir(dir);
}

int AppEventStoreCallback::OnCreate(NativeRdb::RdbStore& rdbStore)
{
    std::vector<std::pair<std::string, std::string>> fields = {
        {Blocks::FIELD_NAME, SQL_TEXT_TYPE}
    };
    std::string sql = GenerateCreateSql(Blocks::TABLE, fields);
    if (int ret = rdbStore.ExecuteSql(sql); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to create blocks table, ret=%{public}d", ret);
        return ret;
    }
    return NativeRdb::E_OK;
}

int AppEventStoreCallback::OnUpgrade(NativeRdb::RdbStore& rdbStore, int oldVersion, int newVersion)
{
    HiLog::Debug(LABEL, "OnUpgrade, oldVersion=%{public}d, newVersion=%{public}d", oldVersion, newVersion);
    return NativeRdb::E_OK;
}

std::shared_ptr<NativeRdb::RdbStore> AppEventStore::GetDbStore()
{
    if (dbStore_ == nullptr) {
        std::lock_guard<std::mutex> lockGuard(dbMutex_);
        if (dbStore_ == nullptr) {
            dbStore_ = CreateDbStore();
        }
    }
    return dbStore_;
}

void AppEventStore::InitDbStoreDir(const std::string& dir)
{
    if (dir.empty()) {
        HiLog::Error(LABEL, "failed to init db dir, path is empty");
        return;
    }
    dirPath_ = FileUtil::GetFilePathByDir(dir, DATABASE_DIR);
    dirPath_.push_back(PATH_DELIMITER);
}

int AppEventStore::CreateBlockTable(const std::string& name)
{
    auto dbStore = GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to create table %{public}s, db is null", name.c_str());
        return DB_FAILED;
    }

    std::vector<std::pair<std::string, std::string>> fields = {
        {Block::FIELD_PACKAGE, SQL_TEXT_TYPE},
        {Block::FIELD_SIZE, SQL_INT_TYPE}
    };
    std::string sql = GenerateCreateSql(Block::TABLE_PREFIX + name, fields);
    if (int ret = dbStore->ExecuteSql(sql); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to create table %{public}s, ret=%{pulic}d", name.c_str(), ret);
        return DB_FAILED;
    }
    HiLog::Info(LABEL, "create table %{public}s successfully", name.c_str());
    return DB_SUCC;
}

int AppEventStore::DropBlockTable(const std::string& name)
{
    auto dbStore = GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to drop table %{public}s, db is null", name.c_str());
        return DB_FAILED;
    }

    std::string sql = GenerateDropSql(Block::TABLE_PREFIX + name);
    if (int ret = dbStore->ExecuteSql(sql); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to drop table %{public}s, ret=%{pulic}d", name.c_str(), ret);
        return DB_FAILED;
    }
    HiLog::Info(LABEL, "drop table %{public}s successfully", name.c_str());
    return DB_SUCC;
}

std::shared_ptr<NativeRdb::RdbStore> AppEventStore::CreateDbStore()
{
    if (dirPath_.empty()) {
        HiLog::Error(LABEL, "failed to create db store, path is empty");
        return nullptr;
    }
    if (!FileUtil::IsFileExists(dirPath_) && !FileUtil::ForceCreateDirectory(dirPath_)) {
        HiLog::Error(LABEL, "failed to create database dir, errno=%{public}d", errno);
        return nullptr;
    }

    int ret = NativeRdb::E_OK;
    NativeRdb::RdbStoreConfig config(dirPath_ + DATABASE_NAME);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    AppEventStoreCallback callback;
    auto dbStore = NativeRdb::RdbHelper::GetRdbStore(config, 1, callback, ret);
    if (ret != NativeRdb::E_OK || dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to create db store, ret=%{pulic}d", ret);
        return nullptr;
    }
    HiLog::Info(LABEL, "create db store successfully");
    return dbStore;
}

int AppEventStore::DestroyDbStore()
{
    if (dbStore_ == nullptr) {
        return DB_SUCC;
    }

    dbStore_ = nullptr;
    if (int ret = NativeRdb::RdbHelper::DeleteRdbStore(dirPath_ + DATABASE_NAME); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to destroy db store, ret=%{pulic}d", ret);
        return DB_FAILED;
    }
    HiLog::Info(LABEL, "destroy db store successfully");
    return DB_SUCC;
}
} // namespace HiviewDFX
} // namespace OHOS
