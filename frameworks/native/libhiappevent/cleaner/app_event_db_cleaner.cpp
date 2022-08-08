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
#include "app_event_db_cleaner.h"

#include "app_event_cache.h"
#include "file_util.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, 0xD002D07, "HiAppEvent_DbCleaner" };
const std::string DATABASE_DIR = "databases/";
}
uint64_t AppEventDbCleaner::GetFilesSize()
{
    return FileUtil::GetDirSize(path_ + DATABASE_DIR);
}

uint64_t AppEventDbCleaner::ClearSpace(uint64_t curSize, uint64_t maxSize)
{
    HiLog::Info(LABEL, "start to clear the space occupied by database files");
    if (curSize <= maxSize) {
        return curSize;
    }
    std::map<std::string, std::pair<int, int64_t>> blocksStat;
    int result = AppEventCache::GetInstance()->GetBlocksStat(blocksStat);
    if (result != 0 || blocksStat.empty()) {
        return curSize;
    }

    uint64_t nowSize = curSize;
    uint64_t otherSize = curSize - GetFilesSize();
    const int TIMES = 10; // remove 1/10 records each time
    for (int i = 0; i < TIMES; ++i) {
        for (auto it = blocksStat.begin(); it != blocksStat.end();) {
            std::pair<int, int64_t>& statPair = it->second;
            if (statPair.first == 0) {
                it = blocksStat.erase(it);
                continue;
            }
            int delNum = statPair.first / TIMES;
            if (delNum == 0) {
                // data in table less than 10 records will remove at one time
                delNum = statPair.first;
                // mark as 0, will remove from map at next loop
                statPair.first = 0;
            }

            // delete the records in the block
            auto block = AppEventCache::GetInstance()->GetBlock(it->first);
            if (block->Remove(delNum) != 0) {
                ++it;
                HiLog::Error(LABEL, "failed to delete record in block=%{public}s", it->first.c_str());
                continue;
            }

            // update the now size of db file
            nowSize = otherSize + GetFilesSize();
            if (nowSize <= maxSize) {
                return nowSize;
            }
            ++it;
        }
    }
    return nowSize;
}

void AppEventDbCleaner::ClearData()
{
    HiLog::Info(LABEL, "start to clear the db data");
    if (AppEventCache::GetInstance()->Close() != 0) {
        HiLog::Error(LABEL, "failed to clear the db data");
    }
}
} // namespace HiviewDFX
} // namespace OHOS
