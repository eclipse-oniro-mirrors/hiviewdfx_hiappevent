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
#include "napi_config_builder.h"

#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiConfigBuilder"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t CONFIG_PARAM_NUM = 2;
constexpr size_t POLICY_PARAM_NUM = 1;
constexpr size_t INDEX_OF_NAME_CONFIG = 0;
constexpr size_t INDEX_OF_VALUE_CONFIG = 1;
constexpr const char* const APP_CRASH = "APP_CRASH";
constexpr const char* const MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
constexpr const char* const RESOURCE_OVERLIMIT = "RESOURCE_OVERLIMIT";
constexpr int CRASH_LOG_MAX_CAPACITY = 5 * 1024 * 1024;  // 5M
struct crashConfig {
    uint8_t type;
    std::function<bool(const napi_env, const napi_value, const std::string&, uint32_t&)> func;
};

std::map<std::string, std::map<std::string, napi_valuetype>> GetEventPolicyItem()
{
    std::map<std::string, std::map<std::string, napi_valuetype>> eventPolicyItem = {
        {"mainThreadJankPolicy", {
            {"logType", napi_number}, {"ignoreStartupTime", napi_number}, {"sampleInterval", napi_number},
            {"sampleCount", napi_number}, {"reportTimesPerApp", napi_number}, {"autoStopSampling", napi_boolean}
        }},
        {"cpuUsageHighPolicy", {
            {"foregroundLoadThreshold", napi_number}, {"backgroundLoadThreshold", napi_number},
            {"threadLoadThreshold", napi_number}, {"perfLogCaptureCount", napi_number},
            {"threadLoadInterval", napi_number}
        }},
        {"appCrashPolicy", {{"pageSwitchLogEnable", napi_boolean}}},
        {"appFreezePolicy", {{"pageSwitchLogEnable", napi_boolean}}},
        {"resourceOverlimitPolicy", {{"pageSwitchLogEnable", napi_boolean}}},
        {"addressSanitizerPolicy", {{"pageSwitchLogEnable", napi_boolean}}},
    };
    return eventPolicyItem;
}

bool GetCrashConfigBoolValue(const napi_env env, const napi_value configs, const std::string& key, uint32_t& out)
{
    napi_value value = NapiUtil::GetProperty(env, configs, key);
    if (!NapiUtil::IsBoolean(env, value)) {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value should be bool type.", key.c_str());
        return false;
    }

    out = NapiUtil::GetBoolean(env, value) == true ? 1 : 0;
    return true;
}

bool GetCrashConfigUIntValue(const napi_env env, const napi_value configs, const std::string& key, uint32_t& out)
{
    napi_value value = NapiUtil::GetProperty(env, configs, key);
    if (!NapiUtil::IsNumber(env, value)) {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value should be a number.", key.c_str());
        return false;
    }

    int64_t intValue = NapiUtil::GetInt64(env, value);
    if (intValue < 0 || intValue > CRASH_LOG_MAX_CAPACITY) {
        HILOG_ERROR(LOG_CORE, "Set crash config item(%{public}s) failed, the value is over range.", key.c_str());
        return false;
    }

    out = static_cast<uint32_t>(intValue);
    return true;
}
}

std::unique_ptr<EventConfigPack> NapiConfigBuilder::BuildEventConfig(const napi_env env, const napi_value params[],
    size_t len)
{
    if (len < CONFIG_PARAM_NUM) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("setEventParam"));
        return nullptr;
    }
    if (!NapiUtil::IsString(env, params[INDEX_OF_NAME_CONFIG])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("name", "string"));
        return nullptr;
    }
    if (!NapiUtil::IsObject(env, params[INDEX_OF_VALUE_CONFIG])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("value", "object"));
        return nullptr;
    }

    eventConfigPack_->eventName = NapiUtil::GetString(env, params[INDEX_OF_NAME_CONFIG]);
    std::unordered_set<std::string> whiteList = { MAIN_THREAD_JANK, RESOURCE_OVERLIMIT };
    if (eventConfigPack_->eventName == APP_CRASH) {
        GetAppCrashConfig(env, params[INDEX_OF_VALUE_CONFIG]);
    } else if (whiteList.count(eventConfigPack_->eventName) != 0) {
        GetCommonConfig(env, params[INDEX_OF_VALUE_CONFIG]);
    } else {
        HILOG_ERROR(LOG_CORE, "Failed to set event config, name is invalid. name=%{public}s",
            eventConfigPack_->eventName.c_str());
    }
    return std::move(eventConfigPack_);
}

