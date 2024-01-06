/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
const HiLogLabel LABEL = { LOG_CORE, 0xD002D07, "test_processor" };

void PrintReportConfig(int64_t processorSeq)
{
    ReportConfig config;
    AppEventProcessorMgr::GetProcessorConfig(processorSeq, config);
    HiLog::Info(LABEL, ".name=%{public}s", config.name.c_str());
    HiLog::Info(LABEL, ".debugMode=%{public}d", config.debugMode);
    HiLog::Info(LABEL, ".routeInfo=%{public}s", config.routeInfo.c_str());
    HiLog::Info(LABEL, ".appId=%{public}s", config.appId.c_str());
    HiLog::Info(LABEL, ".triggerCond.row=%{public}d", config.triggerCond.row);
    HiLog::Info(LABEL, ".triggerCond.size=%{public}d", config.triggerCond.size);
    HiLog::Info(LABEL, ".triggerCond.timeout=%{public}d", config.triggerCond.timeout);
    HiLog::Info(LABEL, ".triggerCond.onStartup=%{public}d", config.triggerCond.onStartup);
    HiLog::Info(LABEL, ".triggerCond.onBackground=%{public}d", config.triggerCond.onBackground);
    HiLog::Info(LABEL, ".userIdNames.size=%{public}zu", config.userIdNames.size());
    HiLog::Info(LABEL, ".userPropertyNames.size=%{public}zu", config.userPropertyNames.size());
    HiLog::Info(LABEL, ".eventConfigs.size=%{public}zu", config.eventConfigs.size());
}

void PrintEvent(const AppEventInfo& event)
{
    HiLog::Info(LABEL, "AppEventInfo.domain=%{public}s", event.domain.c_str());
    HiLog::Info(LABEL, "AppEventInfo.name=%{public}s", event.name.c_str());
    HiLog::Info(LABEL, "AppEventInfo.eventType=%{public}d", event.eventType);
    HiLog::Info(LABEL, "AppEventInfo.timestamp=%{public}" PRId64, event.timestamp);
    HiLog::Info(LABEL, "AppEventInfo.params=%{public}s", event.params.c_str());
}
}

int TestProcessor::OnReport(
    int64_t processorSeq,
    const std::vector<UserId>& userIds,
    const std::vector<UserProperty>& userProperties,
    const std::vector<AppEventInfo>& events)
{
    HiLog::Info(LABEL, "OnReport start");
    PrintReportConfig(processorSeq);
    for (const auto& event : events) {
        PrintEvent(event);
    }
    HiLog::Info(LABEL, "OnReport end");
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
