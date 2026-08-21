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
 * @tc.name: ExternalLogManagerStruct001
 * @tc.desc: test ExternalLogManager struct default values
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogManagerStruct001, TestSize.Level0)
{
    ExternalLogManager logManager;
    EXPECT_TRUE(logManager.externalLogs.empty());
    EXPECT_TRUE(logManager.linkExternalLogs.empty());
}

/**
 * @tc.name: ExternalLogManagerStruct002
 * @tc.desc: test ExternalLogManager struct with data
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogManagerStruct002, TestSize.Level0)
{
    ExternalLogManager logManager;
    logManager.externalLogs = {"/path/log1.txt", "/path/log2.txt"};
    logManager.linkExternalLogs = {{"link1", "link2"}, {"link3"}};
    EXPECT_EQ(logManager.externalLogs.size(), 2u);
    EXPECT_EQ(logManager.linkExternalLogs.size(), 2u);
    EXPECT_EQ(logManager.linkExternalLogs[0].size(), 2u);
    EXPECT_EQ(logManager.linkExternalLogs[1].size(), 1u);
}

/**
 * @tc.name: ExternalLogWrapperInfoStruct001
 * @tc.desc: test ExternalLogWrapperInfo struct default values
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogWrapperInfoStruct001, TestSize.Level0)
{
    ExternalLogWrapperInfo info;
    EXPECT_TRUE(info.filePath.empty());
    EXPECT_EQ(info.generationTime, 0);
    EXPECT_EQ(info.sizeInKb, 0);
    EXPECT_TRUE(info.sysEvent.empty());
}

/**
 * @tc.name: ExternalLogWrapperInfoStruct002
 * @tc.desc: test ExternalLogWrapperInfo struct with assigned values
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogWrapperInfoStruct002, TestSize.Level0)
{
    ExternalLogWrapperInfo info;
    info.filePath = "/data/log/test.txt";
    info.generationTime = 1756735345342LL;
    info.sizeInKb = 1024;
    info.sysEvent = "APP_CRASH";
    EXPECT_EQ(info.filePath, "/data/log/test.txt");
    EXPECT_EQ(info.generationTime, 1756735345342LL);
    EXPECT_EQ(info.sizeInKb, 1024);
    EXPECT_EQ(info.sysEvent, "APP_CRASH");
}

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
    logMgr.externalLogs = {"/log1.txt", "/log2.txt"};
    logMgr.linkExternalLogs = {{"linkA", "linkB"}};
    event->SetExternalLogManager(logMgr);

    ExternalLogManager getResult = event->GetExternalLogManager();
    EXPECT_EQ(getResult.externalLogs.size(), 2u);
    EXPECT_EQ(getResult.externalLogs[0], "/log1.txt");
    EXPECT_EQ(getResult.externalLogs[1], "/log2.txt");
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
    logMgr.externalLogs = {"/log1.txt"};
    logMgr.linkExternalLogs = {{"linkA"}};
    event->SetExternalLogManager(logMgr);

    auto copiedEvent = std::make_shared<AppEventPack>(*event);
    ExternalLogManager copiedLogMgr = copiedEvent->GetExternalLogManager();
    EXPECT_EQ(copiedLogMgr.externalLogs.size(), 1u);
    EXPECT_EQ(copiedLogMgr.externalLogs[0], "/log1.txt");
    EXPECT_EQ(copiedLogMgr.linkExternalLogs.size(), 1u);
    EXPECT_EQ(copiedLogMgr.linkExternalLogs[0][0], "linkA");
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
}

/**
 * @tc.name: GetDirSizeInodeDedup001
 * @tc.desc: test GetDirSize counts unique inodes only (hardlink dedup)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, GetDirSizeInodeDedup001, TestSize.Level0)
{
    std::string dir = TEST_DIR + "inode_dedup/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));

    std::string filePath = dir + "file1.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(filePath, std::string(1024, 'A'), true));

    std::string linkPath = dir + "file1_link.txt";
    link(filePath.c_str(), linkPath.c_str());

    uint64_t size = FileUtil::GetDirSize(dir);
    EXPECT_EQ(size, 1024u);  // Not 2048

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: GetDirSizeInodeDedup002
 * @tc.desc: test GetDirSize counts separate files with different inodes
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, GetDirSizeInodeDedup002, TestSize.Level0)
{
    std::string dir = TEST_DIR + "inode_separate/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));

    std::string file1 = dir + "file1.txt";
    std::string file2 = dir + "file2.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(file1, std::string(512, 'A'), true));
    ASSERT_TRUE(FileUtil::SaveStringToFile(file2, std::string(512, 'B'), true));

    uint64_t size = FileUtil::GetDirSize(dir);
    EXPECT_EQ(size, 1024u);

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: GetDirSizeInodeDedup003
 * @tc.desc: test GetDirSize with empty directory
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, GetDirSizeInodeDedup003, TestSize.Level0)
{
    std::string dir = TEST_DIR + "inode_empty/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));

    uint64_t size = FileUtil::GetDirSize(dir);
    EXPECT_EQ(size, 0u);

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: InsertLinkEventsLogic001
 * @tc.desc: test InsertLinkEvents logic - event with no linkExternalLogs stored directly
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, InsertLinkEventsLogic001, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("testDomain", "testName", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {"/log1.txt"};
    event->SetExternalLogManager(logMgr);

    ExternalLogManager result = event->GetExternalLogManager();
    EXPECT_EQ(result.linkExternalLogs.size(), 0u);
}

/**
 * @tc.name: InsertLinkEventsLogic002
 * @tc.desc: test InsertLinkEvents logic - event with linkExternalLogs creates link events
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, InsertLinkEventsLogic002, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("testDomain", "testName", 1);
    event->SetParamStr(R"({"external_log":["/log1.txt","/log2.txt"],"key":"value"})");

    ExternalLogManager logMgr;
    logMgr.externalLogs = {"/log1.txt", "/log2.txt"};
    logMgr.linkExternalLogs = {
        {"observer1_log1", "observer2_log1"},
        {"observer1_log2"}
    };
    event->SetExternalLogManager(logMgr);

    size_t observerNum = logMgr.linkExternalLogs[0].size();
    EXPECT_EQ(observerNum, 2u);

    for (size_t i = 0; i < observerNum; ++i) {
        std::vector<std::string> linkExternalLogs;
        for (size_t j = 0; j < logMgr.externalLogs.size(); ++j) {
            if (i >= logMgr.linkExternalLogs[j].size()) {
                continue;
            }
            linkExternalLogs.push_back(logMgr.linkExternalLogs[j][i]);
        }

        if (i == 0) {
            EXPECT_EQ(linkExternalLogs.size(), 2u);
            EXPECT_EQ(linkExternalLogs[0], "observer1_log1");
            EXPECT_EQ(linkExternalLogs[1], "observer1_log2");
        } else if (i == 1) {
            EXPECT_EQ(linkExternalLogs.size(), 1u);
            EXPECT_EQ(linkExternalLogs[0], "observer2_log1");
        }
    }
}

/**
 * @tc.name: ExternalLogManagerError001
 * @tc.desc: test ERR_LOG_MANAGER_ALREADY_REGISTERED error code mapping
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, ExternalLogManagerError001, TestSize.Level0)
{
    EXPECT_EQ(NapiError::ERR_LOG_MANAGER_ALREADY_REGISTERED, 11106001);
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
        {"MAIN_THREAD_JANK", OH_MAIN_THREAD_JANK},
        {"APP_HICOLLIE", OH_APP_HICOLLIE},
        {"SCROLL_JANK", OH_SCROLL_JANK},
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
 * @tc.name: AppEventExternalLogManagerSingleton001
 * @tc.desc: test AppEventExternalLogManager singleton returns same instance
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventExternalLogTest, AppEventExternalLogManagerSingleton001, TestSize.Level0)
{
    auto& instance1 = AppEventExternalLogManager::GetInstance();
    auto& instance2 = AppEventExternalLogManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}
}  // namespace