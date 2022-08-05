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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H

#include <memory>
#include <mutex>
#include <string>

#include "rdb_store.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventStore {
public:
    AppEventStore(const std::string& dir);
    ~AppEventStore() {}
    std::shared_ptr<NativeRdb::RdbStore> GetDbStore();
    int CreateBlockTable(const std::string& name);
    int DropBlockTable(const std::string& name);
    int DestroyDbStore();

private:
    void InitDbStoreDir(const std::string& dir);
    std::shared_ptr<NativeRdb::RdbStore> CreateDbStore();

private:
    std::shared_ptr<NativeRdb::RdbStore> dbStore_;
    std::string dirPath_;
    std::mutex dbMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H
