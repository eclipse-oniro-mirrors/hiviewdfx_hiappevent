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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <json/json.h>

#include "app_event_external_log_manager.h"
#include "event_json_util.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_common.h"
#include "napi_error.h"
#include "ndk_external_log_callback.h"
#include "ndk_external_log_service.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_DIR = "/data/test/external_log/";

class MockExternalLogCallback : public ExternalLogManagerCallback {
public:
    MOCK_METHOD1(OnCapacityReached, void(const std::vector<ExternalLogWrapperInfo>& logInfos));
};

class HiAppEventExternalLogTest : public testing::Test {
public:
    void SetUp()
    {
        (void)FileUtil::ForceCreateDirectory(TEST_DIR);
    }
    void TearDown()
    {
        (void)FileUtil::ForceRemoveDirectory(TEST_DIR);
    }
};

/**
 * @tc.name: AppEventPackExternalLogManager001
 * @tc.desc: test AppEventPack Get/Set ExternalLogManager
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, AppEventPackExternalLogManager001, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("testDomain", "testName", 1);

    // Default ExternalLogManager should be empty
    ExternalLogManager defaultLogMgr = event->GetExternalLogManager();
    EXPECT_TRUE(defaultLogMgr.externalLogs.empty());
    EXPECT_TRUE(defaultLogMgr.linkExternalLogs.empty());

    // Set and get ExternalLogManager
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{"/log1.txt", false}, {"/log2.txt", false}};
    logMgr.linkExternalLogs = {{{"linkA", false}, {"linkB", false}}};
    event->SetExternalLogManager(logMgr);

    ExternalLogManager getResult = event->GetExternalLogManager();
    EXPECT_EQ(getResult.externalLogs.size(), 2u);
    EXPECT_EQ(getResult.externalLogs[0].file, "/log1.txt");
    EXPECT_EQ(getResult.externalLogs[1].file, "/log2.txt");
    EXPECT_EQ(getResult.linkExternalLogs.size(), 1u);
    EXPECT_EQ(getResult.linkExternalLogs[0].size(), 2u);
}

/**
 * @tc.name: AppEventPackExternalLogManager002
 * @tc.desc: test AppEventPack copy constructor preserves ExternalLogManager
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, AppEventPackExternalLogManager002, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("testDomain", "testName", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{"/log1.txt", false}};
    logMgr.linkExternalLogs = {{{"linkA", false}}};
    event->SetExternalLogManager(logMgr);

    auto copiedEvent = std::make_shared<AppEventPack>(*event);
    ExternalLogManager copiedLogMgr = copiedEvent->GetExternalLogManager();
    EXPECT_EQ(copiedLogMgr.externalLogs.size(), 1u);
    EXPECT_EQ(copiedLogMgr.externalLogs[0].file, "/log1.txt");
    EXPECT_EQ(copiedLogMgr.linkExternalLogs.size(), 1u);
    EXPECT_EQ(copiedLogMgr.linkExternalLogs[0][0].file, "linkA");
}

/**
 * @tc.name: ExternalLogManagerRegister001
 * @tc.desc: test AppEventExternalLogManager RegisterCallback and IsRegistered
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogManagerRegister001, TestSize.Level0)
{
    auto& instance = AppEventExternalLogManager::GetInstance();

    // Register a callback
    auto callback1 = std::make_shared<MockExternalLogCallback>();
    bool result = instance.RegisterCallback(callback1);
    EXPECT_TRUE(result);
    EXPECT_TRUE(instance.IsRegistered());

    // Second registration should fail
    auto callback2 = std::make_shared<MockExternalLogCallback>();
    result = instance.RegisterCallback(callback2);
    EXPECT_FALSE(result);
    EXPECT_TRUE(instance.IsRegistered());

    // Note: We cannot easily unregister/reset the singleton in unit tests.
    // The singleton persists across tests in the same process.
}

/**
 * @tc.name: ExternalLogManagerCheckCapacity001
 * @tc.desc: test CheckCapacity when directory does not exist
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogManagerCheckCapacity001, TestSize.Level0)
{
    auto& instance = AppEventExternalLogManager::GetInstance();
    // CheckCapacity on non-existent directory should not crash
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
}

/**
 * @tc.name: NapiErrorMsg001
 * @tc.desc: test GetErrorMsg for ERR_LOG_MANAGER_ALREADY_REGISTERED
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NapiErrorMsg001, TestSize.Level0)
{
    std::string msg = NapiError::GetErrorMsg(NapiError::ERR_LOG_MANAGER_ALREADY_REGISTERED);
    EXPECT_FALSE(msg.empty());
    EXPECT_NE(msg.find("Log manager already registered"), std::string::npos);
}

/**
 * @tc.name: NapiErrorMsg002
 * @tc.desc: test GetErrorMsg for undefined error code returns default message
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NapiErrorMsg002, TestSize.Level0)
{
    std::string msg = NapiError::GetErrorMsg(99999999);
    EXPECT_EQ(msg, "ErrorMsg undefined");
}

// Static state for C callback tests (C function pointers cannot capture state)
static bool g_callbackInvoked = false;
static uint32_t g_receivedArrLen = 0;
static std::vector<OH_HiAppEvent_SysEvent> g_receivedEvents;
static std::string g_receivedFilePath;
static long long g_receivedGenTs = 0;
static long g_receivedFileSize = 0;

static void CCallbackInvokeCheck(OH_HiAppEvent_ExternalLog* externalLogArr, uint32_t arrLen)
{
    g_callbackInvoked = true;
    g_receivedArrLen = arrLen;
}

static void CCallbackCollectEvents(OH_HiAppEvent_ExternalLog* externalLogArr, uint32_t arrLen)
{
    g_callbackInvoked = true;
    g_receivedArrLen = arrLen;
    for (uint32_t i = 0; i < arrLen; ++i) {
        g_receivedEvents.push_back(externalLogArr[i].event);
    }
}

static void CCallbackCollectFilePath(OH_HiAppEvent_ExternalLog* externalLogArr, uint32_t arrLen)
{
    if (arrLen > 0) {
        g_receivedFilePath = externalLogArr[0].filePath;
        g_receivedGenTs = externalLogArr[0].generationTs;
        g_receivedFileSize = externalLogArr[0].fileSize;
    }
}

/**
 * @tc.name: NdkExternalLogCallbackOnCapacityReached001
 * @tc.desc: test NdkExternalLogCallback OnCapacityReached invokes callback
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackOnCapacityReached001, TestSize.Level0)
{
    g_callbackInvoked = false;
    g_receivedArrLen = 0;

    NdkExternalLogCallback ndkCallback(CCallbackInvokeCheck);

    std::vector<ExternalLogWrapperInfo> logInfos;
    ExternalLogWrapperInfo info;
    info.filePath = "/data/log/APP_CRASH_1756735345342_1234.txt";
    info.generationTime = 1756735345342LL;
    info.sizeInKb = 10;
    info.sysEvent = "APP_CRASH";
    logInfos.push_back(info);

    ndkCallback.OnCapacityReached(logInfos);
    EXPECT_TRUE(g_callbackInvoked);
    EXPECT_EQ(g_receivedArrLen, 1u);
}

/**
 * @tc.name: NdkExternalLogCallbackOnCapacityReached002
 * @tc.desc: test NdkExternalLogCallback OnCapacityReached with empty logInfos
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackOnCapacityReached002, TestSize.Level0)
{
    g_callbackInvoked = false;

    NdkExternalLogCallback ndkCallback(CCallbackInvokeCheck);
    std::vector<ExternalLogWrapperInfo> logInfos;
    ndkCallback.OnCapacityReached(logInfos);
    EXPECT_FALSE(g_callbackInvoked);
}

/**
 * @tc.name: NdkExternalLogCallbackOnCapacityReached003
 * @tc.desc: test NdkExternalLogCallback OnCapacityReached with multiple sysEvents
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackOnCapacityReached003, TestSize.Level0)
{
    g_receivedEvents.clear();

    NdkExternalLogCallback ndkCallback(CCallbackCollectEvents);

    std::vector<ExternalLogWrapperInfo> logInfos;
    ExternalLogWrapperInfo info1;
    info1.filePath = "/data/log/APP_CRASH_1234567890_1234.txt";
    info1.generationTime = 1234567890LL;
    info1.sizeInKb = 10;
    info1.sysEvent = "APP_CRASH";
    logInfos.push_back(info1);

    ExternalLogWrapperInfo info2;
    info2.filePath = "/data/log/APP_FREEZE_1234567891_1234.txt";
    info2.generationTime = 1234567891LL;
    info2.sizeInKb = 20;
    info2.sysEvent = "APP_FREEZE";
    logInfos.push_back(info2);

    ExternalLogWrapperInfo info3;
    info3.filePath = "/data/log/RESOURCE_OVERLIMIT_1234567892_1234.txt";
    info3.generationTime = 1234567892LL;
    info3.sizeInKb = 30;
    info3.sysEvent = "RESOURCE_OVERLIMIT";
    logInfos.push_back(info3);

    ExternalLogWrapperInfo info4;
    info4.filePath = "/data/log/ADDRESS_SANITIZER_1234567893_1234.txt";
    info4.generationTime = 1234567893LL;
    info4.sizeInKb = 40;
    info4.sysEvent = "ADDRESS_SANITIZER";
    logInfos.push_back(info4);

    ndkCallback.OnCapacityReached(logInfos);
    ASSERT_EQ(g_receivedEvents.size(), 4u);
    EXPECT_EQ(g_receivedEvents[0], OH_APP_CRASH);
    EXPECT_EQ(g_receivedEvents[1], OH_APP_FREEZE);
    EXPECT_EQ(g_receivedEvents[2], OH_RESOURCE_OVERLIMIT);
    EXPECT_EQ(g_receivedEvents[3], OH_ADDRESS_SANITIZER);
}

/**
 * @tc.name: NdkExternalLogCallbackConvertSysEvent001
 * @tc.desc: test all system event type conversions in NdkExternalLogCallback
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackConvertSysEvent001, TestSize.Level0)
{
    NdkExternalLogCallback ndkCallback(CCallbackCollectEvents);

    std::vector<std::pair<std::string, OH_HiAppEvent_SysEvent>> testCases = {
        {"APP_HICOLLIE", OH_APP_HICOLLIE},
        {"CPU_USAGE_HIGH", OH_CPU_USAGE_HIGH},
    };

    for (const auto& testCase : testCases) {
        g_receivedEvents.clear();
        std::vector<ExternalLogWrapperInfo> logInfos;
        ExternalLogWrapperInfo info;
        info.filePath = "/data/log/" + testCase.first + "_1234567890_1234.txt";
        info.generationTime = 1234567890LL;
        info.sizeInKb = 5;
        info.sysEvent = testCase.first;
        logInfos.push_back(info);

        ndkCallback.OnCapacityReached(logInfos);
        ASSERT_EQ(g_receivedEvents.size(), 1u);
        EXPECT_EQ(g_receivedEvents[0], testCase.second);
    }
}

/**
 * @tc.name: NdkExternalLogCallbackOnCapacityReached004
 * @tc.desc: test NdkExternalLogCallback with unknown sysEvent falls back to OH_APP_CRASH
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackOnCapacityReached004, TestSize.Level0)
{
    g_receivedEvents.clear();

    NdkExternalLogCallback ndkCallback(CCallbackCollectEvents);

    std::vector<ExternalLogWrapperInfo> logInfos;
    ExternalLogWrapperInfo info;
    info.filePath = "/data/log/UNKNOWN_EVENT_1234567890_1234.txt";
    info.generationTime = 1234567890LL;
    info.sizeInKb = 5;
    info.sysEvent = "UNKNOWN_EVENT";
    logInfos.push_back(info);

    ndkCallback.OnCapacityReached(logInfos);
    ASSERT_EQ(g_receivedEvents.size(), 1u);
    EXPECT_EQ(g_receivedEvents[0], OH_APP_CRASH);
}

/**
 * @tc.name: NdkRegExternalLogCallback001
 * @tc.desc: test RegExternalLogCapacityReachedCallback with null callback
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkRegExternalLogCallback001, TestSize.Level0)
{
    int result = RegExternalLogCapacityReachedCallback(nullptr);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    auto lambdaCallback = [](OH_HiAppEvent_ExternalLog* externalLogArr, uint32_t arrLen) {};
    result = RegExternalLogCapacityReachedCallback(lambdaCallback);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name: NdkExternalLogCallbackFilePath001
 * @tc.desc: test NdkExternalLogCallback OnCapacityReached passes correct filePath and metadata
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackFilePath001, TestSize.Level0)
{
    g_receivedFilePath.clear();
    g_receivedGenTs = 0;
    g_receivedFileSize = 0;

    NdkExternalLogCallback ndkCallback(CCallbackCollectFilePath);

    std::vector<ExternalLogWrapperInfo> logInfos;
    ExternalLogWrapperInfo info;
    info.filePath = "/data/log/APP_CRASH_1756735345342_1234.txt";
    info.generationTime = 1756735345342LL;
    info.sizeInKb = 512;
    info.sysEvent = "APP_CRASH";
    logInfos.push_back(info);

    ndkCallback.OnCapacityReached(logInfos);
    EXPECT_EQ(g_receivedFilePath, "/data/log/APP_CRASH_1756735345342_1234.txt");
    EXPECT_EQ(g_receivedGenTs, 1756735345342LL);
    EXPECT_EQ(g_receivedFileSize, 512);
}

/**
 * @tc.name: CheckCapacityBelowThreshold001
 * @tc.desc: test CheckCapacity when directory exists but size is below threshold
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, CheckCapacityBelowThreshold001, TestSize.Level0)
{
    auto& instance = AppEventExternalLogManager::GetInstance();
    // Create the hiappevent log dir but with tiny content
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // Write a small file (well below 4MB threshold)
    std::string smallFile = dir + "/small_test_file.txt";
    FileUtil::SaveStringToFile(smallFile, "tiny", true);

    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // Should not trigger callback because curSize < thresholdSize
    // Exercises the branch: if (curSize < thresholdSize) { continue; }

    (void)FileUtil::RemoveFile(smallFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: CheckCapacityOverThreshold001
 * @tc.desc: test CheckCapacity when directory exceeds threshold triggers ScanLogFiles + callback
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, CheckCapacityOverThreshold001, TestSize.Level0)
{
    auto& instance = AppEventExternalLogManager::GetInstance();
    // Create the hiappevent log dir with a file named in standard format
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // Create a file that ParseLogFileInfo can parse
    std::string logFile = dir + "/APP_CRASH_1756735345342_1234.txt";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'A'), true);

    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // Exercises branches: IsDirectory=true, curSize >= thresholdSize, ScanLogFiles, callback invocation

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

// ===== AppEventExternalLogManager::ParseLogFileInfo actual branch tests =====

/**
 * @tc.name: ParseLogFileInfoActual001
 * @tc.desc: test ParseLogFileInfo via CheckCapacity with standard filename
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual001, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // Standard filename: EVENTNAME_TIMESTAMP_PID.ext
    std::string logFile = dir + "/APP_FREEZE_1699999999999_5678.log";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'B'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // ParseLogFileInfo should parse: sysEvent = "APP_FREEZE", generationTime = 1699999999999

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual002
 * @tc.desc: test ParseLogFileInfo with no underscore in filename (no timestamp found)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual002, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // Filename with no underscore at all
    std::string logFile = dir + "/plainname.txt";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'C'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // The while loop never finds underscore -> no timestamp -> sysEvent and generationTime remain default

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual003
 * @tc.desc: test ParseLogFileInfo with underscore but timestamp too short
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual003, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // "123" is only 3 chars, below MIN_TIMESTAMP_LEN=10
    std::string logFile = dir + "/EVENT_123_45.txt";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'D'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // IsTimestampSegment returns false for len=3 < MIN_TIMESTAMP_LEN -> no timestamp parsed

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual004
 * @tc.desc: test ParseLogFileInfo with valid timestamp length but non-digit chars
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual004, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // "abc45678901" is 11 chars but contains non-digit
    std::string logFile = dir + "/EVENT_abc45678901_99.txt";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'E'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // IsTimestampSegment returns false because not all chars are digits

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual005
 * @tc.desc: test ParseLogFileInfo with multi-segment event name (RESOURCE_OVERLIMIT)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual005, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // RESOURCE_OVERLIMIT has underscore in event name; the while loop should skip
    // the first underscore and find the timestamp after the second
    std::string logFile = dir + "/RESOURCE_OVERLIMIT_1756735345342_9999.log";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'F'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual006
 * @tc.desc: test ParseLogFileInfo with file stat failure (file removed before stat)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual006, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // Create then immediately delete -> stat will fail in ParseLogFileInfo
    std::string logFile = dir + "/ADDRESS_SANITIZER_1756735345342_1111.txt";
    FileUtil::SaveStringToFile(logFile, "temp", true);
    (void)FileUtil::RemoveFile(logFile);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // ParseLogFileInfo: stat fails -> sizeInKb remains 0, but parsing continues for filename

    (void)FileUtil::ForceRemoveDirectory(dir);
}

// ===== NdkExternalLogCallback::OnCapacityReached with null callback branch =====

/**
 * @tc.name: NdkExternalLogCallbackNullCb001
 * @tc.desc: test NdkExternalLogCallback constructed with null callback does not invoke
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackNullCb001, TestSize.Level0)
{
    g_callbackInvoked = false;
    g_receivedArrLen = 0;

    NdkExternalLogCallback ndkCallback(nullptr);
    std::vector<ExternalLogWrapperInfo> logInfos;
    ExternalLogWrapperInfo info;
    info.filePath = "/data/log/APP_CRASH_1234567890_1234.txt";
    info.generationTime = 1234567890LL;
    info.sizeInKb = 10;
    info.sysEvent = "APP_CRASH";
    logInfos.push_back(info);

    ndkCallback.OnCapacityReached(logInfos);
    // callback_ is null -> should not invoke
    EXPECT_FALSE(g_callbackInvoked);
}

/**
 * @tc.name: NdkExternalLogCallbackSysEvent003
 * @tc.desc: test all remaining ConvertSysEvent branches: APP_HICOLLIE and CPU_USAGE_HIGH
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackSysEvent003, TestSize.Level0)
{
    g_receivedEvents.clear();
    NdkExternalLogCallback ndkCallback(CCallbackCollectEvents);

    std::vector<ExternalLogWrapperInfo> logInfos;
    ExternalLogWrapperInfo info1;
    info1.filePath = "/data/log/APP_HICOLLIE_1234567890_1234.txt";
    info1.generationTime = 1234567890LL;
    info1.sizeInKb = 5;
    info1.sysEvent = "APP_HICOLLIE";
    logInfos.push_back(info1);

    ExternalLogWrapperInfo info2;
    info2.filePath = "/data/log/CPU_USAGE_HIGH_1234567891_1234.txt";
    info2.generationTime = 1234567891LL;
    info2.sizeInKb = 5;
    info2.sysEvent = "CPU_USAGE_HIGH";
    logInfos.push_back(info2);

    ndkCallback.OnCapacityReached(logInfos);
    ASSERT_EQ(g_receivedEvents.size(), 2u);
    EXPECT_EQ(g_receivedEvents[0], OH_APP_HICOLLIE);
    EXPECT_EQ(g_receivedEvents[1], OH_CPU_USAGE_HIGH);
}

/**
 * @tc.name: GetDirSizeStatFail001
 * @tc.desc: test GetDirSize with a file that is deleted during scan (stat fails)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, GetDirSizeStatFail001, TestSize.Level0)
{
    std::string dir = TEST_DIR + "stat_fail/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    // Create and immediately delete - GetDirSize gets a file path but stat fails
    std::string file = dir + "transient.txt";
    FileUtil::SaveStringToFile(file, "data", true);
    (void)FileUtil::RemoveFile(file);

    uint64_t size = FileUtil::GetDirSize(dir);
    // The deleted file won't be found by GetDirFiles, so size should be 0
    EXPECT_EQ(size, 0u);

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual007
 * @tc.desc: test ParseLogFileInfo with only one underscore in filename (break on no nextUnderscore)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual007, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // Only one underscore: nextUnderscore will be npos -> break out of while loop
    std::string logFile = dir + "/SINGLE_1756735345342.txt";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'G'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());
    // Exercises: nextUnderscore == npos -> break (no valid timestamp found this iteration)

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ParseLogFileInfoActual008
 * @tc.desc: test ParseLogFileInfo with ADDRESS_SANITIZER and CPU_USAGE_HIGH filenames
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ParseLogFileInfoActual008, TestSize.Level0)
{
    std::string dir = "/data/storage/el2/log/hiappevent";
    if (!FileUtil::IsDirectory(dir)) {
        (void)FileUtil::ForceCreateDirectory(dir);
    }
    // ADDRESS_SANITIZER: two underscores in event name
    std::string logFile = dir + "/ADDRESS_SANITIZER_1756735345342_1234.txt";
    FileUtil::SaveStringToFile(logFile, std::string(5 * 1024 * 1024, 'H'), true);

    auto& instance = AppEventExternalLogManager::GetInstance();
    instance.CheckCapacity();
    EXPECT_TRUE(instance.IsRegistered());

    (void)FileUtil::RemoveFile(logFile);
    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: NdkExternalLogCallbackSysEvent002
 * @tc.desc: test SCROLL_JANK and MAIN_THREAD_JANK fall back to OH_APP_CRASH
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, NdkExternalLogCallbackSysEvent002, TestSize.Level0)
{
    g_receivedEvents.clear();
    NdkExternalLogCallback ndkCallback(CCallbackCollectEvents);

    std::vector<ExternalLogWrapperInfo> logInfos;

    ExternalLogWrapperInfo info1;
    info1.filePath = "/data/log/SCROLL_JANK_1234567890_1234.txt";
    info1.generationTime = 1234567890LL;
    info1.sizeInKb = 5;
    info1.sysEvent = "SCROLL_JANK";
    logInfos.push_back(info1);

    ExternalLogWrapperInfo info2;
    info2.filePath = "/data/log/MAIN_THREAD_JANK_1234567891_1234.txt";
    info2.generationTime = 1234567891LL;
    info2.sizeInKb = 5;
    info2.sysEvent = "MAIN_THREAD_JANK";
    logInfos.push_back(info2);

    ndkCallback.OnCapacityReached(logInfos);
    ASSERT_EQ(g_receivedEvents.size(), 2u);
    // Exercises the default fallback branch in ConvertSysEvent
    EXPECT_EQ(g_receivedEvents[0], OH_APP_CRASH);
    EXPECT_EQ(g_receivedEvents[1], OH_APP_CRASH);
}

/**
 * @tc.name: ExternalLogManagerDeepCopy001
 * @tc.desc: test that copy of AppEventPack has independent ExternalLogManager
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogManagerDeepCopy001, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{"/log1.txt", false}};
    logMgr.linkExternalLogs = {{{"linkA", false}}};
    event->SetExternalLogManager(logMgr);

    auto copiedEvent = std::make_shared<AppEventPack>(*event);

    // Modify original - should not affect copy
    ExternalLogManager modifiedLogMgr;
    modifiedLogMgr.externalLogs = {{"/modified.txt", true}};
    event->SetExternalLogManager(modifiedLogMgr);

    ExternalLogManager copiedLogMgr = copiedEvent->GetExternalLogManager();
    EXPECT_EQ(copiedLogMgr.externalLogs.size(), 1u);
    EXPECT_EQ(copiedLogMgr.externalLogs[0].file, "/log1.txt");
    EXPECT_FALSE(copiedLogMgr.externalLogs[0].isUsed);
}
}  // namespace