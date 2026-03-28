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
#include "event_policy_utils.h"

#include <application_context.h>
#include <cerrno>
#include <hilog/log.h>
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "page_switch_log.h"
#include "storage_acl.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EventPolicyUtils"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr const char* const APP_EVENT_DIR = "/eventConfig";
constexpr const char* const PAGE_SWITCH_CONFIG = "pageSwitchLogEnable";
constexpr const char* const ADDRESS_SANITIZER_PAGE_SWITCH_LOG_ENABLE = "addrepageSwitchLogEnable";
constexpr const char* const APP_CRASH_PAGE_SWITCH_LOG_ENABLE = "appCrpageSwitchLogEnable";
constexpr const char* const APP_FREEZE_PAGE_SWITCH_LOG_ENABLE = "appFrpageSwitchLogEnable";
constexpr const char* const RESOURCE_OVERLIMIT_PAGE_SWITCH_LOG_ENABLE = "resoupageSwitchLogEnable";
}

EventPolicyUtils& EventPolicyUtils::GetInstance()
{
    static EventPolicyUtils instance;
    return instance;
}

int EventPolicyUtils::ConfigPageSwitch(const std::string& key, std::map<std::string, std::string>& configMap)
{
    auto value = configMap.find(PAGE_SWITCH_CONFIG);
    if (value == configMap.end()) {
        HILOG_ERROR(LOG_CORE, "pageSwitchLogEnable param does not exist.");
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    std::map<std::string, std::string> curConf = {{key, value->second}};
    configMap.erase(PAGE_SWITCH_CONFIG);
    std::string configDir = GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get event config dir");
        return ErrorCode::ERROR_UNKNOWN;
    }
    auto res = SaveEventConfig(configDir, curConf);
    if (res == ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
        SetPageSwitchStatus(GetCurSysPageSwitchStatus(configDir));
    }
    return res;
}

std::string EventPolicyUtils::GetConfigDir(const std::string& subDir)
{
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (!context || context->GetCacheDir().empty()) {
        HILOG_ERROR(LOG_CORE, "Context is null or cache dir is empty.");
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

bool EventPolicyUtils::GetEventPageSwitchStatus(const std::string& name)
{
    std::map<std::string, std::string> enableName = {{"APP_CRASH", "appCr"}, {"APP_FREEZE", "appFr"},
        {"RESOURCE_OVERLIMIT", "resou"}, {"ADDRESS_SANITIZER", "addre"}};
    auto propertyName = enableName.find(name);
    if (propertyName == enableName.end()) {
        HILOG_INFO(LOG_CORE, "%{public}s event is not support page switch log.", name.c_str());
        return false;
    }
    std::string configDir = GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return false;
    }
    std::string property = "user.event_config." + propertyName->second + PAGE_SWITCH_CONFIG;
    std::string value;
    if (!FileUtil::GetDirXattr(configDir, property, value)) {
        HILOG_WARN(LOG_CORE, "failed to get dir cfg xattr.");
        return false;
    }
    RemoveEventConfig(configDir, property);
    
    auto pos = value.find(",");
    if (pos == std::string::npos) {
        HILOG_WARN(LOG_CORE, "failed to parse history enable status. the status format is error.");
        return false;
    }
    return value.substr(pos + 1) == "true";
}

int EventPolicyUtils::SaveEventConfig(const std::string& configDir, const std::map<std::string, std::string>& configMap)
{
    if (runningId_.empty()) {
        runningId_ = HiAppEventConfig::GetInstance().GetRunningId();
        if (runningId_.empty()) {
            return ErrorCode::ERROR_UNKNOWN;
        }
    }

    for (const auto& config : configMap) {
        std::string property = "user.event_config." + config.first;
        std::string newValue = runningId_ + "," + config.second;
        if (!FileUtil::SetDirXattr(configDir, property, newValue)) {
            HILOG_ERROR(LOG_CORE, "failed to SetDirXattr, dir: %{public}s, property: %{public}s, newValue: %{public}s,"
                " err: %{public}s, errno: %{public}d", configDir.c_str(), property.c_str(), newValue.c_str(),
                strerror(errno), errno);
            return ErrorCode::ERROR_UNKNOWN;
        } else {
            HILOG_INFO(LOG_CORE, "succeed to UpdateProperty, property: %{public}s, newValue: %{public}s",
                property.c_str(), newValue.c_str());
        }
    }
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}

bool EventPolicyUtils::GetCurSysPageSwitchStatus(const std::string& configDir)
{
    std::vector<std::string> keys = {ADDRESS_SANITIZER_PAGE_SWITCH_LOG_ENABLE, APP_CRASH_PAGE_SWITCH_LOG_ENABLE,
        APP_FREEZE_PAGE_SWITCH_LOG_ENABLE, RESOURCE_OVERLIMIT_PAGE_SWITCH_LOG_ENABLE};
    std::string value;
    for (const auto& key : keys) {
        if (!FileUtil::GetDirXattr(configDir, "user.event_config." + key, value)) {
            continue;
        }
        auto pos = value.find(",");
        if (pos == std::string::npos) {
            HILOG_WARN(LOG_CORE, "failed to parse history enable status. the value format is error.");
            continue;
        }
        if (value.substr(0, pos) == runningId_ && value.substr(pos + 1) == "true") {
            return true;
        }
    }
    return false;
}

void EventPolicyUtils::RemoveEventConfig(const std::string& configDir, const std::string& property)
{
    std::string historyRunningId;
    if (!FileUtil::GetDirXattr(configDir, property, historyRunningId)) {
        HILOG_WARN(LOG_CORE, "failed to get dir runningId xattr, clear history enable status failed.");
        return;
    }
    auto pos = historyRunningId.find(",");
    if (pos == std::string::npos) {
        HILOG_WARN(LOG_CORE, "failed to clear history enable status. the value format is error.");
        return;
    }

    historyRunningId = historyRunningId.substr(0, pos);
    if (runningId_.empty()) {
        runningId_ = HiAppEventConfig::GetInstance().GetRunningId();
    }
    if (runningId_ != historyRunningId) {
        if (!FileUtil::RemoveDirXattr(configDir, property)) {
            HILOG_WARN(LOG_CORE, "failed to clear history enable status.");
        }
    }
}
}  // HiviewDFX
}  // OHOS
