/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "test_processor.h"

#include <cinttypes>

#include "app_event_processor_mgr.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "TestProcessor"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
void PrintReportConfig(int64_t processorSeq)
{
    ReportConfig config;
    AppEventProcessorMgr::GetProcessorConfig(processorSeq, config);
    HILOG_INFO(LOG_CORE, ".name=%{public}s", config.name.c_str());
    HILOG_INFO(LOG_CORE, ".debugMode=%{public}d", config.debugMode);
    HILOG_INFO(LOG_CORE, ".routeInfo=%{public}s", config.routeInfo.c_str());
    HILOG_INFO(LOG_CORE, ".appId=%{public}s", config.appId.c_str());
    HILOG_INFO(LOG_CORE, ".triggerCond.row=%{public}d", config.triggerCond.row);
    HILOG_INFO(LOG_CORE, ".triggerCond.size=%{public}d", config.triggerCond.size);
    HILOG_INFO(LOG_CORE, ".triggerCond.timeout=%{public}d", config.triggerCond.timeout);
    HILOG_INFO(LOG_CORE, ".triggerCond.onStartup=%{public}d", config.triggerCond.onStartup);
    HILOG_INFO(LOG_CORE, ".triggerCond.onBackground=%{public}d", config.triggerCond.onBackground);
    HILOG_INFO(LOG_CORE, ".userIdNames.size=%{public}zu", config.userIdNames.size());
    HILOG_INFO(LOG_CORE, ".userPropertyNames.size=%{public}zu", config.userPropertyNames.size());
    HILOG_INFO(LOG_CORE, ".eventConfigs.size=%{public}zu", config.eventConfigs.size());
    HILOG_INFO(LOG_CORE, ".configId=%{public}d", config.configId);
    HILOG_INFO(LOG_CORE, ".customConfigs.size=%{public}zu", config.customConfigs.size());
}

void PrintEvent(const AppEventInfo& event)
{
    HILOG_INFO(LOG_CORE, "AppEventInfo.domain=%{public}s", event.domain.c_str());
    HILOG_INFO(LOG_CORE, "AppEventInfo.name=%{public}s", event.name.c_str());
    HILOG_INFO(LOG_CORE, "AppEventInfo.eventType=%{public}d", event.eventType);
    HILOG_INFO(LOG_CORE, "AppEventInfo.timestamp=%{public}" PRId64, event.timestamp);
    HILOG_INFO(LOG_CORE, "AppEventInfo.params=%{public}s", event.params.c_str());
}
}

int TestProcessor::OnReport(
    int64_t processorSeq,
    const std::vector<UserId>& userIds,
    const std::vector<UserProperty>& userProperties,
    const std::vector<AppEventInfo>& events)
{
    HILOG_INFO(LOG_CORE, "OnReport start");
    PrintReportConfig(processorSeq);
    for (const auto& event : events) {
        PrintEvent(event);
    }
    HILOG_INFO(LOG_CORE, "OnReport end");
    return 0;
}

int TestProcessor::ValidateUserId(const UserId& userId)
{
    return 0;
}

int TestProcessor::ValidateUserProperty(const UserProperty& userProperty)
{
    return 0;
}

int TestProcessor::ValidateEvent(const AppEventInfo& event)
{
    return 0;
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
