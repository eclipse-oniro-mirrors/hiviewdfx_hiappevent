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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_MAPPING_DAO_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_MAPPING_DAO_H

#include <memory>
#include <string>
#include <unordered_set>

#include "rdb_store.h"

namespace OHOS {
namespace HiviewDFX {
namespace AppEventMappingDao {
int Create(NativeRdb::RdbStore& dbStore);
int Insert(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t eventSeq, int64_t observerSeq);
int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, int64_t observerSeq, const std::vector<int64_t>& eventSeqs);
int QueryExistEvent(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::vector<int64_t>& eventSeqs,
    std::unordered_set<int64_t>& existEventSeqs);
} // namespace AppEventMappingDao
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_MAPPING_DAO_H
