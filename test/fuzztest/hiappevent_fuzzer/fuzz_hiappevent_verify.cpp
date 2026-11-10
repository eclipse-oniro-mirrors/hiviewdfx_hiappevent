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

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t HALF_DIVISOR = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr int EVENT_TYPE_BOUND = 6;
constexpr int BATCH_REPORT_FACTOR = 10;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
constexpr size_t FUZZ_VALUE_MAX_LEN = 256;
}

static std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

static std::string FuzzStr2(const uint8_t* data, size_t size, size_t maxLen)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t half = size / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(data + half), std::min(size - half, maxLen));
}

static void FuzzVerifyIsValidValues(const std::string& name, const std::string& value,
    const uint8_t* data, size_t size)
{
    OHOS::HiviewDFX::IsValidDomain(name);
    OHOS::HiviewDFX::IsValidEventName(name);
    OHOS::HiviewDFX::IsValidWatcherName(name);
    OHOS::HiviewDFX::IsValidEventType(data[0] % EVENT_TYPE_BOUND);
    OHOS::HiviewDFX::IsValidProcessorName(name);
    OHOS::HiviewDFX::IsValidRouteInfo(name);
    OHOS::HiviewDFX::IsValidAppId(name);
    OHOS::HiviewDFX::IsValidPeriodReport(data[0]);
    OHOS::HiviewDFX::IsValidBatchReport(data[0] * BATCH_REPORT_FACTOR);
    OHOS::HiviewDFX::IsValidUserIdName(name);
    OHOS::HiviewDFX::IsValidUserIdValue(value);
    OHOS::HiviewDFX::IsValidUserPropName(name);
    OHOS::HiviewDFX::IsValidUserPropValue(value);
    OHOS::HiviewDFX::IsValidConfigId(data[0]);
    OHOS::HiviewDFX::IsValidCustomConfigsNum(size);
    OHOS::HiviewDFX::IsValidCustomConfig(name, value);
    OHOS::HiviewDFX::IsValidConfigNameLength(name);
    OHOS::HiviewDFX::IsApp();
}

static void FuzzVerifyAppEvents(const std::string& name, const uint8_t* data)
{
    int eventType = data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST;
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>(name, name, eventType);
    event->AddParam("p", static_cast<int>(data[0]));
    event->AddParam("crash", std::string("d"));
    OHOS::HiviewDFX::VerifyAppEvent(event);
    OHOS::HiviewDFX::VerifyCustomEventParams(event);
}

static void FuzzVerifyReportConfigs(const std::string& name, const uint8_t* data)
{
    OHOS::HiviewDFX::HiAppEvent::ReportConfig config;
    config.name = name;
    config.routeInfo = name;
    config.appId = name;
    config.triggerCond.row = data[0];
    config.triggerCond.timeout = data[1];
    OHOS::HiviewDFX::HiAppEvent::EventConfig eventConfig;
    eventConfig.domain = name;
    eventConfig.name = name;
    config.eventConfigs = { eventConfig };
    OHOS::HiviewDFX::IsValidEventConfig(eventConfig);
    OHOS::HiviewDFX::VerifyReportConfig(config);
}

void FuzzHiappeventVerify(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FuzzStr(data, size, FUZZ_NAME_MAX_LEN);
    std::string value = FuzzStr2(data, size, FUZZ_VALUE_MAX_LEN);
    FuzzVerifyIsValidValues(name, value, data, size);
    FuzzVerifyAppEvents(name, data);
    FuzzVerifyReportConfigs(name, data);
}
