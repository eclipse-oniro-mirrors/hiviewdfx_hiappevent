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
#include <string>
#include <vector>
#include "hiappevent_base.h"
#include "app_event_store.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

void FuzzAppEventObserverDao(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string n = FS(data, size, FUZZ_KEY_MAX_LEN);
    auto& s = OHOS::HiviewDFX::AppEventStore::GetInstance();
    s.InitDbStore();
    OHOS::HiviewDFX::AppEventCacheCommon::Observer o = {n, 0, "{}"};
    s.InsertObserver(o);
    s.UpdateObserver(1, "{}");
    s.QueryObserverSeq(n);
    std::string filters;
    s.QueryObserverSeqAndFilters(n, 0, filters);
    std::vector<int64_t> seqs;
    s.QueryObserverSeqs(n, seqs);
    std::vector<OHOS::HiviewDFX::AppEventCacheCommon::Observer> obs;
    s.QueryWatchers(obs);
    s.DeleteObserver(1);
    s.DestroyDbStore();
}
