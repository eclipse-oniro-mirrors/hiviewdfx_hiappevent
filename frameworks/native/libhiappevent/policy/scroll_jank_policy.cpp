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
#include "scroll_jank_policy.h"
 
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <hilog/log.h>
 
#include "hiappevent_base.h"
#include "perf_monitor_adapter.h"
#include "xcollie/watchdog.h"
 
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
 
#undef LOG_TAG
#define LOG_TAG "ScrollJankPolicy"
 
namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int INVALID_PARAM = -1;
constexpr int32_t MIN_SCROLL_JANK_THRESHOLD = 33;
constexpr int32_t MAX_SCROLL_JANK_THRESHOLD = 500;
 
bool ChangeConfigToIntValue(const std::string& strValue, int& out)
{
    const int decimal = 10;
    char* end = nullptr;
    errno = 0;
    long result = strtol(strValue.c_str(), &end, decimal);
    if (strValue.empty() || *end != '\0' || errno == ERANGE ||
        result < MIN_SCROLL_JANK_THRESHOLD || result > MAX_SCROLL_JANK_THRESHOLD) {
        HILOG_ERROR(LOG_CORE, "Set scroll jank config failed, the value(%{public}s) is invalid.", strValue.c_str());
        return false;
    }
    out = static_cast<int>(result);
    return true;
}
}
 
int ScrollJankPolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    auto it = configMap.find("scrollJankThreshold");
    if (it == configMap.end()) {
        it = configMap.find("scroll_jank_threshold");
    }
    if (it == configMap.end()) {
        HILOG_ERROR(LOG_CORE, "Set scroll jank config failed, scrollJankThreshold is not set.");
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    int threshold = 0;
    if (!ChangeConfigToIntValue(it->second, threshold)) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    PerfMonitorAdapter::GetInstance().SetScrollJankThreshold(threshold);
    Watchdog::GetInstance().SetScrollSampleInterval(threshold);
    HILOG_INFO(LOG_CORE, "Set scroll jank threshold success, threshold: %{public}d.", threshold);
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}
 
int ScrollJankPolicy::SetEventPolicy(const std::map<uint8_t, uint32_t>& configMap)
{
    return INVALID_PARAM;
}
}  // HiviewDFX
}  // OHOS