void NapiConfigBuilder::GetAppCrashConfig(const napi_env env, const napi_value params)
{
    std::vector<std::string> configKeys;
    NapiUtil::GetPropertyNames(env, params, configKeys);
    if (configKeys.empty()) {
        HILOG_INFO(LOG_CORE, "Set config failed, the config is empty.");
        return;
    }

    std::map<std::string, crashConfig> crashConfigs = {
        {"extend_pc_lr_printing", {.type = 0, .func = GetCrashConfigBoolValue}},
        {"log_file_cutoff_sz_bytes", {.type = 1, .func = GetCrashConfigUIntValue}},
        {"simplify_vma_printing", {.type = 2, .func = GetCrashConfigBoolValue}}
    };

    uint32_t configValue = 0;
    for (const auto& configKey : configKeys) {
        auto it = crashConfigs.find(configKey);
        if (it == crashConfigs.end()) {
            HILOG_WARN(LOG_CORE, "Set crash config item(%{public}s) failed, it is not supported.", configKey.c_str());
            continue;
        }

        if ((it->second.func)(env, params, configKey, configValue)) {
            eventConfigPack_->configUintMap[it->second.type] = configValue;
            eventConfigPack_->isValid = true;
            HILOG_INFO(LOG_CORE, "Set crash config item(%{public}s) success. Value=%{public}d", configKey.c_str(),
                configValue);
        }
    }
}

void NapiConfigBuilder::GetCommonConfig(const napi_env env, const napi_value params)
{
    std::vector<std::string> configKeys;
    NapiUtil::GetPropertyNames(env, params, configKeys);
    if (configKeys.empty()) {
        HILOG_INFO(LOG_CORE, "Set MainThreadJank config failed, the config is empty.");
        return;
    }

    eventConfigPack_->isValid = true;
    std::map<std::string, std::string> configMap;
    for (const auto& configkey : configKeys) {
        napi_value configValue = NapiUtil::GetProperty(env, params, configkey);
        if (!NapiUtil::IsString(env, configValue)) {
            HILOG_ERROR(LOG_CORE, "Failed to get config Value, the value type of the item(%{public}s) is not string.",
                configkey.c_str());
            eventConfigPack_->isValid = false;
            return;
        }
        configMap[configkey] = NapiUtil::GetString(env, configValue);
    }
    eventConfigPack_->configStringMap = std::move(configMap);
}

std::unique_ptr<EventPolicyPack> NapiConfigBuilder::BuildEventPolicy(const napi_env env, const napi_value param)
{
    if (!NapiUtil::IsObject(env, param)) {
        HILOG_ERROR(LOG_CORE, "the policy type is invalid, it shuould be a EventPloicy");
        return std::move(eventPolicyPack_);
    }

    std::vector<std::string> policyNames;
    NapiUtil::GetPropertyNames(env, param, policyNames);
    auto eventPolicyItem = GetEventPolicyItem();
    for (const auto& policyName : policyNames) {
        if (eventPolicyItem.find(policyName) == eventPolicyItem.end()) {
            HILOG_INFO(LOG_CORE, "skip config event(%{public}s) policy , it is invalid.", policyName.c_str());
            continue;
        }
        napi_value policy = NapiUtil::GetProperty(env, param, policyName);
        if (!GetPolicyConfig(env, policyName, policy)) {
            return std::move(eventPolicyPack_);
        }
    }
    eventPolicyPack_->isValid = true;
    return std::move(eventPolicyPack_);
}

bool NapiConfigBuilder::GetPolicyConfig(const napi_env env, const std::string& name, const napi_value policy)
{
    std::vector<std::string> configKeys;
    NapiUtil::GetPropertyNames(env, policy, configKeys);
    std::map<std::string, std::string> configMap;
    auto curEventPolicy = GetEventPolicyItem().at(name);
    for (const auto& configKey : configKeys) {
        auto it = curEventPolicy.find(configKey);
        if (it == curEventPolicy.end()) {
            HILOG_INFO(LOG_CORE, "skip config event policy item(%{public}s), it is invalid.", configKey.c_str());
            continue;
        }
        napi_value configValue = NapiUtil::GetProperty(env, policy, configKey);
        if (NapiUtil::GetType(env, configValue) != it->second) {
            HILOG_ERROR(LOG_CORE, "Config event ploicy param(%{public}s) type is invalid", configKey.c_str());
            return false;
        }
        configMap[configKey] = NapiUtil::ConvertToString(env, configValue);
    }
    eventPolicyPack_->policyStringMaps[name] = std::move(configMap);
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS