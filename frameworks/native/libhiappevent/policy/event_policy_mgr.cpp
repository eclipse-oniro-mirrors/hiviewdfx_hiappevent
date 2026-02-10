/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <application_context.h>
#include <hilog/log.h>

#include "file_util.h"
#include "hiappevent_config.h"
#include "hiappevent_base.h"
#include "page_switch_log.h"
#include "resource_overlimit_mgr.h"
#include "storage_acl.h"
#include "xcollie/watchdog.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EventPolicyMgr"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr const char* const APP_EVENT_DIR = "/eventConfig";
constexpr const char* const PAGE_SWITCH_CONFIG = "pageSwitchLogEnable";
constexpr const char* const MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
constexpr const char* const MAIN_THREAD_JANK_V2 = "MAIN_THREAD_JANK_V2";
constexpr const char* const ADDRESS_SANITIZER_POLICY = "addressSanitizerPolicy";
constexpr const char* const APP_CRASH_POLICY = "appCrashPolicy";
constexpr const char* const APP_FREEZE_POLICY = "appFreezePolicy";
constexpr const char* const CPU_USAGE_HIGH_POLICY = "cpuUsageHighPolicy";
constexpr const char* const MAIN_THREAD_JANK_POLICY = "mainThreadJankPolicy";
constexpr const char* const RESOURCE_OVERLIMIT_POLICY = "resourceOverlimitPolicy";
constexpr const char* const ADDRESS_SANITIZER_PAGE_SWITCH_LOG_ENABLE = "addrepageSwitchLogEnable";
constexpr const char* const APP_CRASH_PAGE_SWITCH_LOG_ENABLE = "appCrpageSwitchLogEnable";
constexpr const char* const APP_FREEZE_PAGE_SWITCH_LOG_ENABLE = "appFrpageSwitchLogEnable";
constexpr const char* const RESOURCE_OVERLIMIT_PAGE_SWITCH_LOG_ENABLE = "resoupageSwitchLogEnable";

extern "C" int DFX_SetCrashLogConfig(uint8_t type, uint32_t value) __attribute__((weak));

std::string GetConfigDir(const std::string& subDir)
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "Context is null.");
        return "";
    }
    if (context->GetCacheDir().empty()) {
        HILOG_ERROR(LOG_CORE, "cache dir obtained from context is empty.");
        return "";
    }
    std::string configDir = context->GetCacheDir() + subDir;
    if (!FileUtil::IsFileExists(configDir) && !FileUtil::ForceCreateDirectory(configDir)) {
        HILOG_ERROR(LOG_CORE, "failed to create dir=%{public}s", configDir.c_str());
        return "";
    }
    if (OHOS::StorageDaemon::AclSetAccess(configDir, "u:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set acl access dir=%{public}s", configDir.c_str());
        return "";
    }
    return configDir;
}

int SaveEventConfig(const std::string& configDir, const std::map<std::string, std::string>& configMap)
{
    for (const auto& config : configMap) {
        std::string property = "user.event_config." + config.first;
        if (!FileUtil::SetDirXattr(configDir, property, config.second)) {
            HILOG_ERROR(LOG_CORE, "failed to SetDirXattr, dir: %{public}s, property: %{public}s, newValue: %{public}s,"
                " err: %{public}s, errno: %{public}d", configDir.c_str(), property.c_str(), config.second.c_str(),
                strerror(errno), errno);
            return ErrorCode::ERROR_UNKNOWN;
        } else {
            HILOG_INFO(LOG_CORE, "succeed to UpdateProperty, property: %{public}s, newValue: %{public}s",
                property.c_str(), config.second.c_str());
        }
    }
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}

bool GetCurSysPageSwitchStatus(const std::string& configDir)
{
    std::vector<std::string> keys = {ADDRESS_SANITIZER_PAGE_SWITCH_LOG_ENABLE, APP_CRASH_PAGE_SWITCH_LOG_ENABLE,
        APP_FREEZE_PAGE_SWITCH_LOG_ENABLE, RESOURCE_OVERLIMIT_PAGE_SWITCH_LOG_ENABLE};
    std::string value;
    for (const auto& key : keys) {
        if (FileUtil::GetDirXattr(configDir, "user.event_config." + key, value) && value == "true") {
            return true;
        }
    }
    return false;
}

int ConfigPageSwitch(const std::string& key, std::map<std::string, std::string>& configMap)
{
    auto value = configMap.find(PAGE_SWITCH_CONFIG);
    if (value == configMap.end()) {
        HILOG_ERROR(LOG_CORE, "pageSwitchLogEnable param is not set.");
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    std::map<std::string, std::string> curConf = {{key, value->second}};
    configMap.erase(PAGE_SWITCH_CONFIG);
    std::string configDir = GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return ErrorCode::ERROR_UNKNOWN;
    }
    auto res = SaveEventConfig(configDir, curConf);
    if (res == ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
        SetPageSwitchStatus(GetCurSysPageSwitchStatus(configDir));
    }
    return res;
}

