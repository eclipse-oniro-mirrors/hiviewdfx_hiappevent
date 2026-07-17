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
#include "app_crash_policy.h"
 	 
#include <cerrno>
#include <cstdint>
#include <hilog/log.h>
#include "event_policy_utils.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "storage_acl.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AppCrashPolicy"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t MAX_CUTOFF_SZ_BYTES = 5 * 1024 * 1024; // 5M: 5242880
constexpr const char* DMP_INFO_PATH = "/data/storage/el2/log/hiappevent/info";
constexpr const char* APP_EVENT_DIR = "/eventConfig";
enum CrashLogConfigType : uint8_t {
    EXTEND_PC_LR_PRINTING = 0,
    LOG_FILE_CUTOFF_SZ_BYTES,
    SIMPLIFY_VMA_PRINTING,
    MERGE_CPPCRASH_APP_LOG,
    COLLECT_MINIDUMP
};
struct crashConfig {
    uint8_t type;
    std::function<bool(const std::string&, uint32_t&)> func;
};

extern "C" int DFX_SetCrashLogConfig(uint8_t type, uint32_t value) __attribute__((weak));

bool ChangeCrashConfigToUIntValue(const std::string& strValue, uint32_t& out)
{
    const int decimal = 10;
    char* end;
    unsigned long result = strtoul(strValue.c_str(), &end, decimal);
    if (strValue.empty() || *end != '\0' || errno == ERANGE || result > UINT32_MAX) {
        HILOG_ERROR(LOG_CORE, "Set crash config item failed, failed to convert str to int.");
        return false;
    }
    out = static_cast<uint32_t>(result);
    if (out > MAX_CUTOFF_SZ_BYTES) {
        HILOG_ERROR(LOG_CORE, "Set crash config item failed, the value(%{public}s) is over range.", strValue.c_str());
        return false;
    }
    return true;
}

bool ChangeCrashConfigToBoolValue(const std::string& strValue, uint32_t& out)
{
    if (strValue == "true") {
        out = 1;
        return true;
    } else if (strValue == "false") {
        out = 0;
        return true;
    }
    HILOG_ERROR(LOG_CORE, "Set crash config item failed, the value(%{public}s) should be bool type.", strValue.c_str());
    return false;
}

bool SetMinidumpConfig(const std::string& strValue, uint32_t& out)
{
    if (ChangeCrashConfigToBoolValue(strValue, out) == false) {
        HILOG_ERROR(LOG_CORE, "failed to set crash config item, the value(%{public}s) should be bool type.",
            strValue.c_str());
        return false;
    }
    bool ret = false;
    std::string configDir = EventPolicyUtils::GetInstance().GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return false;
    }

    if (EventPolicyUtils::GetInstance().SaveEventConfig(configDir, {{"minidump", strValue}}, false) ==
        ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
        ret = true;
        HILOG_INFO(LOG_CORE, "set minidump config to %{public}d", out);
        }
    return ret;
}

void RemoveInfoDir(const std::string& dirPath)
{
    if (FileUtil::IsFileExists(dirPath) && !FileUtil::ForceRemoveDirectory(dirPath)) {
        HILOG_ERROR(LOG_CORE, "failed to remove dir=%{public}s", dirPath.c_str());
    }
}
}

int AppCrashPolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    int res = EventPolicyUtils::GetInstance().ConfigPageSwitch("APP_CRASH", conf);
    if (conf.size() < configMap.size()) {  // whether pageSwitchLogEnable is set.
        if (conf.size() == 0 || res != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
            return res;
        }
    }
    return SetAppCrashLogPolicy(conf);
}

int AppCrashPolicy::SetEventPolicy(const std::map<uint8_t, uint32_t> &configMap)
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

AppCrashPolicy::AppCrashPolicy()
{
    RemoveInfoDir(DMP_INFO_PATH);
    InitMiniDumpConfig();
}

int AppCrashPolicy::SetAppCrashLogPolicy(const std::map<std::string, std::string>& configMap)
{
    if (configMap.empty()) {
        HILOG_WARN(LOG_CORE, "the app crash policy is empty.");
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    std::map<std::string, crashConfig> crashConfigs = {
        {"extendPcLrPrinting", {.type = EXTEND_PC_LR_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"extend_pc_lr_printing", {.type = EXTEND_PC_LR_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"logFileCutoffSzBytes", {.type = LOG_FILE_CUTOFF_SZ_BYTES, .func = ChangeCrashConfigToUIntValue}},
        {"log_file_cutoff_sz_bytes", {.type = LOG_FILE_CUTOFF_SZ_BYTES, .func = ChangeCrashConfigToUIntValue}},
        {"simplifyVmaPrinting", {.type = SIMPLIFY_VMA_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"simplify_vma_printing", {.type = SIMPLIFY_VMA_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"merge_cppcrash_app_log", {.type = MERGE_CPPCRASH_APP_LOG, .func = ChangeCrashConfigToBoolValue}},
        {"collectMinidump", {.type = COLLECT_MINIDUMP, .func = SetMinidumpConfig}},
        {"collect_minidump", {.type = COLLECT_MINIDUMP, .func = SetMinidumpConfig}}
    };
    std::map<uint8_t, uint32_t> crashConfigMap;
    uint32_t configValue = 0;
    for (const auto& [key, value] : configMap) {
        auto it = crashConfigs.find(key);
        if (it == crashConfigs.end()) {
            HILOG_WARN(LOG_CORE, "failed to set crash config item, the item(%{public}s) is invalid.", key.c_str());
            continue;
        }
        if ((it->second.func)(value, configValue)) {
            crashConfigMap[it->second.type] = configValue;
        } else {
            HILOG_WARN(LOG_CORE, "failed to set crash config item. The value(%{public}s) is invalid or operation "
                "failed.", value.c_str());
        }
    }
    if (crashConfigMap.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to set crash log config, name or value is invalid.");
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return SetEventPolicy(crashConfigMap);
}

void AppCrashPolicy::InitMiniDumpConfig()
{
    std::string configDir = EventPolicyUtils::GetInstance().GetConfigDir(APP_EVENT_DIR);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return;
    }
    std::string property = std::string("user.event_config.minidump");
    std::string value;
    if (!FileUtil::GetDirXattr(configDir, property, value)) {
        HILOG_WARN(LOG_CORE, "failed to get dir cfg xattr about minidump.");
        return;
    }
    if (value == "true") {
        SetEventPolicy({{COLLECT_MINIDUMP, 1}});
        HILOG_INFO(LOG_CORE, "get minidump config in init is true");
    }
}
}  // HiviewDFX
}  // OHOS