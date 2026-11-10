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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "app_event_processor.h"
#include "app_event_processor_proxy.h"
#include "hiappevent_base.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr int FIRST_EVENT_TYPE = 1;
constexpr int BOOL_VALUE_COUNT = 2;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t HALF_DIVISOR = 2;
constexpr size_t MAX_LEN_OF_NAME = 32;
constexpr int NUM_OF_EVENT_TYPES = 4;

class FuzzEventProcessor : public OHOS::HiviewDFX::HiAppEvent::AppEventProcessor {
public:
    int OnReport(int64_t processorSeq, const std::vector<OHOS::HiviewDFX::HiAppEvent::UserId>& userIds,
        const std::vector<OHOS::HiviewDFX::HiAppEvent::UserProperty>& userProperties,
        const std::vector<OHOS::HiviewDFX::HiAppEvent::AppEventInfo>& events) override
    {
        return 0;
    }

    int ValidateUserId(const OHOS::HiviewDFX::HiAppEvent::UserId& userId) override
    {
        return 0;
    }

    int ValidateUserProperty(const OHOS::HiviewDFX::HiAppEvent::UserProperty& userProperty) override
    {
        return 0;
    }

    int ValidateEvent(const OHOS::HiviewDFX::HiAppEvent::AppEventInfo& event) override
    {
        return 0;
    }
};
}

static std::string FS(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

static std::string FS2(const uint8_t* data, size_t size, size_t maxLen = FUZZ_NAME_MAX_LEN)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}

void FuzzAppEventProcessorProxy(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FS(data, size, MAX_LEN_OF_NAME);
    std::string domain = FS2(data, size, MAX_LEN_OF_NAME);
    OHOS::HiviewDFX::HiAppEvent::ReportConfig config;
    config.name = name;
    config.routeInfo = domain;
    config.appId = domain;
    config.triggerCond.row = data[0];
    config.triggerCond.timeout = data[1];
    config.configId = data[0];
    config.configName = name;
    OHOS::HiviewDFX::HiAppEvent::EventConfig eventConfig;
    eventConfig.domain = domain;
    eventConfig.name = name;
    eventConfig.isRealTime = data[0] % BOOL_VALUE_COUNT != 0;
    config.eventConfigs = {eventConfig};
    config.ToString();
    eventConfig.ToString();
    config.triggerCond.ToString();
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>(domain, name,
        data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    eventConfig.IsValidEvent(event);
    eventConfig.IsRealTimeEvent(event);
    OHOS::HiviewDFX::HiAppEvent::EventConfig emptyConfig;
    emptyConfig.IsValidEvent(event);
    emptyConfig.IsRealTimeEvent(event);
    emptyConfig.ToString();
    auto processor = std::make_shared<FuzzEventProcessor>();
    OHOS::HiviewDFX::HiAppEvent::AppEventProcessorProxy proxy(name, processor);
    proxy.SetReportConfig(config);
    proxy.GetReportConfig();
    proxy.GenerateHashCode();
    proxy.VerifyEvent(event);
    proxy.IsRealTimeEvent(event);
    proxy.OnTrigger(config.triggerCond);
    proxy.ProcessEvent(event);
    proxy.ProcessTimeout();
    proxy.ProcessStartup();
    proxy.ProcessBackground();
    proxy.SetSeq(data[0]);
    proxy.GetSeq();
    proxy.GetName();
    proxy.OnEvents({event});
}
