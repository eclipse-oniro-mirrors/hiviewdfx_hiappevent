/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_OS_EVENT_LISTENER_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_OS_EVENT_LISTENER_H

#include <string>
#include <thread>
#include <vector>

#include "json/json.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

class OsEventListener {
public:
    OsEventListener();
    ~OsEventListener();
    bool StartListening();
    bool RemoveOsEventDir();
    void GetEvents(std::vector<std::shared_ptr<AppEventPack>>& events);

private:
    void Init();
    bool InitDir(const std::string& dirPath);
    bool RegisterDirListener(const std::string& dirPath);
    void HandleDirEvent();
    void HandleInotify(const std::string& files);
    void GetEventsFromFiles(const std::vector<std::string>& files, std::vector<std::shared_ptr<AppEventPack>>& events);
    std::shared_ptr<AppEventPack> GetAppEventPackFromJson(const std::string& jsonStr);

private:
    int inotifyFd_ = -1;
    int inotifyWd_ = -1;
    bool inotifyStopFlag_ = true;
    std::string osEventPath_;
    std::unique_ptr<std::thread> inotifyThread_ = nullptr;
    std::vector<std::shared_ptr<AppEventPack>> historyEvents_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_OS_EVENT_LISTENER_H