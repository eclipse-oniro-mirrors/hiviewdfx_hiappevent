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
#include "main_thread_jank_policy.h"

#include "event_policy_utils.h"
#include "xcollie/watchdog.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "MainThreadJankPolicy"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int INVALID_PARAM = -1;

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
}

int MainThreadJankPolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    auto unifiedConfigMap = UnifiedCAndTsParamNames(configMap);
    return Watchdog::GetInstance().ConfigEventPolicy(unifiedConfigMap);
}

int MainThreadJankPolicy::SetEventPolicy(const std::map<uint8_t, uint32_t>& configMap)
{
    return INVALID_PARAM;
}

int MainThreadJankConfig::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    return Watchdog::GetInstance().SetEventConfig(configMap);
}

int MainThreadJankConfig::SetEventPolicy(const std::map<uint8_t, uint32_t>& configMap)
{
    return INVALID_PARAM;
}
}  // HiviewDFX
}  // OHOS
