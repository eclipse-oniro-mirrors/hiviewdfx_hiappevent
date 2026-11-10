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
#include "hiappevent_base.h"
#include "app_event_db_cleaner.h"
#include "app_event_store.h"
#include "fuzz_cases.h"

namespace {
constexpr uint64_t BYTES_PER_KB = 1024;
constexpr int RESERVED_EVENT_NUM = 100;
constexpr int RESERVED_OS_EVENT_NUM = 150;
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr int SECOND_DATA_INDEX = 1;
}

void FuzzAppEventDbCleaner(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    OHOS::HiviewDFX::AppEventDbCleaner c("/data/test/fuzz/hiappevent/");
    c.GetFilesSize();
    c.ClearSpace(static_cast<uint64_t>(data[0]) * BYTES_PER_KB,
        static_cast<uint64_t>(data[SECOND_DATA_INDEX % size]) * BYTES_PER_KB);
    {
        auto& store = OHOS::HiviewDFX::AppEventStore::GetInstance();
        store.InitDbStore();
        auto ev = std::make_shared<OHOS::HiviewDFX::AppEventPack>("dm", "nm", 1);
        store.InsertEvent(ev);
        store.DeleteHistoryEvent(RESERVED_EVENT_NUM, RESERVED_OS_EVENT_NUM);
    }
    c.ClearData();
}
