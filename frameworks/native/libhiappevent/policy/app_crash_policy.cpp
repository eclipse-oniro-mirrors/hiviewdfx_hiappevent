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
#include <hilog/log.h>
#include "event_policy_utils.h"
#include "hiappevent_base.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AppCrashPolicy"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr uint32_t MAX_CUTOFF_SZ_BYTES = 5 * 1024 * 1024; // 5M: 5242880
enum CrashLogConfigType : uint8_t {
    EXTEND_PC_LR_PRINTING = 0,
    LOG_FILE_CUTOFF_SZ_BYTES,
    SIMPLIFY_VMA_PRINTING,
    MERGE_CPPCRASH_APP_LOG,
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
    out = strtoul(strValue.c_str(), &end, decimal);
    if (strValue.empty() || *end != '\0' || errno == ERANGE) {
        HILOG_ERROR(LOG_CORE, "Set crash config item failed, failed to convert str to int.");
        return false;
    }
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
}

int AppCrashPolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    int res = EventPolicyUtils::GetInstance().ConfigPageSwitch("appCrpageSwitchLogEnable", conf);
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

int AppCrashPolicy::SetAppCrashLogPolicy(const std::map<std::string, std::string>& configMap)
{
    std::map<std::string, crashConfig> crashConfigs = {
        {"extendPcLrPrinting", {.type = EXTEND_PC_LR_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"extend_pc_lr_printing", {.type = EXTEND_PC_LR_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"logFileCutoffSzBytes", {.type = LOG_FILE_CUTOFF_SZ_BYTES, .func = ChangeCrashConfigToUIntValue}},
        {"log_file_cutoff_sz_bytes", {.type = LOG_FILE_CUTOFF_SZ_BYTES, .func = ChangeCrashConfigToUIntValue}},
        {"simplifyVmaPrinting", {.type = SIMPLIFY_VMA_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"simplify_vma_printing", {.type = SIMPLIFY_VMA_PRINTING, .func = ChangeCrashConfigToBoolValue}},
        {"merge_cppcrash_app_log", {.type = MERGE_CPPCRASH_APP_LOG, .func = ChangeCrashConfigToBoolValue}}
    };
    std::map<uint8_t, uint32_t> crashConfigMap;
    uint32_t configValue = 0;
    for (const auto& [key, value] : configMap) {
        auto it = crashConfigs.find(key);
        if (it == crashConfigs.end()) {
            HILOG_WARN(LOG_CORE, "Set crash config item failed, the item(%{public}s) is invalid.", key.c_str());
            continue;
        }
        if ((it->second.func)(value, configValue)) {
            crashConfigMap[it->second.type] = configValue;
        } else {
            HILOG_WARN(LOG_CORE, "Set crash config item failed, the value(%{public}s) is invalid.", value.c_str());
        }
    }
    if (crashConfigMap.empty()) {
        HILOG_ERROR(LOG_CORE, "Failed to set crash log config, name or value is invalid.");
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return SetEventPolicy(crashConfigMap);
}
}  // HiviewDFX
}  // OHOS