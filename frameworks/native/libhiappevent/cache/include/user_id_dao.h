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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_USER_ID_DAO_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_USER_ID_DAO_H

#include <memory>
#include <string>
#include <unordered_map>

#include "rdb_store.h"

namespace OHOS {
namespace HiviewDFX {
namespace UserIdDao {
int Create(NativeRdb::RdbStore& dbStore);
int Insert(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name, const std::string& value);
int Update(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name, const std::string& value);
int Delete(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name);
int Query(std::shared_ptr<NativeRdb::RdbStore> dbStore, const std::string& name, std::string& out);
int QueryAll(std::shared_ptr<NativeRdb::RdbStore> dbStore, std::unordered_map<std::string, std::string>& out);
} // namespace UserIdDao
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_USER_ID_DAO_H