std::map<std::string, std::string> UnifiedCAndTsParamNames(const std::map<std::string, std::string>& configMap)
{
    std::map<std::string, std::string> paramTs2CMap = {
        {"logType", "log_type"}, {"ignoreStartupTime", "ignore_startup_time"}, {"sampleInterval", "sample_interval"},
        {"sampleCount", "sample_count"}, {"reportTimesPerApp", "report_times_per_app"},
        {"autoStopSampling", "auto_stop_sampling"}
    };
    std::map<std::string, std::string> unifiedConfigMap;
    for (const auto& config : configMap) {
        auto cKey = paramTs2CMap.find(config.first);
        if (cKey == paramTs2CMap.end()) {
            unifiedConfigMap[config.first] = config.second;
        } else {
            unifiedConfigMap[cKey->second] = config.second;
        }
    }
    return unifiedConfigMap;
}

int SetAddressSanitizerPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    return ConfigPageSwitch(ADDRESS_SANITIZER_PAGE_SWITCH_LOG_ENABLE, conf);
}

int SetAppCrashPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    return ConfigPageSwitch(APP_CRASH_PAGE_SWITCH_LOG_ENABLE, conf);
}

int SetAppFreezePolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    return ConfigPageSwitch(APP_FREEZE_PAGE_SWITCH_LOG_ENABLE, conf);
}

int SetCpuUsageHighPolicy(const std::map<std::string, std::string>& configMap)
{
    std::string configDir = GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return ErrorCode::ERROR_UNKNOWN;
    }
    return SaveEventConfig(configDir, configMap);
}

int SetMainThreadJankConfig(const std::map<std::string, std::string>& configMap)
{
    return Watchdog::GetInstance().SetEventConfig(configMap);
}

int SetMainThreadJankPolicy(const std::map<std::string, std::string>& configMap)
{
    auto unifiedConfigMap = UnifiedCAndTsParamNames(configMap);
    return Watchdog::GetInstance().ConfigEventPolicy(unifiedConfigMap);
}

int SetResourceOverlimitPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    int res = ConfigPageSwitch(RESOURCE_OVERLIMIT_PAGE_SWITCH_LOG_ENABLE, conf);
    if (conf.size() < configMap.size()) {
        if (conf.size() == 0 || res != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
            return res;
        }
    }
    ResourceOverlimitMgr::GetInstance().SetRunningId(HiAppEventConfig::GetInstance().GetRunningId());
    return ResourceOverlimitMgr::GetInstance().SetEventConfig(conf);
}
}

EventPolicyMgr& EventPolicyMgr::GetInstance()
{
    static EventPolicyMgr instance;
    return instance;
}

int EventPolicyMgr::SetEventPolicy(const std::string& name, const std::map<std::string, std::string>& configMap) const
{
    std::map<std::string, std::function<int(const std::map<std::string, std::string>&)>> eventPolicy = {
        {ADDRESS_SANITIZER_POLICY, SetAddressSanitizerPolicy},
        {APP_CRASH_POLICY, SetAppCrashPolicy},
        {APP_FREEZE_POLICY, SetAppFreezePolicy},
        {CPU_USAGE_HIGH_POLICY, SetCpuUsageHighPolicy},
        {MAIN_THREAD_JANK, SetMainThreadJankConfig},
        {MAIN_THREAD_JANK_V2, SetMainThreadJankPolicy},
        {MAIN_THREAD_JANK_POLICY, SetMainThreadJankPolicy},
        {RESOURCE_OVERLIMIT, SetResourceOverlimitPolicy},
        {RESOURCE_OVERLIMIT_POLICY, SetResourceOverlimitPolicy}
    };
    auto policy = eventPolicy.find(name);
    if (policy == eventPolicy.end()) {
        HILOG_ERROR(LOG_CORE, "Failed to set event policy, name is invalid. name=%{public}s", name.c_str());
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return (policy->second)(configMap);
}

int EventPolicyMgr::SetEventPolicy(const std::map<uint8_t, uint32_t> &configMap) const
{
    if (DFX_SetCrashLogConfig == nullptr) {
        HILOG_ERROR(LOG_CORE, "set crash log config func was not found.");
        return ErrorCode::ERROR_UNKNOWN;
    }
 
    for (const auto& [key, value] : configMap) {
        (void)DFX_SetCrashLogConfig(key, value);
    }
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}

bool EventPolicyMgr::GetEventPageSwitchStatus(const std::string& eventName) const
{
    std::map<std::string, std::string> enableName = {{"APP_CRASH", "appCr"}, {"APP_FREEZE", "appFr"},
        {"RESOURCE_OVERLIMIT", "resou"}, {"ADDRESS_SANITIZER", "addre"}};
    auto propertyName = enableName.find(eventName);
    if (propertyName == enableName.end()) {
        HILOG_INFO(LOG_CORE, "%{public}s event is not support page switch log.", eventName.c_str());
        return false;
    }
    std::string configDir = GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return false;
    }
    std::string property = "user.event_config." + propertyName->second + PAGE_SWITCH_CONFIG;
    std::string value;
    if (!FileUtil::GetDirXattr(configDir, property, value) || value == "false") {
        HILOG_INFO(LOG_CORE, "failed to get dir xattr or pageSwitch value=%{public}s.", value.c_str());
        return false;
    }
    return true;
}
}  // HiviewDFX
}  // OHOS
