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

#include <cctype>
#include <hilog/log.h>
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
constexpr size_t CAMEL_SNAKE_RESERVE_DIVISOR = 2;

/**
 * @brief Determines whether an underscore should be inserted before the uppercase
 *        letter at position i in a camelCase string.
 *
 * An underscore is inserted when any of the following conditions is met:
 * 1. The previous character is a lowercase letter (e.g., 'p' before 'L' in "HeapLog");
 * 2. The previous character is a digit (e.g., '8' before 'H' in "v8Heap");
 * 3. The previous character is uppercase and the next character is lowercase
 *    (e.g., 'P' before 'S' and 'e' after 'S' in "HTTPServer"; the underscore is
 *    inserted before the last uppercase in a consecutive uppercase run, yielding
 *    "http_server").
 *
 * @param s The camelCase string to convert.
 * @param i The position of the current character.
 * @return true if an underscore should be inserted before the current character; false otherwise.
 */
bool NeedUnderscoreBefore(const std::string& s, size_t i)
{
    if (i == 0 || !std::isupper(static_cast<unsigned char>(s[i]))) {
        return false;
    }
    unsigned char prev = static_cast<unsigned char>(s[i - 1]);
    unsigned char next = (i + 1 < s.size()) ? static_cast<unsigned char>(s[i + 1]) : '\0';
    return std::islower(prev) || std::isdigit(prev) || (std::isupper(prev) && std::islower(next));
}

std::string CamelToSnake(const std::string& camel)
{
    std::string snake;
    snake.reserve(camel.size() + camel.size() / CAMEL_SNAKE_RESERVE_DIVISOR);
    for (size_t i = 0; i < camel.size(); ++i) {
        if (NeedUnderscoreBefore(camel, i)) {
            snake += '_';
        }
        snake += static_cast<char>(std::tolower(static_cast<unsigned char>(camel[i])));
    }
    return snake;
}

int SetEventConfig(const std::map<std::string, std::string>& configMap)
{
    int rtn = ErrorCode::ERROR_UNKNOWN;
    std::string configDir = EventPolicyUtils::GetInstance().GetConfigDir(RAWHEAP);
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get rawheap config dir");
        return rtn;
    }
    std::map<std::string, std::string> validConfigMap;
    for (const auto& [key, value] : configMap) {
        if (key == "v8HeapLogEnable" || key == "jsvmHeapLogEnable") {
            validConfigMap.insert({CamelToSnake(key), value == "true" ? "1" : "0"});
            continue;
        }
        if (key == "js_heap_logtype" || key == "jsHeapLogtype") {
            if (value != "event" && value != "event_rawheap") {
                HILOG_WARN(LOG_CORE, "failed SetEventConfig, invalid key %{public}s, value: %{public}s",
                    key.c_str(), value.c_str());
                return rtn;
            }
            validConfigMap.insert({"js_heap_logtype", value});
            continue;
        }
        HILOG_WARN(LOG_CORE, "failed SetEventConfig, invalid key %{public}s, value: %{public}s",
            key.c_str(), value.c_str());
    }
    if (!validConfigMap.empty()) {
        rtn = EventPolicyUtils::GetInstance().SaveEventConfig(configDir, validConfigMap);
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