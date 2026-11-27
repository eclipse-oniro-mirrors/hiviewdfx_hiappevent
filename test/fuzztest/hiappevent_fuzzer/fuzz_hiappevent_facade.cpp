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
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "hiappevent_base.h"
#include "hiappevent_facade.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 2;
constexpr size_t HALF_DIVISOR = 2;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_FIRST = 1;
constexpr int RESULT_MOD = 2;
constexpr int BATCH_REPORT_FACTOR = 10;
constexpr int64_t OBSERVER_SEQ = 1;
constexpr int64_t INVALID_OBSERVER_SEQ = 999;
constexpr uint32_t QUERY_EVENT_NUM = 100;
constexpr uint64_t FUZZ_MAX_STORAGE_SIZE = 1024;
constexpr size_t FUZZ_STR_MAX_LEN = 32;
constexpr size_t FUZZ_VALUE_MAX_LEN = 256;
constexpr const char* const FUZZ_STORAGE_DIR = "/data/test/fuzz/hiappevent/";
constexpr const char* const FUZZ_TEST_FILE = "/data/test/fuzz/hiappevent/f";
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

static void FuzzFacadeConfigWrite(const std::string& name, const std::string& value, const uint8_t* data,
    std::shared_ptr<OHOS::HiviewDFX::AppEventPack>& event, OHOS::HiviewDFX::HiAppEvent::ReportConfig& config)
{
    OHOS::HiviewDFX::AppEventConfigFacade::SetConfigurationItem(name, value);
    OHOS::HiviewDFX::AppEventConfigFacade::SetStorageDir(FUZZ_STORAGE_DIR);
    OHOS::HiviewDFX::AppEventConfigFacade::GetDisable();
    OHOS::HiviewDFX::AppEventConfigFacade::GetMaxStorageSize();
    OHOS::HiviewDFX::AppEventConfigFacade::GetStorageDir();
    OHOS::HiviewDFX::AppEventConfigFacade::GetRunningId();
    OHOS::HiviewDFX::AppEventConfigFacade::RefreshFreeSize();
    OHOS::HiviewDFX::AppEventWriteFacade::FacadeWriteEvent(event);
    OHOS::HiviewDFX::AppEventWriteFacade::FacadeSetEventParam(event);
    std::map<std::string, std::string> strConfigMap = { { name, value } };
    OHOS::HiviewDFX::AppEventWriteFacade::SetEventPolicy(name, strConfigMap);
    std::map<uint8_t, uint32_t> intConfigMap = { { data[0], data[1] } };
    OHOS::HiviewDFX::AppEventWriteFacade::SetEventPolicy(name, intConfigMap);
    config.name = name;
    OHOS::HiviewDFX::AppEventObserverFacade::AddProcessor(name, config);
    OHOS::HiviewDFX::AppEventObserverFacade::RemoveObserver(name);
    OHOS::HiviewDFX::AppEventObserverFacade::RemoveObserver(INVALID_OBSERVER_SEQ);
    OHOS::HiviewDFX::AppEventObserverFacade::Load(name);
}

static void FuzzFacadeObserverUserInfo(const std::string& name, const std::string& value,
    std::shared_ptr<OHOS::HiviewDFX::AppEventPack>& event, OHOS::HiviewDFX::HiAppEvent::ReportConfig& config)
{
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> events = { event };
    OHOS::HiviewDFX::AppEventObserverFacade::HandleEvents(events);
    OHOS::HiviewDFX::AppEventObserverFacade::HandleTimeout();
    OHOS::HiviewDFX::AppEventObserverFacade::HandleBackground();
    OHOS::HiviewDFX::AppEventObserverFacade::SubmitTaskToFFRTQueue([]() {}, "fuzz");
    OHOS::HiviewDFX::AppEventObserverFacade::UnregisterProcessor(name);
    OHOS::HiviewDFX::AppEventObserverFacade::SetReportConfig(OBSERVER_SEQ, config);
    OHOS::HiviewDFX::HiAppEvent::ReportConfig outConfig;
    OHOS::HiviewDFX::AppEventObserverFacade::GetReportConfig(OBSERVER_SEQ, outConfig);
    OHOS::HiviewDFX::AppEventUserInfoFacade::SetUserId(name, value);
    std::string userIdOut;
    OHOS::HiviewDFX::AppEventUserInfoFacade::GetUserId(name, userIdOut);
    OHOS::HiviewDFX::AppEventUserInfoFacade::RemoveUserId(name);
    OHOS::HiviewDFX::AppEventUserInfoFacade::SetUserProperty(name, value);
    OHOS::HiviewDFX::AppEventUserInfoFacade::GetUserProperty(name, userIdOut);
    OHOS::HiviewDFX::AppEventUserInfoFacade::RemoveUserProperty(name);
    OHOS::HiviewDFX::AppEventUserInfoFacade::GetUserIdVersion();
    OHOS::HiviewDFX::AppEventUserInfoFacade::GetUserPropertyVersion();
}

