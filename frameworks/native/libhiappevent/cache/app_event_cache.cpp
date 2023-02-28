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
#include "app_event_cache.h"

#include "app_event_blocks_dao.h"
#include "app_event_cache_common.h"
#include "app_event_store.h"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
namespace OHOS {
namespace HiviewDFX {
std::mutex AppEventCache::cacheMutex_;
std::shared_ptr<AppEventCache> AppEventCache::instance_ = nullptr;

std::shared_ptr<AppEventCache> AppEventCache::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lockGuard(cacheMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppEventCache>();
        }
    }
    return instance_;
}

bool AppEventCache::IsOpen()
{
    return store_ != nullptr;
}

int AppEventCache::Open(const std::string& dir)
{
    if (dir.empty()) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(cacheMutex_);
    if (store_ != nullptr) {
        store_.reset();
    }
    store_ = std::make_shared<AppEventStore>(dir);
    return DB_SUCC;
}

int AppEventCache::Close()
{
    if (store_ == nullptr) {
        return DB_SUCC;
    }

    std::lock_guard<std::mutex> lockGuard(cacheMutex_);
    blocks_.clear(); // delete blocks
    int ret = store_->DestroyDbStore(); // delete db
    store_.reset();
    return ret;
}

int AppEventCache::CreateBlock(const std::string& name)
{
    if (store_ == nullptr) {
        return DB_FAILED;
    }
    if (blocks_.find(name) != blocks_.end()) {
        return DB_SUCC;
    }
    std::lock_guard<std::mutex> lockGuard(cacheMutex_);

    // add a record to blocks table and create a block table
    if (AppEventBlocksDao(store_).AddBlock(name) != DB_SUCC || store_->CreateBlockTable(name) != DB_SUCC) {
        return DB_FAILED;
    }
    blocks_[name] = std::make_shared<AppEventBlock>(store_, name);
    return DB_SUCC;
}

int AppEventCache::DestroyBlock(const std::string& name)
{
    if (store_ == nullptr) {
        return DB_FAILED;
    }
    if (blocks_.find(name) == blocks_.end()) {
        return DB_SUCC;
    }
    std::lock_guard<std::mutex> lockGuard(cacheMutex_);

    // delete the record form blocks table and delete the block table
    if (AppEventBlocksDao(store_).RemoveBlock(name) != DB_SUCC || store_->DropBlockTable(name) != DB_SUCC) {
        return DB_FAILED;
    }
    blocks_.erase(name);
    return DB_SUCC;
}

std::shared_ptr<AppEventBlock> AppEventCache::GetBlock(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(cacheMutex_);
    return blocks_.find(name) == blocks_.end() ? nullptr : blocks_[name];
}

int AppEventCache::GetBlocksStat(std::map<std::string, std::pair<int, int64_t>>& blocksStat)
{
    if (store_ == nullptr) {
        return DB_FAILED;
    }

    std::lock_guard<std::mutex> lockGuard(cacheMutex_);
    int result = DB_SUCC;
    for (auto& block : blocks_) {
        std::pair<int, int64_t> statPair;
        if (block.second->GetStat(statPair) != DB_SUCC) {
            result = DB_FAILED;
            continue;
        }
        blocksStat.emplace(block.first, statPair);
    }
    return result;
}
} // namespace HiviewDFX
} // namespace OHOS
