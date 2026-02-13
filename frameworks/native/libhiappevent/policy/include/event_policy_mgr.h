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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_POLICY_EVENT_POLICY_MGR_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_POLICY_EVENT_POLICY_MGR_H

#include <map>
#include <string>

namespace OHOS {
namespace HiviewDFX {
class EventPolicyMgr {
public:
    static EventPolicyMgr& GetInstance();
    EventPolicyMgr(const EventPolicyMgr&) = delete;
    EventPolicyMgr& operator=(const EventPolicyMgr&) = delete;
    bool GetEventPageSwitchStatus(const std::string& eventName) const;
    int SetEventPolicy(const std::string& name, const std::map<std::string, std::string>& configMap) const;
    int SetEventPolicy(const std::map<uint8_t, uint32_t> &configMap) const;
private:
    EventPolicyMgr() = default;
    ~EventPolicyMgr() = default;
};
}  // HiviewDFX
}  // OHOS
#endif  // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_POLICY_EVENT_POLICY_MGR_H
