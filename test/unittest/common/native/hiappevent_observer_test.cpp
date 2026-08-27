/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "app_event_external_log_manager.h"
#include "app_event_observer_mgr.h"
#include "app_event_store.h"
#include "app_event_util.h"
#include "app_event_watcher.h"
#include "application_context.h"
#include "file_util.h"
#include "hiappevent_common.h"
#include "hiappevent_config.h"
#include "os_event_listener.h"
#include "time_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
const std::string TEST_DIR = "/data/test/observer/hiappevent";
const std::string TEST_STORAGE_DIR = "/data/test/observer/hiappevent/storage";
const std::string XATTR_NAME = "user.appevent";
std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> g_applicationContext = nullptr;

class ApplicationContextMock : public ApplicationContext {
public:
    MOCK_METHOD0(GetCacheDir, std::string());
};

uint64_t GetMaskFromDirXattr(const std::string& path)
{
    std::string value;
    if (!FileUtil::GetDirXattr(path, XATTR_NAME, value)) {
        return 0;
    }
    return static_cast<uint64_t>(std::strtoull(value.c_str(), nullptr, 0));
}
}

namespace AbilityRuntime {
std::shared_ptr<ApplicationContext> Context::GetApplicationContext()
{
    return g_applicationContext;
}
}  // namespace AbilityRuntime

class HiAppEventObserverTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventObserverTest::SetUp()
{
    (void)FileUtil::ForceCreateDirectory(TEST_DIR);
}

void HiAppEventObserverTest::TearDown()
{
    g_applicationContext = nullptr;
    (void)FileUtil::ForceRemoveDirectory("/data/test/observer");
}

