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

#include "napi_env_watcher_manager.h"

#include <algorithm>

#include "app_event_observer_mgr.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EnvWatcherManager"

namespace OHOS {
namespace HiviewDFX {

EnvWatcherManager &EnvWatcherManager::GetInstance()
{
    static EnvWatcherManager envWatcherManager;
    return envWatcherManager;
}

void EnvWatcherManager::RemoveEnvWatcherRecord(const NapiAppEventWatcher* watcher)
{
    if (watcher) {
        std::lock_guard<std::mutex> lockGuard(mutex_);
        napiEnvWatcherRecords_.remove_if([watcher](const auto& pair) {
            return pair.second == watcher;
        });
    }
}

void EnvWatcherManager::AddEnvWatcherRecord(const napi_env env, NapiAppEventWatcher* watcher)
{
    if (!watcher) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (envs_.find(env) == envs_.end()) {
        auto envPtr = new napi_env{env};
        if (napi_add_env_cleanup_hook(env, EnvWatcherManager::OnEnvRemove, envPtr) == napi_ok) {
            envs_.insert(env);
        } else {
            HILOG_ERROR(LOG_CORE, "failed in napi_add_env_cleanup_hook.");
        }
    }
    napiEnvWatcherRecords_.emplace_back(env, watcher);
}

void EnvWatcherManager::OnEnvRemove(void* envPtr)
{
    if (envPtr == nullptr) {
        HILOG_ERROR(LOG_CORE, "cur envPtr is nullptr.");
        return;
    }
    auto env = reinterpret_cast<napi_env*>(envPtr);
    EnvWatcherManager& manager = GetInstance();
    std::lock_guard<std::mutex> lockGuard(manager.mutex_);
    manager.envs_.erase(*env);
    manager.napiEnvWatcherRecords_.remove_if([env](const auto& pair) {
        if (pair.first == *env) {
            pair.second->DeleteWatcherContext();
            return true;
        }
        return false;
    });
    delete env;
}
}
}
