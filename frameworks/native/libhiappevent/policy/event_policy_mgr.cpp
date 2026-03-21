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
#include "event_policy_mgr.h"

#include <hilog/log.h>

#include "event_policy_utils.h"
#include "hiappevent_base.h"
#include "address_sanitizer_policy.h"
#include "app_crash_policy.h"
#include "app_freeze_policy.h"
#include "cpu_usage_high_policy.h"
#include "main_thread_jank_policy.h"
#include "resource_overlimit_policy.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EventPolicyMgr"

namespace OHOS {
namespace HiviewDFX {
EventPolicyMgr::EventPolicyMgr()
{
    InitializePolicies();
}

void EventPolicyMgr::InitializePolicies()
{
    RegisterPolicy("addressSanitizerPolicy", std::make_shared<AddressSanitizerPolicy>());
    RegisterPolicy("appCrashPolicy", std::make_shared<AppCrashPolicy>());
    RegisterPolicy("APP_CRASH", std::make_shared<AppCrashPolicy>());
    RegisterPolicy("appFreezePolicy", std::make_shared<AppFreezePolicy>());
    RegisterPolicy("cpuUsageHighPolicy", std::make_shared<CpuUsageHighPolicy>());
    RegisterPolicy("MAIN_THREAD_JANK", std::make_shared<MainThreadJankConfig>());
    RegisterPolicy("MAIN_THREAD_JANK_V2", std::make_shared<MainThreadJankPolicy>());
    RegisterPolicy("mainThreadJankPolicy", std::make_shared<MainThreadJankPolicy>());
    RegisterPolicy("RESOURCE_OVERLIMIT", std::make_shared<ResourceOverlimitPolicy>());
    RegisterPolicy("resourceOverlimitPolicy", std::make_shared<ResourceOverlimitPolicy>());
}

void EventPolicyMgr::RegisterPolicy(const std::string& name, EventPolicyPtr policy)
{
    if (policy) {
        policies_[name] = policy;
    }
}

EventPolicyMgr& EventPolicyMgr::GetInstance()
{
    static EventPolicyMgr instance;
    return instance;
}

int EventPolicyMgr::SetEventPolicy(const std::string& name, const std::map<std::string, std::string>& configMap) const
{
    auto policy = policies_.find(name);
    if (policy == policies_.end()) {
        HILOG_ERROR(LOG_CORE, "Failed to set str event policy, name(%{public}s) is invalid.", name.c_str());
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return policy->second->SetEventPolicy(configMap);
}

int EventPolicyMgr::SetEventPolicy(const std::string& name, const std::map<uint8_t, uint32_t> &configMap) const
{
    auto policy = policies_.find(name);
    if (policy == policies_.end()) {
        HILOG_ERROR(LOG_CORE, "Failed to set int event policy, name(%{public}s) is invalid.", name.c_str());
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return policy->second->SetEventPolicy(configMap);
}

bool EventPolicyMgr::GetEventPageSwitchStatus(const std::string& name) const
{
    return EventPolicyUtils::GetInstance().GetEventPageSwitchStatus(name);
}
}  // HiviewDFX
}  // OHOS