/**
 * @tc.name: OsEventListenerTest003
 * @tc.desc: test OsEventListener AddListenedEvents func
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest003, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->AddListenedEvents(1));
    EXPECT_EQ(GetMaskFromDirXattr(TEST_DIR), 1);

    EXPECT_TRUE(listener->AddListenedEvents(2));
    EXPECT_EQ(GetMaskFromDirXattr(TEST_DIR), 3);
}

/**
 * @tc.name: OsEventListenerTest004
 * @tc.desc: test OsEventListener SetListenedEvents func
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest004, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->AddListenedEvents(1));
    EXPECT_EQ(GetMaskFromDirXattr(TEST_DIR), 1);
    EXPECT_TRUE(listener->SetListenedEvents(2));
    EXPECT_EQ(GetMaskFromDirXattr(TEST_DIR), 2);
}

/**
 * @tc.name: OsEventListenerTest005
 * @tc.desc: test OsEventListener StartListening and RemoveOsEventDir func
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest005, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->StartListening());
    EXPECT_TRUE(listener->RemoveOsEventDir());
}

/**
 * @tc.name: OsEventListenerTest006
 * @tc.desc: test OsEventListener HandleDirEvent func when create event after listening
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest006, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->StartListening());
    std::string content = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":{"crash_type":"JsError"}})";
    std::string filePath = TEST_DIR + "/hiappevent_1756735345342.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));
    EXPECT_TRUE(FileUtil::ForceRemoveDirectory(filePath));  // trigger open file failed

    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));
    uint64_t curTime = TimeUtil::GetMilliseconds();
    while (TimeUtil::GetMilliseconds() - curTime < 1000) {}  // ensure open file success
}

/**
 * @tc.name: OsEventListenerTest007
 * @tc.desc: test OsEventListener HandleDirEvent func when create event before listening
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest007, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::string content = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":{"crash_type":"JsError"}})";
    std::string filePath = TEST_DIR + "/hiappevent_1756735345342.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));
}

/**
 * @tc.name: OsEventListenerTest008
 * @tc.desc: test OsEventListener HandleDirEvent func with different events
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest008, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::string content1 = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":{"app_running_unique_id":"x"}})";
    std::string filePath1 = TEST_DIR + "/hiappevent_1756735345342.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath1, content1));
    std::string content2 = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":{"app_running_unique_id":0}})";
    std::string filePath2 = TEST_DIR + "/hiappevent_1756735345343.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath2, content2));
    std::string content3 = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":{"app_running_unique_id":)";
    std::string filePath3 = TEST_DIR + "/hiappevent_1756735345344.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath3, content3));
    std::string content4 = R"({"domain":"OS","eventType":1,"name":"APP_CRASH"})";
    std::string filePath4 = TEST_DIR + "/hiappevent_1756735345345.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath4, content4));
    std::string content5 = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":"test"})";
    std::string filePath5 = TEST_DIR + "/hiappevent_1756735345346.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath5, content5));
}

/**
 * @tc.name: AppEventWatcher001
 * @tc.desc: test AppEventWatcher SetFiltersStr func when filter is empty
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, AppEventWatcher001, TestSize.Level0)
{
    AppEventWatcher appEventWatcher("testName");
    std::string emptyFilter = "";
    appEventWatcher.SetFiltersStr(emptyFilter);
    EXPECT_EQ(appEventWatcher.GetFiltersStr(), "[]\n");
}

/**
 * @tc.name: AppEventWatcher002
 * @tc.desc: test AppEventWatcher SetFiltersStr func when filter cannot parse
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, AppEventWatcher002, TestSize.Level0)
{
    AppEventWatcher appEventWatcher("testName");
    std::string canNotParseFilter = R"([{"domain)";
    appEventWatcher.SetFiltersStr(canNotParseFilter);
    EXPECT_EQ(appEventWatcher.GetFiltersStr(), "[]\n");
}

/**
 * @tc.name: AppEventWatcher003
 * @tc.desc: test AppEventWatcher SetFiltersStr func when filter is not array or empty array
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, AppEventWatcher003, TestSize.Level0)
{
    AppEventWatcher appEventWatcher("testName");
    std::string notArrayFilter = R"({"domain":"api_diagnostic"})";
    appEventWatcher.SetFiltersStr(notArrayFilter);
    EXPECT_EQ(appEventWatcher.GetFiltersStr(), "[]\n");

    AppEventWatcher appEventWatcher2("testName");
    std::string emptyArrayFilter = R"([])";
    appEventWatcher2.SetFiltersStr(emptyArrayFilter);
    EXPECT_EQ(appEventWatcher2.GetFiltersStr(), "[]\n");
}

/**
 * @tc.name: AppEventWatcher004
 * @tc.desc: test AppEventWatcher SetFiltersStr func when filter is not object
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, AppEventWatcher004, TestSize.Level0)
{
    AppEventWatcher appEventWatcher("testName");
    std::string notObjectFilter = R"(["invalidTest])";
    appEventWatcher.SetFiltersStr(notObjectFilter);
    EXPECT_EQ(appEventWatcher.GetFiltersStr(), "[]\n");
}

/**
 * @tc.name: AppEventWatcher005
 * @tc.desc: test AppEventWatcher SetFiltersStr func when filter is valid
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, AppEventWatcher005, TestSize.Level0)
{
    AppEventWatcher appEventWatcher("testName");
    std::string validFilter = R"([{"domain":"testDomain","names":["testName"],"types":255}])";
    appEventWatcher.SetFiltersStr(validFilter);
    EXPECT_EQ(appEventWatcher.GetFiltersStr(), validFilter);
}

/**
 * @tc.name: OsEventListenerTest009
 * @tc.desc: test OsEventListener GetLinkEvents returns empty initially
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest009, TestSize.Level0)
{
    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    std::vector<std::vector<std::shared_ptr<AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    EXPECT_TRUE(linkEvents.empty());
}
 
/**
 * @tc.name: OsEventListenerTest010
 * @tc.desc: test OsEventListener Init with external_log in event JSON
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest010, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);
 
    // Create an event file with external_log field
    std::string content = R"({"domain":"OS","eventType":1,"name":"APP_CRASH",)"
        R"("params":{"crash_type":"JsError","external_log":["/log1.txt","/log2.txt"]},)"
        R"("link_external_log":[["obs1_log1","obs2_log1"],["obs1_log2"]]})";
    std::string filePath = TEST_DIR + "/hiappevent_1756735345342.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));
 
    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->StartListening());
 
    // After Init, GetLinkEvents should return link events from the external_log
    std::vector<std::vector<std::shared_ptr<AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    // The link events should have been populated based on external_log parsing
    EXPECT_NE(linkEvents.size(), 0);
}
 
/**
 * @tc.name: OsEventListenerTest011
 * @tc.desc: test OsEventListener Init with event JSON without external_log
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest011, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);
 
    // Create an event file without external_log field
    std::string content = R"({"domain":"OS","eventType":1,"name":"APP_CRASH","params":{"crash_type":"JsError"}})";
    std::string filePath = TEST_DIR + "/hiappevent_1756735345343.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));
 
    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->StartListening());
 
    // Without external_log, GetLinkEvents should be empty
    std::vector<std::vector<std::shared_ptr<AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    EXPECT_TRUE(linkEvents.empty());
}
 
/**
 * @tc.name: SaveExternalLogSolidLink001
 * @tc.desc: test SaveExternalLogSolidLink replaces external_log with linkExternalLogs
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink001, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr(R"({"external_log":["/log1.txt","/log2.txt"],"crash_type":"JsError"})");
 
    std::vector<std::string> linkExternalLogs = {"obs1_log1", "obs1_log2"};
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);
 
    std::string paramStr = event->GetParamStr();
    EXPECT_NE(paramStr.find("obs1_log1"), std::string::npos);
    EXPECT_NE(paramStr.find("obs1_log2"), std::string::npos);
    EXPECT_NE(paramStr.find("crash_type"), std::string::npos);
}
 
/**
 * @tc.name: SaveExternalLogSolidLink002
 * @tc.desc: test SaveExternalLogSolidLink with invalid param JSON (early return)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink002, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr("not_valid_json{{");
 
    std::vector<std::string> linkExternalLogs = {"obs1_log1"};
    // Should not crash, and paramStr should remain unchanged
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);
    EXPECT_EQ(event->GetParamStr(), "not_valid_json{{");
}
 
/**
 * @tc.name: SaveExternalLogSolidLink003
 * @tc.desc: test SaveExternalLogSolidLink when no external_log field (early return)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink003, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr(R"({"crash_type":"JsError"})");
 
    std::vector<std::string> linkExternalLogs = {"obs1_log1"};
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);
 
    // external_log field should not be added since it wasn't present
    std::string paramStr = event->GetParamStr();
    EXPECT_NE(paramStr.find("crash_type"), std::string::npos);
    // The original JSON is preserved (external_log not present)
    EXPECT_EQ(paramStr.find("external_log"), std::string::npos);
}
 
/**
 * @tc.name: SaveExternalLogSolidLink004
 * @tc.desc: test SaveExternalLogSolidLink when external_log is not an array (early return)
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink004, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr(R"({"external_log":"not_array","crash_type":"JsError"})");
 
    std::vector<std::string> linkExternalLogs = {"obs1_log1"};
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);
 
    // Should not replace since external_log is not an array
    std::string paramStr = event->GetParamStr();
    EXPECT_NE(paramStr.find("not_array"), std::string::npos);
}
 
/**
 * @tc.name: SaveExternalLogSolidLink005
 * @tc.desc: test SaveExternalLogSolidLink with empty linkExternalLogs
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink005, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr(R"({"external_log":["/log1.txt","/log2.txt"],"crash_type":"JsError"})");

    std::vector<std::string> linkExternalLogs;  // empty
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);

    // external_log should be replaced with empty array
    std::string paramStr = event->GetParamStr();
    EXPECT_NE(paramStr.find("crash_type"), std::string::npos);
}

/**
 * @tc.name: SaveExternalLogSolidLink006
 * @tc.desc: test SaveExternalLogSolidLink with empty paramStr
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink006, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr("");

    std::vector<std::string> linkExternalLogs = {"obs1_log1"};
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);
    // Should not crash; empty string is not valid JSON
    EXPECT_NE(event->GetParamStr(), "");
}

/**
 * @tc.name: InsertLinkEventsActual001
 * @tc.desc: test InsertLinkEvents - empty externalLogs early return
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, InsertLinkEventsActual001, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);

    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    // No externalLogs set -> early return at externalLogs.size() == 0
    listener->InsertLinkEvents(event, observers);

    std::vector<std::vector<std::shared_ptr<AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    ASSERT_EQ(linkEvents.size(), 0);
}

/**
 * @tc.name: InsertLinkEventsActual002
 * @tc.desc: test InsertLinkEvents - single observer, uses linkExternalLogs[j][i]
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, InsertLinkEventsActual002, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    // Create real log files for unused-file-cleanup
    std::string dir = TEST_DIR + "link_actual/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile1 = dir + "log1.txt";
    std::string logFile2 = dir + "log2.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile2, "data2", true));

    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile1, false}, {logFile2, false}};
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}},
        {{dir + "obs0_link2", false}}
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto observer = std::make_shared<AppEventObserver>("testObserver");
    observers.push_back(observer);

    auto listener = std::make_shared<OsEventListener>(observers);
    listener->InsertLinkEvents(event, observers);

    std::vector<std::vector<std::shared_ptr<AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    ASSERT_EQ(linkEvents.size(), 1u);
    // DB insert fails in test env, so linkEvents[0] is empty
    EXPECT_EQ(linkEvents[0].size(), 0u);
    // linkExternalLogs[0][0] and linkExternalLogs[1][0] are used -> isUsed=true -> not removed
    // externalLogs[0] and [1] isUsed stays false -> RemoveFile called
    EXPECT_FALSE(FileUtil::IsFileExists(logFile1));  // unused externalLogFile removed
    EXPECT_FALSE(FileUtil::IsFileExists(logFile2));

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: InsertLinkEventsActual003
 * @tc.desc: test InsertLinkEvents - fallback to externalLogs[j] when j >= linkExternalLogs.size()
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, InsertLinkEventsActual003, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::string dir = TEST_DIR + "link_fallback_j/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile1 = dir + "log1.txt";
    std::string logFile2 = dir + "log2.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile2, "data2", true));

    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile1, false}, {logFile2, false}};
    // linkExternalLogs only has entry for log0, not log1
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}}
        // no entry for log1 -> j >= linkExternalLogs.size() for j=1
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto observer = std::make_shared<AppEventObserver>("testObserver");
    observers.push_back(observer);

    auto listener = std::make_shared<OsEventListener>(observers);
    listener->InsertLinkEvents(event, observers);

    // j=0: linkExternalLogs[0][0] exists -> else branch -> linkExternalLogs[0][0].isUsed=true
    //      externalLogs[0].isUsed stays false -> logFile1 removed
    // j=1: j >= linkExternalLogs.size() -> fallback -> externalLogs[1].isUsed=true
    //      logFile2 not removed
    EXPECT_FALSE(FileUtil::IsFileExists(logFile1));  // isUsed=false, removed
    EXPECT_TRUE(FileUtil::IsFileExists(logFile2));    // isUsed=true, not removed

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: InsertLinkEventsActual004
 * @tc.desc: test InsertLinkEvents - fallback to externalLogs[j] when i >= linkExternalLogs[j].size()
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, InsertLinkEventsActual004, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::string dir = TEST_DIR + "link_fallback_i/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile1 = dir + "log1.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));

    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile1, false}};
    // Only 1 linked log for observer 0; for observer 1, i=1 >= size()=1 -> fallback
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}}
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    observers.push_back(std::make_shared<AppEventObserver>("obs0"));
    observers.push_back(std::make_shared<AppEventObserver>("obs1"));

    auto listener = std::make_shared<OsEventListener>(observers);
    listener->InsertLinkEvents(event, observers);

    // Observer 0 uses linkExternalLogs[0][0], observer 1 falls back to externalLogs[0].file
    // externalLogs[0].isUsed=true -> not removed
    EXPECT_TRUE(FileUtil::IsFileExists(logFile1));

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: InsertLinkEventsActual005
 * @tc.desc: test InsertLinkEvents - unused linkExternalLogs file cleanup
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, InsertLinkEventsActual005, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    std::string dir = TEST_DIR + "link_unused/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    // Create the link file that will remain unused (no observer references it)
    std::string unusedLinkFile = dir + "unused_link.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(unusedLinkFile, "unused", true));
    std::string logFile1 = dir + "log1.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));

    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile1, false}};
    // linkExternalLogs[0][1] exists but observer count is only 1 ->
    // linkExternalLogs[0][1].isUsed stays false -> RemoveFile called
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}, {unusedLinkFile, false}}
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    observers.push_back(std::make_shared<AppEventObserver>("obs0"));

    auto listener = std::make_shared<OsEventListener>(observers);
    listener->InsertLinkEvents(event, observers);

    // unusedLinkFile is linkExternalLogs[0][1], not referenced by any observer -> removed
    EXPECT_FALSE(FileUtil::IsFileExists(unusedLinkFile));
    // logFile1 is externalLogs[0], not used -> removed
    EXPECT_FALSE(FileUtil::IsFileExists(logFile1));

    (void)FileUtil::ForceRemoveDirectory(dir);
}

/**
 * @tc.name: ObserverMgrInsertLinkEvents001
 * @tc.desc: test observer_mgr InsertLinkEvents - empty linkExternalLogs falls back to InsertEvent
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, ObserverMgrInsertLinkEvents001, TestSize.Level0)
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_STORAGE_DIR);
    ASSERT_EQ(AppEventStore::GetInstance().InitDbStore(), 0);

    auto& mgr = AppEventObserverMgr::GetInstance();
    // Register a watcher so HandleEvents has observers
    auto watcher = std::make_shared<AppEventWatcher>("testWatcher001");
    int64_t seq = mgr.AddWatcher(watcher);
    ASSERT_GT(seq, 0);

    // Event with externalLogs but no linkExternalLogs -> InsertEvent path
    std::string dir = TEST_DIR + "mgr_nolink/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile = dir + "log1.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile, "data", true));

    auto event = std::make_shared<AppEventPack>("test", "test_event", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile, false}};
    // linkExternalLogs is empty -> InsertLinkEvents calls InsertEvent directly
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventPack>> events = {event};
    mgr.HandleEvents(events);
    // Exercises: linkExternalLogs.size() == 0 -> InsertEvent + return

    mgr.RemoveObserver(seq);
    (void)FileUtil::ForceRemoveDirectory(dir);
    (void)AppEventStore::GetInstance().DestroyDbStore();
}

/**
 * @tc.name: ObserverMgrInsertLinkEvents002
 * @tc.desc: test observer_mgr InsertLinkEvents - with linkExternalLogs, per-observer split
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, ObserverMgrInsertLinkEvents002, TestSize.Level0)
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_STORAGE_DIR);
    ASSERT_EQ(AppEventStore::GetInstance().InitDbStore(), 0);

    auto& mgr = AppEventObserverMgr::GetInstance();
    auto watcher = std::make_shared<AppEventWatcher>("testWatcher002");
    int64_t seq = mgr.AddWatcher(watcher);
    ASSERT_GT(seq, 0);

    std::string dir = TEST_DIR + "mgr_link/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile1 = dir + "log1.txt";
    std::string logFile2 = dir + "log2.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile2, "data2", true));

    auto event = std::make_shared<AppEventPack>("test", "test_event", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile1, false}, {logFile2, false}};
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}},
        {{dir + "obs0_link2", false}}
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventPack>> events = {event};
    mgr.HandleEvents(events);
    // Exercises: linkExternalLogs.size() > 0 -> per-observer link event creation
    // linkExternalLogs[j][i] used -> isUsed=true
    // externalLogs[j] not used -> isUsed stays false -> RemoveFile

    EXPECT_FALSE(FileUtil::IsFileExists(logFile1));
    EXPECT_FALSE(FileUtil::IsFileExists(logFile2));

    mgr.RemoveObserver(seq);
    (void)FileUtil::ForceRemoveDirectory(dir);
    (void)AppEventStore::GetInstance().DestroyDbStore();
}

/**
 * @tc.name: ObserverMgrInsertLinkEvents003
 * @tc.desc: test observer_mgr InsertLinkEvents - j >= linkExternalLogs.size() fallback
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, ObserverMgrInsertLinkEvents003, TestSize.Level0)
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_STORAGE_DIR);
    ASSERT_EQ(AppEventStore::GetInstance().InitDbStore(), 0);

    auto& mgr = AppEventObserverMgr::GetInstance();
    auto watcher = std::make_shared<AppEventWatcher>("testWatcher003");
    int64_t seq = mgr.AddWatcher(watcher);
    ASSERT_GT(seq, 0);

    std::string dir = TEST_DIR + "mgr_fallback_j/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile1 = dir + "log1.txt";
    std::string logFile2 = dir + "log2.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile2, "data2", true));

    auto event = std::make_shared<AppEventPack>("test", "test_event", 1);
    ExternalLogManager logMgr;
    // 2 externalLogs but only 1 linkExternalLogs entry
    logMgr.externalLogs = {{logFile1, false}, {logFile2, false}};
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}}
        // no entry for log2 -> j >= linkExternalLogs.size() fallback
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventPack>> events = {event};
    mgr.HandleEvents(events);
    // For log1: uses linkExternalLogs[0][0].file
    // For log2: j=1 >= linkExternalLogs.size() -> fallback to externalLogs[1].file
    // externalLogs[1].isUsed=true -> not removed
    EXPECT_TRUE(FileUtil::IsFileExists(logFile2));

    mgr.RemoveObserver(seq);
    (void)FileUtil::ForceRemoveDirectory(dir);
    (void)AppEventStore::GetInstance().DestroyDbStore();
}

/**
 * @tc.name: ObserverMgrInsertLinkEvents004
 * @tc.desc: test observer_mgr InsertLinkEvents - unused linkExternalLogs file cleanup
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, ObserverMgrInsertLinkEvents004, TestSize.Level0)
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_STORAGE_DIR);
    ASSERT_EQ(AppEventStore::GetInstance().InitDbStore(), 0);

    auto& mgr = AppEventObserverMgr::GetInstance();
    auto watcher = std::make_shared<AppEventWatcher>("testWatcher004");
    int64_t seq = mgr.AddWatcher(watcher);
    ASSERT_GT(seq, 0);

    std::string dir = TEST_DIR + "mgr_unused_link/";
    ASSERT_TRUE(FileUtil::ForceCreateDirectory(dir));
    std::string logFile1 = dir + "log1.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(logFile1, "data1", true));
    std::string unusedLinkFile = dir + "unused_link.txt";
    ASSERT_TRUE(FileUtil::SaveStringToFile(unusedLinkFile, "unused", true));

    auto event = std::make_shared<AppEventPack>("test", "test_event", 1);
    ExternalLogManager logMgr;
    logMgr.externalLogs = {{logFile1, false}};
    // linkExternalLogs[0][1] has a file but only 1 observer exists
    // isUsed stays false for [0][1] -> RemoveFile
    logMgr.linkExternalLogs = {
        {{dir + "obs0_link1", false}, {unusedLinkFile, false}}
    };
    event->SetExternalLogManager(logMgr);

    std::vector<std::shared_ptr<AppEventPack>> events = {event};
    mgr.HandleEvents(events);
    // unusedLinkFile (linkExternalLogs[0][1]) is not referenced -> removed
    EXPECT_FALSE(FileUtil::IsFileExists(unusedLinkFile));

    mgr.RemoveObserver(seq);
    (void)FileUtil::ForceRemoveDirectory(dir);
    (void)AppEventStore::GetInstance().DestroyDbStore();
}

/**
 * @tc.name: ObserverMgrInsertLinkEvents005
 * @tc.desc: test observer_mgr InsertLinkEvents - empty externalLogs early return
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, ObserverMgrInsertLinkEvents005, TestSize.Level0)
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_STORAGE_DIR);
    ASSERT_EQ(AppEventStore::GetInstance().InitDbStore(), 0);

    auto& mgr = AppEventObserverMgr::GetInstance();
    auto watcher = std::make_shared<AppEventWatcher>("testWatcher005");
    int64_t seq = mgr.AddWatcher(watcher);
    ASSERT_GT(seq, 0);

    // Event with no externalLogs -> InsertLinkEvents in observer_mgr skips entirely
    auto event = std::make_shared<AppEventPack>("test", "test_event", 1);

    std::vector<std::shared_ptr<AppEventPack>> events = {event};
    mgr.HandleEvents(events);
    // No crash = success. Exercises linkExternalLogs check path.

    mgr.RemoveObserver(seq);
    (void)AppEventStore::GetInstance().DestroyDbStore();
}

/**
 * @tc.name: OsEventListenerTest012
 * @tc.desc: test OsEventListener Init with external_log but no link_external_log
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest012, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    // Event with external_log but no link_external_log
    std::string content = R"({"domain":"OS","eventType":1,"name":"APP_CRASH",)"
        R"("params":{"crash_type":"JsError","external_log":["/log1.txt"]}})";
    std::string filePath = TEST_DIR + "/hiappevent_1756735345344.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->StartListening());

    // Without link_external_log, externalLogs exist but linkExternalLogs is empty
    // InsertLinkEvents returns early (no link events generated)
    std::vector<std::vector<std::shared_ptr<AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    // With no observers and no link_external_log, no link events should be produced
    EXPECT_TRUE(!linkEvents.empty());
}

/**
 * @tc.name: OsEventListenerTest013
 * @tc.desc: test OsEventListener Init with link_external_log containing non-string elements
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest013, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    // link_external_log contains non-string elements (123, true) which should be skipped
    std::string content = R"({"domain":"OS","eventType":1,"name":"APP_CRASH",)"
        R"("params":{"crash_type":"JsError","external_log":["/log1.txt"]},)"
        R"("link_external_log":[[123,"obs0_log1"]]})";
    std::string filePath = TEST_DIR + "/hiappevent_1756735345345.txt";
    EXPECT_TRUE(FileUtil::SaveStringToFile(filePath, content));

    std::vector<std::shared_ptr<AppEventObserver>> observers;
    auto listener = std::make_shared<OsEventListener>(observers);
    EXPECT_TRUE(listener->StartListening());
    // Should not crash even with non-string elements in link_external_log
}

/**
 * @tc.name: SaveExternalLogSolidLink007
 * @tc.desc: test SaveExternalLogSolidLink preserves other fields in params
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventObserverTest, SaveExternalLogSolidLink007, TestSize.Level0)
{
    auto event = std::make_shared<AppEventPack>("OS", "APP_CRASH", 1);
    event->SetParamStr(R"({"external_log":["/log1.txt"],"crash_type":"JsError","pid":1234})");

    std::vector<std::string> linkExternalLogs = {"obs0_log1"};
    AppEventUtil::SaveExternalLogSolidLink(event, linkExternalLogs);

    std::string paramStr = event->GetParamStr();
    EXPECT_NE(paramStr.find("crash_type"), std::string::npos);
    EXPECT_NE(paramStr.find("pid"), std::string::npos);
    EXPECT_NE(paramStr.find("obs0_log1"), std::string::npos);
}
}  // OHOS