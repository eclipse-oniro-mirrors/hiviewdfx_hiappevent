/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_CUSTOM_EVENT_PARAM_DAO_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_CUSTOM_EVENT_PARAM_DAO_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "app_event_cache_common.h"
#include "hiappevent_base.h"
#include "rdb_store.h"

namespace OHOS {
namespace HiviewDFX {
namespace CustomEventParamDao {
int Create(NativeRdb::RdbStore& dbStore);
int BatchInsert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const AppEventCacheCommon::CustomEvent& customEvent);
int Updates(std::shared_ptr<NativeRdb::RdbStore> dbStore, const AppEventCacheCommon::CustomEvent& customEvent);
int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore);
int Query(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::unordered_map<std::string, std::string>& params,
    const AppEventCacheCommon::CustomEvent& customEvent);
int QueryParamkeys(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::unordered_set<std::string>& out,
    const AppEventCacheCommon::CustomEvent& customEvent);
} // namespace CustomEventParamDao
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_CUSTOM_EVENT_PARAM_DAO_H
