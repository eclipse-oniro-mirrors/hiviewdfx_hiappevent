/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "app_event_watcher.h"
#include "application_context.h"
#include "file_util.h"
#include "hiappevent_config.h"
#include "os_event_listener.h"
#include "time_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
const std::string TEST_DIR = "/data/test/observer/hiappevent";
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
 * @tc.name: OsEventListenerTest001
 * @tc.desc: test OsEventListener Init func
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest001, TestSize.Level0)
{
    auto listenerWithContextIsNullptr = std::make_shared<OsEventListener>();

    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillOnce(::testing::Return(""))
        .WillRepeatedly(::testing::Return("testDir"));
    g_applicationContext.reset(contextMock);

    auto listenerWithCacheDirIsEmpty = std::make_shared<OsEventListener>();
    auto listenerWithPathIsNotExist = std::make_shared<OsEventListener>();
    std::vector<std::shared_ptr<AppEventPack>> event;
    listenerWithPathIsNotExist->GetEvents(event);
    EXPECT_TRUE(event.empty());
}

/**
 * @tc.name: OsEventListenerTest002
 * @tc.desc: test OsEventListener Init func when osEventPath_ is exist
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventObserverTest, OsEventListenerTest002, TestSize.Level0)
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return("/data/test/observer"));
    g_applicationContext.reset(contextMock);

    auto validListener = std::make_shared<OsEventListener>();
    std::vector<std::shared_ptr<AppEventPack>> event;
    validListener->GetEvents(event);
    EXPECT_TRUE(event.empty());
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

    auto listener = std::make_shared<OsEventListener>();
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

    auto listener = std::make_shared<OsEventListener>();
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

    auto listener = std::make_shared<OsEventListener>();
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

    auto listener = std::make_shared<OsEventListener>();
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

    auto listener = std::make_shared<OsEventListener>();
    EXPECT_TRUE(listener->StartListening());
    uint64_t curTime = TimeUtil::GetMilliseconds();
    while (TimeUtil::GetMilliseconds() - curTime < 1000) {}  // ensure open file success
    std::vector<std::shared_ptr<AppEventPack>> event;
    listener->GetEvents(event);
    EXPECT_EQ(event.size(), 1);
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

    auto listener = std::make_shared<OsEventListener>();
    EXPECT_TRUE(listener->StartListening());
    uint64_t curTime = TimeUtil::GetMilliseconds();
    while (TimeUtil::GetMilliseconds() - curTime < 1000) {}  // ensure open file success
    std::vector<std::shared_ptr<AppEventPack>> event;
    listener->GetEvents(event);
    EXPECT_EQ(event.size(), 4);
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
}  // OHOS