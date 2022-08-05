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
#include "app_event_block_dao.h"

#include <cinttypes>

#include "app_event_cache_common.h"
#include "app_event_store.h"
#include "hilog/log.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "value_object.h"
#include "values_bucket.h"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_BlockDao" };
}
AppEventBlockDao::AppEventBlockDao(std::shared_ptr<AppEventStore> store, const std::string& name)
    : store_(store)
{
    table_ = Block::TABLE_PREFIX + name;
}

int AppEventBlockDao::InsertPackage(const std::string& package)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to insert %{public}s, db is null", table_.c_str());
        return DB_FAILED;
    }

    HiLog::Debug(LABEL, "start to insert %{public}s, size=%{public}zu", table_.c_str(), package.size());
    NativeRdb::ValuesBucket values;
    values.PutString(Block::FIELD_PACKAGE, package);
    values.PutInt(Block::FIELD_SIZE, package.size());
    int64_t seq = 0;
    if (int ret = dbStore->Insert(seq, table_, values); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to insert %{public}s, ret=%{public}d", table_.c_str(), ret);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int AppEventBlockDao::DeletePackageBySeq(int64_t seq)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to delete %{public}s by seq, db is null", table_.c_str());
        return DB_FAILED;
    }

    HiLog::Debug(LABEL, "start to delete %{public}s, seq=%{public}" PRId64, table_.c_str(), seq);
    std::string cond;
    cond += Block::FIELD_SEQ;
    cond += " <= ?";
    int delRow = 0;
    std::vector<std::string> fields = { std::to_string(seq) };
    if (int ret = dbStore->Delete(delRow, table_, cond, fields); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to delete %{public}s by seq, ret=%{public}d", table_.c_str(), ret);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int AppEventBlockDao::DeletePackageByNum(int num)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to delete %{public}s by num, db is null", table_.c_str());
        return DB_FAILED;
    }

    HiLog::Debug(LABEL, "start to delete %{public}s, num=%{public}d", table_.c_str(), num);
    std::string sql;
    sql.append("DELETE FROM ").append(table_)
        .append(" WHERE rowid IN(SELECT rowid FROM ").append(table_)
        .append(" ORDER BY seq LIMIT ").append(std::to_string(num)).append(")");
    std::vector<NativeRdb::ValueObject> objects;
    if (int ret = dbStore->ExecuteSql(sql, objects); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to delete %{public}s by num, ret=%{public}d", table_.c_str(), ret);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int AppEventBlockDao::GetPackagesBySize(int size, std::vector<std::string> &packages, int64_t& seq)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to query %{public}s, db is null", table_.c_str());
        return DB_FAILED;
    }

    HiLog::Debug(LABEL, "start to query %{public}s, size=%{public}d", table_.c_str(), size);
    std::string sql;
    sql.append("SELECT ")
        .append(Block::FIELD_SEQ).append(",").append(Block::FIELD_PACKAGE).append(",").append(Block::FIELD_SIZE)
        .append(" FROM ").append(table_)
        .append(" a WHERE (SELECT SUM(").append(Block::FIELD_SIZE).append(") FROM ").append(table_)
        .append(" b WHERE b.").append(Block::FIELD_SEQ).append(" <= a.").append(Block::FIELD_SEQ).append(") <= ")
        .append(std::to_string(size)).append(" ORDER BY seq");
    auto resultSet = dbStore->QuerySql(sql, std::vector<std::string> {});
    if (resultSet == nullptr) {
        HiLog::Error(LABEL, "failed to query %{public}s", table_.c_str());
        return DB_FAILED;
    }

    int totalSize = 0;
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetLong(0, seq); // 0 means seq field

        std::string package;
        resultSet->GetString(1, package); // 1 means package field
        packages.emplace_back(package);

        int packageSize = 0;
        resultSet->GetInt(2, packageSize); // 2 means packageSize field
        totalSize += packageSize;
    }
    resultSet->Close();
    return totalSize;
}

int AppEventBlockDao::CountPackages(int& num, int64_t& size)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        HiLog::Error(LABEL, "failed to count %{public}s, db is null", table_.c_str());
        return DB_FAILED;
    }

    std::string sql;
    sql.append("SELECT count(*), sum(").append(Block::FIELD_SIZE).append(") FROM ").append(table_);
    auto resultSet = dbStore->QuerySql(sql, std::vector<std::string> {});
    if (resultSet == nullptr) {
        HiLog::Error(LABEL, "failed to count %{public}s, resultSet is null", table_.c_str());
        return DB_FAILED;
    }
    if (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        resultSet->GetInt(0, num); // 0 means totalNum field
        resultSet->GetLong(1, size); // 1 means totalSize field
    }
    resultSet->Close();
    return DB_SUCC;
}
} // namespace HiviewDFX
} // namespace OHOS