static void FuzzFacadeStoreVerify(const std::string& name, const uint8_t* data,
    std::shared_ptr<OHOS::HiviewDFX::AppEventPack>& event, OHOS::HiviewDFX::HiAppEvent::ReportConfig& config)
{
    OHOS::HiviewDFX::AppEventStoreFacade::InitDbStore();
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> queryEvents;
    OHOS::HiviewDFX::AppEventStoreFacade::QueryEvents(queryEvents, OBSERVER_SEQ);
    OHOS::HiviewDFX::AppEventStoreFacade::TakeEvents(queryEvents, OBSERVER_SEQ, QUERY_EVENT_NUM);
    OHOS::HiviewDFX::AppEventStoreFacade::QueryObserverSeq(name);
    std::vector<int64_t> observerSeqs;
    OHOS::HiviewDFX::AppEventStoreFacade::QueryObserverSeqs(name, observerSeqs);
    std::vector<int64_t> eventSeqs = { OBSERVER_SEQ };
    OHOS::HiviewDFX::AppEventStoreFacade::DeleteData(OBSERVER_SEQ, eventSeqs);
    OHOS::HiviewDFX::AppEventStoreFacade::DeleteEventMapping(OBSERVER_SEQ, eventSeqs);
    OHOS::HiviewDFX::AppEventStoreFacade::CheckStorageSpace();
    OHOS::HiviewDFX::AppEventStoreFacade::ClearData(FUZZ_STORAGE_DIR);
    OHOS::HiviewDFX::AppEventStoreFacade::IsStorageSpaceFull(FUZZ_STORAGE_DIR, FUZZ_MAX_STORAGE_SIZE);
    OHOS::HiviewDFX::AppEventStoreFacade::ReleaseSomeStorageSpace(FUZZ_STORAGE_DIR, FUZZ_MAX_STORAGE_SIZE);
    OHOS::HiviewDFX::AppEventStoreFacade::DestroyDbStore();
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyTheAppEvent(event);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyTheCustomEventParams(event);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyTheReportConfig(config);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsApp();
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidAppId(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidBatchReport(data[0] * BATCH_REPORT_FACTOR);
}

static void FuzzFacadeVerifyUtility(const std::string& name, const std::string& value, const uint8_t* data)
{
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidConfigId(data[0]);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidConfigNameLength(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidCustomConfig(name, value);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidCustomConfigsNum(0);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidDomain(name);
    OHOS::HiviewDFX::HiAppEvent::EventConfig eventConfig;
    eventConfig.domain = name;
    eventConfig.name = name;
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidEventConfig(eventConfig);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidEventName(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidEventType(data[0] % EVENT_TYPE_COUNT);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidPeriodReport(data[0]);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidProcessorName(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidRouteInfo(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidUserIdName(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidUserIdValue(value);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidUserPropName(name);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidUserPropValue(value);
    OHOS::HiviewDFX::AppEventVerifyFacade::VerifyIsValidWatcherName(name);
    OHOS::HiviewDFX::AppEventUtilityFacade::WriteApiEndEventAsync(name, data[0], data[0] % RESULT_MOD, data[1],
        OHOS::HiviewDFX::ApiType::NATIVE);
    OHOS::HiviewDFX::AppEventUtilityFacade::WriteApiEndMetric(name, data[0], data[0] % RESULT_MOD, data[1]);
    OHOS::HiviewDFX::AppEventUtilityFacade::GetDate();
    OHOS::HiviewDFX::AppEventUtilityFacade::GetElapsedMilliSecondsSinceBoot();
    OHOS::HiviewDFX::AppEventUtilityFacade::ForceCreateDirectory(FUZZ_STORAGE_DIR);
    OHOS::HiviewDFX::AppEventUtilityFacade::GetFilePathByDir(FUZZ_STORAGE_DIR, "f");
    OHOS::HiviewDFX::AppEventUtilityFacade::IsFileExists(FUZZ_TEST_FILE);
    OHOS::HiviewDFX::AppEventUtilityFacade::SaveStringToFile(FUZZ_TEST_FILE, "v");
}

void FuzzHiappeventFacade(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string name = FuzzStr(data, size, FUZZ_STR_MAX_LEN);
    std::string value = FuzzStr2(data, size, FUZZ_VALUE_MAX_LEN);
    int eventType = data[0] % EVENT_TYPE_COUNT + EVENT_TYPE_FIRST;
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>(name, name, eventType);
    OHOS::HiviewDFX::HiAppEvent::ReportConfig config;
    FuzzFacadeConfigWrite(name, value, data, event, config);
    FuzzFacadeObserverUserInfo(name, value, event, config);
    FuzzFacadeStoreVerify(name, data, event, config);
    FuzzFacadeVerifyUtility(name, value, data);
}
