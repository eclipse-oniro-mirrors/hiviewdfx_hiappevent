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
constexpr const char* const APP_EVENT_DIR = "/eventConfig";
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
        HILOG_ERROR(LOG_CORE, "failed to get rawheap config dir");
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

int SaveEventConfig(std::map<std::string, std::string>& configMap)
{
    if (configMap.find("useRefinedLogFileName") == configMap.end()) {
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    std::string configDir = EventPolicyUtils::GetInstance().GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get event config dir");
        return ErrorCode::ERROR_UNKNOWN;
    }
    int ret = EventPolicyUtils::GetInstance().SaveEventConfig(
        configDir, {{"useRefinedLogFileName", configMap["useRefinedLogFileName"]}}, false);
    configMap.erase("useRefinedLogFileName");
    return ret;
}
}

int ResourceOverlimitPolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    if (configMap.empty()) {
        HILOG_WARN(LOG_CORE, "the resource overlimit policy config is empty.");
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    auto conf = configMap;
    if (configMap.find("pageSwitchLogEnable") != configMap.end()) {  // whether pageSwitchLogEnable is set.
        int ret = EventPolicyUtils::GetInstance().ConfigPageSwitch("RESOURCE_OVERLIMIT", conf);
        if (conf.size() == 0 || ret != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
            return ret;
        }
    }
    if (configMap.find("useRefinedLogFileName") != configMap.end()) {  // whether useRefinedLogFileName is set.
        int ret = SaveEventConfig(conf);
        if (conf.size() == 0 || ret != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
            return ret;
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