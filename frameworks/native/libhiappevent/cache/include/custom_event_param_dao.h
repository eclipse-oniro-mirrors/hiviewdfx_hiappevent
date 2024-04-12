/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hiappevent_base.h"
#include "rdb_store.h"

namespace OHOS {
namespace HiviewDFX {
class CustomEventParamDao {
public:
    CustomEventParamDao(std::shared_ptr<NativeRdb::RdbStore> dbStore);
    ~CustomEventParamDao() = default;
    int64_t Insert(const CustomEventParam& param,
        const std::string& runningId, const std::string& domain, const std::string& name);
    int64_t Update(const CustomEventParam& param,
        const std::string& runningId, const std::string& domain, const std::string& name);
    int Delete(const std::string& runningId);
    int Query(std::unordered_map<std::string, std::string>& params,
        const std::string& runningId, const std::string& domain, const std::string& name = "");
    int QueryParamkeys(std::unordered_set<std::string>& out,
        const std::string& runningId, const std::string& domain, const std::string& name);

private:
    int Create();

private:
    std::shared_ptr<NativeRdb::RdbStore> dbStore_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_CUSTOM_EVENT_PARAM_DAO_H
