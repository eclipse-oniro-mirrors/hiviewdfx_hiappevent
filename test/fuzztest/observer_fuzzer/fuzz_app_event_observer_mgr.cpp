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

#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr int FIRST_EVENT_TYPE = 1;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t MAX_LEN_OF_NAME = 32;
constexpr int NUM_OF_EVENT_TYPES = 4;
constexpr int64_t DEFAULT_SEQ = 1;
constexpr int64_t INVALID_SEQ = 999;
}

static std::string FS(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

void FuzzAppEventObserverMgr(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FS(data, size, MAX_LEN_OF_NAME);
    auto& mgr = OHOS::HiviewDFX::AppEventObserverMgr::GetInstance();
    mgr.HandleBackground();
    mgr.HandleClearUp();
    mgr.HandleTimeout();
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>("d", name,
        data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    event->AddParam("k", static_cast<int>(data[0]));
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> events = {event};
    mgr.HandleEvents(events);
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> emptyEvents;
    mgr.HandleEvents(emptyEvents);
    mgr.StoreEventsWithoutNotify(events);
    mgr.StoreEventsWithoutNotify(emptyEvents);
    mgr.GetObservers();
    mgr.RemoveObserver(name);
    mgr.RemoveObserver(INVALID_SEQ);
    OHOS::HiviewDFX::HiAppEvent::ReportConfig config;
    config.name = name;
    mgr.SetReportConfig(DEFAULT_SEQ, config);
    OHOS::HiviewDFX::HiAppEvent::ReportConfig outConfig;
    mgr.GetReportConfig(DEFAULT_SEQ, outConfig);
    mgr.GetReportConfig(INVALID_SEQ, outConfig);
    mgr.SubmitTaskToFFRTQueue([]() {}, "fuzz");
}
