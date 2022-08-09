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
#include "app_event_block.h"

#include "app_event_cache_common.h"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
namespace OHOS {
namespace HiviewDFX {
AppEventBlock::AppEventBlock(std::shared_ptr<AppEventStore> store, const std::string& name)
{
    blockDao_ = std::make_shared<AppEventBlockDao>(store, name);
}

int AppEventBlock::GetStat(std::pair<int, int64_t>& statPair)
{
    return blockDao_->CountPackages(statPair.first, statPair.second);
}

int AppEventBlock::Add(const std::string& package)
{
    return blockDao_->InsertPackage(package);
}

int AppEventBlock::Remove(int num)
{
    return blockDao_->DeletePackageByNum(num);
}

int AppEventBlock::Take(int size, std::vector<std::string>& packages)
{
    int64_t seq = 0;
    int result = blockDao_->GetPackagesBySize(size, packages, seq);
    if (result == 0) {
        return DB_SUCC;
    } else if (result > 0 && blockDao_->DeletePackageBySeq(seq) == DB_SUCC) {
        return result;
    }
    return DB_FAILED;
}
} // namespace HiviewDFX
} // namespace OHOS
