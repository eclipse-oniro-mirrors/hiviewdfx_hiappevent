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

#include "hiappevent_clean.h"

#include <memory>
#include <vector>

#include "app_event_db_cleaner.h"
#include "app_event_log_cleaner.h"
#include "hiappevent_base.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEventClean {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_Clean" };

void CreateCleaners(const std::string& dir, std::vector<std::shared_ptr<AppEventCleaner>>& cleaners)
{
    cleaners.push_back(std::make_shared<AppEventDbCleaner>(dir));
    cleaners.push_back(std::make_shared<AppEventLogCleaner>(dir));
}

uint64_t GetCurStorageSize(const std::string& dir)
{
    std::vector<std::shared_ptr<AppEventCleaner>> cleaners;
    CreateCleaners(dir, cleaners);
    uint64_t curSize = 0;
    for (auto& cleaner : cleaners) {
        curSize += cleaner->GetFilesSize();
    }
    return curSize;
}
}
bool IsStorageSpaceFull(const std::string& dir, uint64_t maxSize)
{
    return GetCurStorageSize(dir) > maxSize;
}

bool ReleaseSomeStorageSpace(const std::string& dir, uint64_t maxSize)
{
    HiLog::Info(LABEL, "start to clear the storage space");
    std::vector<std::shared_ptr<AppEventCleaner>> cleaners;
    CreateCleaners(dir, cleaners);
    auto curSize = GetCurStorageSize(dir);
    for (auto it = cleaners.rbegin(); it != cleaners.rend(); ++it) { // clear the log space first
        curSize = (*it)->ClearSpace(curSize, maxSize);
        if (curSize <= maxSize) {
            return true;
        }
    }
    return curSize <= maxSize;
}

void ClearData(const std::string& dir)
{
    std::vector<std::shared_ptr<AppEventCleaner>> cleaners;
    CreateCleaners(dir, cleaners);
    for (auto& cleaner : cleaners) { // clear the db data first
        cleaner->ClearData();
    }
}
} // namespace HiAppEventClean
} // namespace HiviewDFX
} // namespace OHOS
