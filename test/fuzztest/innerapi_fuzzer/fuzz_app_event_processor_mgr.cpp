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
#include <string>
#include <vector>

#include "app_event_processor_mgr.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t PROCESSOR_NAME_MAX_LEN = 64;

std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen = PROCESSOR_NAME_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}
}

void FuzzAppEventProcessorMgr(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FuzzStr(data, size);
    OHOS::HiviewDFX::HiAppEvent::ReportConfig config;
    config.name = name;
    config.routeInfo = name;
    config.appId = name;
    config.triggerCond.row = data[0];
    config.triggerCond.timeout = data[1];
    int64_t processorSeq = OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::AddProcessor(config);
    if (processorSeq > 0) {
        OHOS::HiviewDFX::HiAppEvent::ReportConfig outConfig;
        OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::GetProcessorConfig(processorSeq, outConfig);
        OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::SetProcessorConfig(processorSeq, config);
        std::vector<int64_t> processorSeqs;
        OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::GetProcessorSeqs(name, processorSeqs);
        OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::RemoveProcessor(processorSeq);
    }
    OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::AddProcessorAsync(config,
        [](int64_t processorId) {
            OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::RemoveProcessor(processorId);
        });
    OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::RegisterProcessor(name, nullptr);
    OHOS::HiviewDFX::HiAppEvent::AppEventProcessorMgr::UnregisterProcessor(name);
}
