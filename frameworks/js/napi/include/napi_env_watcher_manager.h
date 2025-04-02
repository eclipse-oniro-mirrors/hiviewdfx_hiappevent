/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ENV_WATCHER_MANAGER_H_
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ENV_WATCHER_MANAGER_H_

#include <list>
#include <mutex>
#include <set>

#include "napi_app_event_watcher.h"
#include "napi/native_api.h"

namespace OHOS {
namespace HiviewDFX {

class EnvWatcherManager {
public:
    static EnvWatcherManager& GetInstance();

    EnvWatcherManager& operator=(const EnvWatcherManager&) = delete;
    EnvWatcherManager(const EnvWatcherManager&) = delete;
    EnvWatcherManager& operator=(const EnvWatcherManager&&) = delete;
    EnvWatcherManager(const EnvWatcherManager&&) = delete;
    void RemoveEnvWatcherRecord(const NapiAppEventWatcher* watcher);
    void AddEnvWatcherRecord(const napi_env env, NapiAppEventWatcher* const watcher);
private:
    ~EnvWatcherManager() = default;
    EnvWatcherManager() = default;
    static void OnEnvRemove(void* env);
    std::mutex mutex_;
    std::list<std::pair<const napi_env, NapiAppEventWatcher*>> napiEnvWatcherRecords_;
    std::set<const napi_env> envs_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_ENV_WATCHER_MANAGER_H_
