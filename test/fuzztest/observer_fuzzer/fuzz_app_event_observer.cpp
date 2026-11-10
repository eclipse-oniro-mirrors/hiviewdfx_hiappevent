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
#include <unordered_set>
#include <vector>
#include <algorithm>

#include "app_event_observer.h"
#include "app_event_watcher.h"
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
constexpr int EVENT_SIZE_FACTOR = 100;
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

static OHOS::HiviewDFX::HiAppEvent::TriggerCondition FuzzTriggerCond(const uint8_t* data)
{
    OHOS::HiviewDFX::HiAppEvent::TriggerCondition cond;
    cond.row = data[0];
    cond.size = data[0] * EVENT_SIZE_FACTOR;
    cond.timeout = data[1];
    cond.onStartup = data[0] % BOOL_VALUE_COUNT != 0;
    cond.onBackground = data[1] % BOOL_VALUE_COUNT != 0;
    return cond;
}

void FuzzAppEventObserver(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FS(data, size, MAX_LEN_OF_NAME);
    std::string domain = FS2(data, size, MAX_LEN_OF_NAME);
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>(domain, name,
        data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    event->AddParam("k", static_cast<int>(data[0]));
    OHOS::HiviewDFX::HiAppEvent::AppEventFilter filter1(domain, std::unordered_set<std::string>{name},
        data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    OHOS::HiviewDFX::HiAppEvent::AppEventFilter filter2(domain, data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    OHOS::HiviewDFX::HiAppEvent::AppEventFilter filter3;
    filter1.IsValidEvent(event);
    filter1.IsValidEvent(domain, name, data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    filter1.GetOsEventsMask();
    filter2.IsValidEvent(event);
    filter2.GetOsEventsMask();
    filter3.IsValidEvent(event);
    filter3.IsValidEvent("", "", 0);
    filter3.GetOsEventsMask();
    OHOS::HiviewDFX::HiAppEvent::TriggerCondition cond = FuzzTriggerCond(data);
    OHOS::HiviewDFX::AppEventWatcher watcher(name,
        std::vector<OHOS::HiviewDFX::HiAppEvent::AppEventFilter>{filter1, filter2}, cond);
    watcher.OnEvents({event});
    watcher.VerifyEvent(event);
    watcher.IsRealTimeEvent(event);
    watcher.ProcessEvent(event);
    watcher.ProcessTimeout();
    watcher.ProcessStartup();
    watcher.ProcessBackground();
    watcher.HasTimeoutCondition();
    watcher.GetName();
    watcher.GetSeq();
    watcher.SetSeq(data[0]);
    watcher.SetCurrCondition(cond);
    watcher.ResetCurrCondition();
    watcher.SetTriggerCond(cond);
    watcher.GetFilters();
    watcher.SetFilters({filter1, filter3});
    watcher.AddFilter(filter2);
    watcher.GetOsEventsMask();
    watcher.SetFiltersStr(R"([{"domain":"OS","names":["APP_CRASH"],"types":1}])");
    watcher.SetFiltersStr("");
    watcher.SetFiltersStr("invalid");
    watcher.GetFiltersStr();
}
