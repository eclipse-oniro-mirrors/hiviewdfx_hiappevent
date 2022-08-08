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
#include "app_event_blocks_dao.h"

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
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_BlocksDao" };
}
AppEventBlocksDao::AppEventBlocksDao(std::shared_ptr<AppEventStore> store) : store_(store)
{
    table_ = Blocks::TABLE;
}

bool AppEventBlocksDao::IsBlockExists(const std::string& name)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        return false;
    }

    std::string sql;
    sql.append("SELECT COUNT(*) FROM ").append(table_)
        .append(" WHERE ").append(Blocks::FIELD_NAME).append(" = ?");
    int64_t count = 0;
    std::vector<NativeRdb::ValueObject> objects = { NativeRdb::ValueObject(name) };
    if (int ret = dbStore->ExecuteAndGetLong(count, sql, objects); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to query block %{public}s", name.c_str());
        return false;
    }
    return count != 0;
}

int AppEventBlocksDao::AddBlock(const std::string& name)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        return DB_FAILED;
    }

    HiLog::Debug(LABEL, "start to add block %{public}s", name.c_str());
    NativeRdb::ValuesBucket values;
    values.PutString(Blocks::FIELD_NAME, name);
    int64_t seq = 0;
    if (int ret = dbStore->Insert(seq, table_, values); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to add block %{public}s, ret=%{public}d", name.c_str(), ret);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int AppEventBlocksDao::RemoveBlock(const std::string& name)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        return DB_FAILED;
    }

    HiLog::Debug(LABEL, "start to remove block %{public}s", name.c_str());
    std::string cond;
    cond += Blocks::FIELD_NAME;
    cond += " = ?";
    int delRow = 0;
    std::vector<std::string> fields = { name };
    if (int ret = dbStore->Delete(delRow, table_, cond, fields); ret != NativeRdb::E_OK) {
        HiLog::Error(LABEL, "failed to remove block %{public}s, ret=%{public}d", name.c_str(), ret);
        return DB_FAILED;
    }
    return DB_SUCC;
}

int AppEventBlocksDao::GetBlocks(std::vector<std::string>& names)
{
    auto dbStore = store_->GetDbStore();
    if (dbStore == nullptr) {
        return DB_FAILED;
    }

    std::string sql;
    sql.append("SELECT ").append(Blocks::FIELD_NAME).append(" FROM ").append(table_);
    auto resultSet = dbStore->QuerySql(sql, std::vector<std::string> {});
    if (resultSet == nullptr) {
        HiLog::Error(LABEL, "failed to get blocks");
        return DB_FAILED;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string name;
        resultSet->GetString(0, name); // 0 means blockName field
        names.emplace_back(name);
    }
    resultSet->Close();
    return DB_SUCC;
}
} // namespace HiviewDFX
} // namespace OHOS
