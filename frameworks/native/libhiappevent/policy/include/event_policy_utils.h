/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_POLICY_EVENT_POLICY_UTILS_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_POLICY_EVENT_POLICY_UTILS_H

#include <map>
#include <string>

namespace OHOS {
namespace HiviewDFX {
class EventPolicyUtils {
public:
    static EventPolicyUtils& GetInstance();
    EventPolicyUtils(const EventPolicyUtils&) = delete;
    EventPolicyUtils& operator=(const EventPolicyUtils&) = delete;

    int ConfigPageSwitch(const std::string& key, std::map<std::string, std::string>& configMap);
    std::string GetConfigDir(const std::string& subDir);
    bool GetEventPageSwitchStatus(const std::string& name);
    int SaveEventConfig(const std::string& configDir, const std::map<std::string, std::string>& configMap);

private:
    EventPolicyUtils() = default;
    ~EventPolicyUtils() = default;
    bool GetCurSysPageSwitchStatus(const std::string& configDir);
    void RemoveEventConfig(const std::string& configDir, const std::string& property);
    std::string runningId_;
};
}  // HiviewDFX
}  // OHOS
#endif  // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_POLICY_EVENT_POLICY_UTILS_H