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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_CACHE_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_CACHE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "app_event_block.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventStore;

class AppEventCache {
public:
    static std::shared_ptr<AppEventCache> GetInstance();
    bool IsOpen();
    int Open(const std::string& dir);
    int Close();
    int CreateBlock(const std::string& name);
    int DestroyBlock(const std::string& name);
    std::shared_ptr<AppEventBlock> GetBlock(const std::string& name);
    int GetBlocksStat(std::map<std::string, std::pair<int, int64_t>>& blocksStat);

private:
    static std::mutex cacheMutex_;
    static std::shared_ptr<AppEventCache> instance_;
    std::shared_ptr<AppEventStore> store_;
    std::map<std::string, std::shared_ptr<AppEventBlock>> blocks_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_CACHE_H
