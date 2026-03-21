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
#include "resource_overlimit_policy.h"

#include <hilog/log.h>
#include <set>
#include "event_policy_utils.h"
#include "hiappevent_base.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ResourceOverlimitPolicy"

namespace OHOS {
namespace HiviewDFX {
namespace {
inline constexpr const char* const RAWHEAP = "/rawheap";
constexpr int INVALID_PARAM = -1;

bool IsValid(const std::string& key, const std::string& value)
{
    const std::map<std::string, std::set<std::string>> whiteList = {
        {"js_heap_logtype", {"event", "event_rawheap"}},
        {"jsHeapLogtype", {"event", "event_rawheap"}}
    };

    auto it = whiteList.find(key);
    if (it == whiteList.end()) {
        HILOG_ERROR(LOG_CORE, "invalid config key: %{public}s", key.c_str());
        return false;
    }
    if (it->second.find(value) == it->second.end()) {
        HILOG_ERROR(LOG_CORE, "invalid value for key %{public}s: %{public}s", key.c_str(), value.c_str());
        return false;
    }
    return true;
}

int SetEventConfig(const std::map<std::string, std::string>& configMap)
{
    int rtn = ErrorCode::ERROR_UNKNOWN;
    std::string configDir = EventPolicyUtils::GetInstance().GetConfigDir(RAWHEAP);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get event config dir");
        return rtn;
    }

    for (const auto& [key, value] : configMap) {
        if (!IsValid(key, value)) {
            HILOG_WARN(LOG_CORE, "failed SetEventConfig, invalid key %{public}s, value: %{public}s",
                       key.c_str(), value.c_str());
            continue;
        }
        if (key == "js_heap_logtype" || key == "jsHeapLogtype") {
            std::map<std::string, std::string> curMap = {{"js_heap_logtype", value}};
            rtn = EventPolicyUtils::GetInstance().SaveEventConfig(configDir, curMap);
        }
    }
    return rtn;
}
}

int ResourceOverlimitPolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    int res = EventPolicyUtils::GetInstance().ConfigPageSwitch("resoupageSwitchLogEnable", conf);
    if (conf.size() < configMap.size()) {  // whether pageSwitchLogEnable is set.
        if (conf.size() == 0 || res != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
            return res;
        }
    }
    return SetEventConfig(conf);
}

int ResourceOverlimitPolicy::SetEventPolicy(const std::map<uint8_t, uint32_t>& configMap)
{
    return INVALID_PARAM;
}
}  // HiviewDFX
}  // OHOS
