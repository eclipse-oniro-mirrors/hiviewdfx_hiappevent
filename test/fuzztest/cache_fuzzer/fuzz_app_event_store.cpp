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
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "hiappevent_base.h"
#include "app_event_store.h"
#include "fuzz_cases.h"

namespace {
constexpr int64_t TEST_OBSERVER_SEQ = 1;
constexpr uint32_t QUERY_EVENT_NUM = 100;
constexpr int RESERVED_EVENT_NUM = 100;
constexpr int RESERVED_OS_EVENT_NUM = 150;
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t FUZZ_NAME_MAX_LEN = 48;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t HALF_DIVISOR = 2;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

static std::string FS2(const uint8_t* d, size_t s, size_t m = FUZZ_NAME_MAX_LEN)
{
    if (s < MIN_FUZZ_DATA_SIZE) {
        return "x";
    }
    size_t h = s / HALF_DIVISOR;
    return std::string(reinterpret_cast<const char*>(d + h), std::min(s - h, m));
}

static void FuzzStoreWrite(const uint8_t* data, size_t size,
    const std::shared_ptr<OHOS::HiviewDFX::AppEventPack>& e, int64_t& seq, int64_t& os)
{
    std::string d = FS(data, size, FUZZ_KEY_MAX_LEN);
    std::string n = FS2(data, size, FUZZ_NAME_MAX_LEN);
    e->AddParam("k", static_cast<int>(data[0]));
    auto& s = OHOS::HiviewDFX::AppEventStore::GetInstance();
    s.InitDbStore();
    seq = s.InsertEvent(e);
    if (seq > 0) {
        e->SetSeq(seq);
    }
    OHOS::HiviewDFX::AppEventCacheCommon::Observer obs("fuzz", data[0]);
    os = s.InsertObserver(obs);
    std::vector<OHOS::HiviewDFX::AppEventCacheCommon::EventObserverInfo> eis = {
        {TEST_OBSERVER_SEQ, TEST_OBSERVER_SEQ}};
    s.InsertEventMapping(eis);
    s.InsertUserId("u1", "v1");
    s.InsertUserProperty("p1", "v1");
    s.InsertApiMetricInfo("kit", "api", "{}");
    std::unordered_map<std::string, std::string> customParams = {{"ck1", d + "_v"}, {"ck2", "v"}};
    e->AddCustomParams(customParams);
    s.InsertCustomEventParams(e);
    s.UpdateUserId("u1", "v2");
    s.UpdateUserProperty("p1", "v2");
    if (os > 0) {
        s.UpdateObserver(os, "f");
    }
}

static void FuzzStoreQueryAndDelete(const uint8_t* data,
    const std::shared_ptr<OHOS::HiviewDFX::AppEventPack>& e, int64_t seq, int64_t os)
{
    auto& s = OHOS::HiviewDFX::AppEventStore::GetInstance();
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> qe;
    s.QueryEvents(qe, TEST_OBSERVER_SEQ);
    s.TakeEvents(qe, TEST_OBSERVER_SEQ, QUERY_EVENT_NUM);
    std::string flt;
    s.QueryObserverSeqAndFilters("fuzz", data[0], flt);
    s.QueryObserverSeq("fuzz", data[0]);
    std::vector<int64_t> osq;
    s.QueryObserverSeqs("fuzz", osq);
    std::vector<OHOS::HiviewDFX::AppEventCacheCommon::Observer> ws;
    s.QueryWatchers(ws);
    std::unordered_map<std::string, std::string> uo;
    s.QueryUserIds(uo);
    std::string uv;
    s.QueryUserId("u1", uv);
    std::unordered_map<std::string, std::string> po;
    s.QueryUserProperties(po);
    std::string pv;
    s.QueryUserProperty("p1", pv);
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> mo;
    s.QueryApiMetricInfoAll(mo);
    s.QueryCustomParamsAdd2EventPack(e);
    std::vector<int64_t> ese = {seq};
    s.DeleteEvent(seq);
    s.DeleteEvent(ese);
    s.DeleteObserver(os);
    s.DeleteEventMapping(os, ese);
    s.DeleteUserId("u1");
    s.DeleteUserProperty("p1");
    s.DeleteCustomEventParams();
    s.ClearApiMetricInfo();
    s.DeleteUnusedParamsExceptCurId("rid");
    s.DeleteUnusedEventMapping();
    s.DeleteHistoryEvent(RESERVED_EVENT_NUM, RESERVED_OS_EVENT_NUM);
    s.DeleteData(os, ese);
    s.DestroyDbStore();
}

void FuzzAppEventStore(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string d = FS(data, size, FUZZ_KEY_MAX_LEN);
    std::string n = FS2(data, size, FUZZ_NAME_MAX_LEN);
    auto e = std::make_shared<OHOS::HiviewDFX::AppEventPack>(d, n, data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST);
    int64_t seq = 0;
    int64_t os = 0;
    FuzzStoreWrite(data, size, e, seq, os);
    FuzzStoreQueryAndDelete(data, e, seq, os);
}
