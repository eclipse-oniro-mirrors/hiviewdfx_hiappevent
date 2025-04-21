/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "app_crash_config.h"

#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AppCrashConfig"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int SANDBOX_LOG_MAX_CAPACITY = 5 * 1024 * 1024;
constexpr int DEFAULT_FAILURE_VALUE = -1;
constexpr int DEFAULT_SUCCESS_VALUE = 0;
struct LogConfig {
    uint8_t type;
    bool (*func)(const std::string&, const std::string&, uint32_t&);
};

extern "C" int DFX_SetCrashLogConfig(uint8_t type, uint32_t value) __attribute__((weak));

bool GetConfigUIntValue(const std::string& key, const std::string& value, uint32_t& configValue)
{
    if (value.empty()) {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value is empty.", key.c_str());
        return false;
    }

    char* numEndIndex = nullptr;
    double result = std::strtod(value.c_str(), &numEndIndex);
    if (*numEndIndex != '\0') {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value(%{public}s) should be a number.",
            key.c_str(), value.c_str());
        return false;
    }

    if (errno == ERANGE || result < 0 || result > std::numeric_limits<uint32_t>::max()) {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value(%{public}s) is over range.",
            key.c_str(), value.c_str());
        return false;
    }
    
    configValue = static_cast<uint32_t>(result);
    return true;
}

bool GetConfigBoolValue(const std::string& key, const std::string& value, uint32_t& configValue)
{
    if (value != "true" && value != "false") {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value(%{public}s) should be boolean type.",
            key.c_str(), value.c_str());
        return false;
    }

    configValue = value == "true" ? 1 : 0;
    return true;
}
}

int SetCrashEventConfig(const std::map<std::string, std::string>& configMap)
{
    int setResult = DEFAULT_FAILURE_VALUE;
    if (DFX_SetCrashLogConfig == nullptr) {
        HILOG_ERROR(LOG_CORE, "set crash log config func was not found.");
        return setResult;
    }

    std::map<std::string, LogConfig> CrashLogConfig = {
        {"extend_pc_lr_printing", {.type = 0, .func = GetConfigBoolValue}},
        {"log_file_cutoff_sz_bytes", {.type = 1, .func = GetConfigUIntValue}},
        {"simplify_vma_printing", {.type = 2, .func = GetConfigBoolValue}}
    };
    uint32_t configValue = 0;
    for (const auto& [key, value] : configMap) {
        auto it = CrashLogConfig.find(key);
        if (it == CrashLogConfig.end()) {
            HILOG_INFO(LOG_CORE, "Set crash config item(%{public}s) failed, because it is not supported.", key.c_str());
            continue;
        }
        if ((it->second.func)(key, value, configValue)) {
            (void)DFX_SetCrashLogConfig(it->second.type, configValue);
            HILOG_INFO(LOG_CORE, "Set crash config item(%{public}s) success. Value=%{public}d", key.c_str(),
                configValue);
            setResult = DEFAULT_SUCCESS_VALUE;
        }
    }
    return setResult;
}
} // HiviewDFX
} // namespace OHOS