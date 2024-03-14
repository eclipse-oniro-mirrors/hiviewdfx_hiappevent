/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "app_event_store.h"
#include "file_util.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventDbCleaner"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DATABASE_DIR = "databases/";

void ClearAllData()
{
    if (AppEventStore::GetInstance().DeleteEvent() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear event table");
    }
    if (AppEventStore::GetInstance().DeleteEventMapping() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear event mapping table");
    }
    if (AppEventStore::GetInstance().DeleteUserId() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear user id table");
    }
    if (AppEventStore::GetInstance().DeleteUserProperty() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear user id table");
    }
}
}
uint64_t AppEventDbCleaner::GetFilesSize()
{
    return FileUtil::GetDirSize(path_ + DATABASE_DIR);
}

uint64_t AppEventDbCleaner::ClearSpace(uint64_t curSize, uint64_t maxSize)
{
    HILOG_INFO(LOG_CORE, "start to clear the space occupied by database files");
    if (curSize <= maxSize) {
        return curSize;
    }
    ClearAllData();
    return 0;
}

void AppEventDbCleaner::ClearData()
{
    HILOG_INFO(LOG_CORE, "start to clear the db data");
    ClearAllData();
}
} // namespace HiviewDFX
} // namespace OHOS
