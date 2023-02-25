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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_WATCHER_APP_EVENT_WATCHER_MGR_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_WATCHER_APP_EVENT_WATCHER_MGR_H

#include <map>
#include <memory>
#include <shared_mutex>

#include "app_event_watcher.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventHandler;

class AppEventWatcherMgr {
public:
    static std::shared_ptr<AppEventWatcherMgr> GetInstance();
    void AddWatcher(const std::shared_ptr<AppEventWatcher>& watcher);
    void RemoveWatcher(const std::string& name);
    void HandleEvent(const std::string& domain, int type, const std::string& event);
    void HandleTimeOut();

private:
    std::shared_ptr<AppEventHandler> CreateEventHandler();
    void DestroyEventHandler();

private:
    std::map<std::string, std::shared_ptr<AppEventWatcher>> watchers_;
    std::shared_ptr<AppEventHandler> handler_;
    static std::shared_mutex mutex_;
    static std::shared_ptr<AppEventWatcherMgr> instance_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_WATCHER_APP_EVENT_WATCHER_MGR_H
