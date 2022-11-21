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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_WATCHER_APP_EVENT_WATCHER_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_WATCHER_APP_EVENT_WATCHER_H

#include <map>
#include <mutex>
#include <string>

namespace OHOS {
namespace HiviewDFX {
struct TriggerCondition {
    int row;
    int size;
    int timeOut;
};

class AppEventWatcher {
public:
    AppEventWatcher(const std::string& name, const std::map<std::string, unsigned int>& filters,
        TriggerCondition cond);
    virtual ~AppEventWatcher() {}
    virtual void OnTrigger(int row, int size);
    std::string GetName() const;
    TriggerCondition GetCond() const;
    void ProcessEvent(const std::string& domain, int type, const std::string& event);
    void ProcessTimeOut();

private:
    bool IsInterestedEvent(const std::string& domain, int type);
    void ResetStatus();

private:
    std::string name_;
    std::map<std::string, unsigned int> filters_;
    TriggerCondition cond_;
    TriggerCondition status_;
    std::mutex mutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_WATCHER_APP_EVENT_WATCHER_H
