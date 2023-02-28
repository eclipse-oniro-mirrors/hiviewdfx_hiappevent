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
#include "app_event_watcher_mgr.h"

#include "app_event_cache.h"
#include "app_event_handler.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, 0xD002D07, "HiAppEvent_WatcherMgr" };
constexpr int TIMEOUT_INTERVAL = 30000; // 30s
}
std::shared_mutex AppEventWatcherMgr::mutex_;
std::shared_ptr<AppEventWatcherMgr> AppEventWatcherMgr::instance_ = nullptr;

std::shared_ptr<AppEventWatcherMgr> AppEventWatcherMgr::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppEventWatcherMgr>();
        }
    }
    return instance_;
}

void AppEventWatcherMgr::AddWatcher(const std::shared_ptr<AppEventWatcher>& watcher)
{
    // if already exists, need to delete the old watcher first
    std::string name = watcher->GetName();
    if (watchers_.find(name) != watchers_.end()) {
        RemoveWatcher(name);
    }

    // need to create a block for the watcher
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (AppEventCache::GetInstance()->CreateBlock(name) != 0) {
        HiLog::Error(LABEL, "addWatcher: failed to create block");
        return;
    }
    watchers_[name] = watcher;

    // if the timeOut condition is set, need to create the event handler
    if (watcher->GetCond().timeOut > 0 && handler_ == nullptr) {
        if (CreateEventHandler() == nullptr) {
            HiLog::Error(LABEL, "addWatcher: failed to create handler");
        }
        handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
    }
    HiLog::Info(LABEL, "add watcher=%{public}s successfully", name.c_str());
}

void AppEventWatcherMgr::RemoveWatcher(const std::string& name)
{
    if (watchers_.find(name) == watchers_.end()) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (AppEventCache::GetInstance()->DestroyBlock(name) != 0) {
        HiLog::Error(LABEL, "failed to remove watcher: failed to destroy block");
    }
    watchers_.erase(name);

    // if the remaining watchers do not need to be triggered timely, the handler is destroyed
    auto it = std::find_if(watchers_.begin(), watchers_.end(),
        [&] (const std::map<std::string, std::shared_ptr<AppEventWatcher>>::value_type& watcher) {
            return watcher.second->GetCond().timeOut > 0;
        });
    if (it == watchers_.end()) {
        DestroyEventHandler();
    }
    HiLog::Info(LABEL, "remove watcher=%{public}s successfully", name.c_str());
}

void AppEventWatcherMgr::HandleEvent(const std::string& domain, int type, const std::string& event)
{
    HiLog::Debug(LABEL, "watcherMgr start to handle event");
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (auto it = watchers_.begin(); it != watchers_.end(); ++it) {
        it->second->ProcessEvent(domain, type, event);
    }
}

void AppEventWatcherMgr::HandleTimeOut()
{
    HiLog::Debug(LABEL, "watcherMgr start to handle timeout");
    if (handler_ == nullptr) {
        HiLog::Error(LABEL, "failed to handle timeOut: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
    std::shared_lock<std::shared_mutex> lock(mutex_);
    for (auto it = watchers_.begin(); it != watchers_.end(); ++it) {
        it->second->ProcessTimeOut();
    }
}

std::shared_ptr<AppEventHandler> AppEventWatcherMgr::CreateEventHandler()
{
    if (handler_ != nullptr) {
        return handler_;
    }
    auto runner = AppExecFwk::EventRunner::Create("AppEventHandler");
    if (runner == nullptr) {
        HiLog::Error(LABEL, "failed to create event runner");
        return nullptr;
    }
    handler_ = std::make_shared<AppEventHandler>(runner);
    return handler_;
}

void AppEventWatcherMgr::DestroyEventHandler()
{
    handler_= nullptr;
}
} // namespace HiviewDFX
} // namespace OHOS
