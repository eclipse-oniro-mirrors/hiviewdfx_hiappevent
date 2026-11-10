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

#include "app_event_util.h"
#include "hiappevent_base.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr int FUZZ_EVENT_TYPE = 1;
}

void FuzzAppEventUtil(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string bundleName;
    std::string appVersion;
    std::string runningId;
    OHOS::HiviewDFX::AppEventUtil::GetApplicationInfo(bundleName, appVersion, runningId);
    OHOS::HiviewDFX::AppEventUtil::IsBetaVersion();
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>("d", "n", FUZZ_EVENT_TYPE);
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> events = { event };
    OHOS::HiviewDFX::AppEventUtil::ReportAppEventReceive(events, "wn", "cb");
    std::vector<std::string> linkExternalLogs = { "p1", "p2" };
    OHOS::HiviewDFX::AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);
}
