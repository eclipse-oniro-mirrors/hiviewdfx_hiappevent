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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_BLOCK_DAO_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_BLOCK_DAO_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class AppEventStore;

class AppEventBlockDao {
public:
    AppEventBlockDao(std::shared_ptr<AppEventStore> store, const std::string& name);
    ~AppEventBlockDao() {}

    // insert package to the table
    int InsertPackage(const std::string& package);

    // delete the data before the seq
    int DeletePackageBySeq(int64_t seq);

    // delete the oldest num pieces of data
    int DeletePackageByNum(int num);

    // the total size of data fetched from the table is less than size
    int GetPackagesBySize(int size, std::vector<std::string> &packages, int64_t& seq);

    // count the total number and size of data in the table
    int CountPackages(int& num, int64_t& size);

private:
    std::shared_ptr<AppEventStore> store_;
    std::string table_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_BLOCK_DAO_H
