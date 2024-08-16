/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cinttypes>

#include "ndk_app_event_processor.h"

#include "app_event_observer_mgr.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NdkProcessor"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int CODE_SUCC = 0;
constexpr int CODE_FAILED = -1;
}

NdkAppEventProcessor::NdkAppEventProcessor(const std::string &name)
{
    config_.name = name;
}

int NdkAppEventProcessor::SetReportRoute(const char* appId, const char* routeInfo)
{
    config_.appId = appId;
    config_.routeInfo = routeInfo;
    return CODE_SUCC;
}

int NdkAppEventProcessor::SetReportPolicy(int periodReport, int batchReport, bool onStartReport,
    bool onBackgroundReport)
{
    config_.triggerCond.timeout = periodReport;
    config_.triggerCond.row = batchReport;
    config_.triggerCond.onStartup = onStartReport;
    config_.triggerCond.onBackground = onBackgroundReport;
    return CODE_SUCC;
}

int NdkAppEventProcessor::SetReportEvent(const char* domain, const char* name, bool isRealTime)
{
    EventConfig event;
    event.domain = domain;
    event.name = name;
    event.isRealTime = isRealTime;
    config_.eventConfigs.emplace_back(event);
    return CODE_SUCC;
}

int NdkAppEventProcessor::SetCustomConfig(const char* key, const char* value)
{
    auto it = config_.customConfigs.find(key);
    if (it != config_.customConfigs.end()) {
        config_.customConfigs.erase(it);
    }
    config_.customConfigs.insert(std::pair<std::string, std::string>(key, value));
    return CODE_SUCC;
}

int NdkAppEventProcessor::SetConfigId(int configId)
{
    config_.configId = configId;
    return CODE_SUCC;
}

int NdkAppEventProcessor::SetReportUserId(const char* const * userIdNames, int size)
{
    for (int i = 0; i < size; ++i) {
        config_.userIdNames.emplace(userIdNames[i]);
    }
    return CODE_SUCC;
}

int NdkAppEventProcessor::SetReportUserProperty(const char* const * userPropertyNames, int size)
{
    for (int i = 0; i < size; ++i) {
        config_.userPropertyNames.emplace(userPropertyNames[i]);
    }
    return CODE_SUCC;
}

HiAppEvent::ReportConfig NdkAppEventProcessor::GetConfig()
{
    return config_;
}
}
}