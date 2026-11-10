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

void FuzzAppEventWatcher(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FS(data, size, MAX_LEN_OF_NAME);
    std::string domain = FS2(data, size, MAX_LEN_OF_NAME);
    OHOS::HiviewDFX::HiAppEvent::AppEventFilter filter(domain, std::unordered_set<std::string>{name},
        data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    OHOS::HiviewDFX::HiAppEvent::TriggerCondition cond;
    cond.row = data[0];
    cond.size = data[0] * EVENT_SIZE_FACTOR;
    cond.timeout = data[1];
    cond.onStartup = data[0] % BOOL_VALUE_COUNT != 0;
    cond.onBackground = data[1] % BOOL_VALUE_COUNT != 0;
    OHOS::HiviewDFX::AppEventWatcher watcher(name,
        std::vector<OHOS::HiviewDFX::HiAppEvent::AppEventFilter>{filter}, cond);
    watcher.GetOsEventsMask();
    watcher.GetFiltersStr();
    watcher.SetFiltersStr(R"([{"domain":"OS","names":["APP_CRASH"],"types":1}])");
    watcher.SetFiltersStr("");
    watcher.SetFiltersStr("invalid");
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>(domain, name,
        data[0] % NUM_OF_EVENT_TYPES + FIRST_EVENT_TYPE);
    watcher.VerifyEvent(event);
    watcher.ProcessEvent(event);
    watcher.ProcessTimeout();
    watcher.HasTimeoutCondition();
    watcher.ResetCurrCondition();
    watcher.ProcessStartup();
    watcher.ProcessBackground();
    watcher.SetSeq(data[0]);
    watcher.GetSeq();
    watcher.GetName();
    watcher.OnEvents({event});
    watcher.IsRealTimeEvent(event);
    OHOS::HiviewDFX::AppEventWatcher watcher2(name);
    watcher2.GetOsEventsMask();
    watcher2.GetFiltersStr();
}
