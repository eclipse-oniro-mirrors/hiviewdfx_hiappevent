/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_OBSERVER_DAO_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_OBSERVER_DAO_H

#include <memory>
#include <string>
#include <vector>

#include "app_event_cache_common.h"
#include "rdb_store.h"

namespace OHOS {
namespace HiviewDFX {
namespace AppEventObserverDao {
int Create(NativeRdb::RdbStore& dbStore);
int Insert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const AppEventCacheCommon::Observer& observer, int64_t& seq);
int Update(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t seq, const std::string& filters);
int QuerySeqAndFilters(std::shared_ptr<NativeRdb::RdbStore> dbStore, const AppEventCacheCommon::Observer& observer,
    int64_t& seq, std::string& filters);
int QuerySeqs(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name,
    std::vector<int64_t>& observerSeqs);
int QueryWatchers(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::vector<AppEventCacheCommon::Observer>& observers);
int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name);
int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t observerSeq);
} // namespace AppEventObserverDao
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_OBSERVER_DAO_H
