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
#include "hiappevent_base.h"
#include "app_event_store.h"
#include "fuzz_cases.h"

namespace {
constexpr int64_t TEST_OBSERVER_SEQ = 1;
constexpr uint32_t QUERY_EVENT_NUM = 100;
constexpr int RESERVED_EVENT_NUM = 1000;
constexpr int RESERVED_OS_EVENT_NUM = 150;
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

void FuzzAppEventDao(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string d = FS(data, size, FUZZ_KEY_MAX_LEN);
    auto& s = OHOS::HiviewDFX::AppEventStore::GetInstance();
    s.InitDbStore();
    auto e = std::make_shared<OHOS::HiviewDFX::AppEventPack>(d, d, data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST);
    s.InsertEvent(e);
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> events;
    s.QueryEvents(events, 1);
    s.TakeEvents(events, TEST_OBSERVER_SEQ, QUERY_EVENT_NUM);
    s.DeleteEvent(1);
    s.DeleteEvent(std::vector<int64_t>{1});
    s.DeleteHistoryEvent(RESERVED_EVENT_NUM, RESERVED_OS_EVENT_NUM);
    s.DestroyDbStore();
}
