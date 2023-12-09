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

#include <iostream>

#include "app_event_processor_mgr.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
void PrintReportConfig(int64_t processorSeq)
{
    ReportConfig config;
    AppEventProcessorMgr::GetProcessorConfig(processorSeq, config);
    std::cout << ".name=" << config.name << std::endl;
    std::cout << ".debugMode=" << config.debugMode << std::endl;
    std::cout << ".routeInfo=" << config.routeInfo << std::endl;
    std::cout << ".triggerCond.row=" << config.triggerCond.row << std::endl;
    std::cout << ".triggerCond.timeout=" << config.triggerCond.timeout << std::endl;
    std::cout << ".triggerCond.onStartup=" << config.triggerCond.onStartup << std::endl;
    std::cout << ".triggerCond.onBackground=" << config.triggerCond.onBackground << std::endl;
    std::cout << ".userIdNames.size=" << config.userIdNames.size() << std::endl;
    std::cout << ".userPropertyNames.size=" << config.userPropertyNames.size() << std::endl;
    std::cout << ".eventConfigs.size=" << config.eventConfigs.size() << std::endl;
}

void PrintEvent(const AppEventInfo& event)
{
    std::cout << "AppEventInfo.domain=" << event.domain << std::endl;
    std::cout << "AppEventInfo.name=" << event.name << std::endl;
    std::cout << "AppEventInfo.eventType=" << event.eventType << std::endl;
    std::cout << "AppEventInfo.timestamp=" << event.timestamp << std::endl;
    std::cout << "AppEventInfo.params=" << event.params << std::endl;
}
}

int TestProcessor::OnReport(
    int64_t processorSeq,
    const std::vector<UserId>& userIds,
    const std::vector<UserProperty>& userProperties,
    const std::vector<AppEventInfo>& events)
{
    std::cout << "OnReport start" << std::endl;
    PrintReportConfig(processorSeq);
    for (const auto& event : events) {
        PrintEvent(event);
    }
    std::cout << "OnReport end" << std::endl;
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
