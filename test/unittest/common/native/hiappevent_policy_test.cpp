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

#include "application_context.h"
#include "event_policy_mgr.h"
#include "file_util.h"
#include "hiappevent_base.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
const std::string TEST_DIR = "/data/test/policy";
const std::string TEST_RUNNING_ID = "123456";
const std::string XATTR_NAME = "user.appevent";
std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> g_applicationContext = nullptr;

class ApplicationContextMock : public ApplicationContext {
public:
    MOCK_METHOD0(GetCacheDir, std::string());
};
}

namespace AbilityRuntime {
std::shared_ptr<ApplicationContext> Context::GetApplicationContext()
{
    return g_applicationContext;
}
}  // namespace AbilityRuntime

void SetTestContext()
{
    ApplicationContextMock* contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .WillRepeatedly(::testing::Return(TEST_DIR));
    g_applicationContext.reset(contextMock);
}

class HiAppEventPolicyTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventPolicyTest::SetUp()
{
    (void)FileUtil::ForceCreateDirectory(TEST_DIR);
    EventPolicyMgr::GetInstance().SetRunningId(TEST_RUNNING_ID);
}

void HiAppEventPolicyTest::TearDown()
{
    g_applicationContext = nullptr;
    (void)FileUtil::ForceRemoveDirectory(TEST_DIR);
}
 
/**
 * @tc.name: HiAppEventPolicyTest001
 * @tc.desc: test the SetEventPolicy func when folder is invalid.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest001, TestSize.Level0)
{
    std::map<std::string, std::string> configMap = {{"pageSwitchLogEnable", "true"}};
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appFreezePolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("addressSanitizerPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name: HiAppEventPolicyTest002
 * @tc.desc: test the SetEventPolicy func when folder is ok.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest002, TestSize.Level0)
{
    SetTestContext();

    std::map<std::string, std::string> configMap = {{"pageSwitchLogEnable", "true"}};
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appFreezePolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("addressSanitizerPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest003
 * @tc.desc: test the SetEventPolicy func for appCrash.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest003, TestSize.Level0)
{
    SetTestContext();

    bool status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_FALSE(status);
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_TRUE(status);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"pageSwitchLogEnable", "false"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_FALSE(status);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("invalidParams");
    EXPECT_FALSE(status);
}

/**
 * @tc.name: HiAppEventPolicyTest004
 * @tc.desc: test the SetEventPolicy func for appFreeze.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest004, TestSize.Level0)
{
    SetTestContext();

    bool status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_FREEZE");
    EXPECT_FALSE(status);
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appFreezePolicy", {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_FREEZE");
    EXPECT_TRUE(status);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appFreezePolicy", {{"pageSwitchLogEnable", "false"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_FREEZE");
    EXPECT_FALSE(status);
}

/**
 * @tc.name: HiAppEventPolicyTest005
 * @tc.desc: test the SetEventPolicy func for resourceOverlimit.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest005, TestSize.Level0)
{
    SetTestContext();

    bool status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("RESOURCE_OVERLIMIT");
    EXPECT_FALSE(status);
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy",
        {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("RESOURCE_OVERLIMIT");
    EXPECT_TRUE(status);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy",
        {{"pageSwitchLogEnable", "false"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("RESOURCE_OVERLIMIT");
    EXPECT_FALSE(status);
}

/**
 * @tc.name: HiAppEventPolicyTest006
 * @tc.desc: test the SetEventPolicy func for addressSanitizer.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest006, TestSize.Level0)
{
    SetTestContext();

    bool status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("ADDRESS_SANITIZER");
    EXPECT_FALSE(status);
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("addressSanitizerPolicy",
        {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("ADDRESS_SANITIZER");
    EXPECT_TRUE(status);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("addressSanitizerPolicy", {{"pageSwitchLogEnable", "false"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("ADDRESS_SANITIZER");
    EXPECT_FALSE(status);
}

/**
 * @tc.name: HiAppEventPolicyTest007
 * @tc.desc: test the SetEventPolicy func for cpuUsageHigh.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest007, TestSize.Level0)
{
    SetTestContext();

    int result = EventPolicyMgr::GetInstance().SetEventPolicy("cpuUsageHighPolicy",
        {{"foregroundLoadThreshold", "11"}, {"backgroundLoadThreshold", "22"}, {"threadLoadThreshold", "33"},
        {"perfLogCaptureCount", "44"}, {"threadLoadInterval", "55"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest008
 * @tc.desc: test the SetEventPolicy func for different event.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest008, TestSize.Level0)
{
    SetTestContext();

    bool status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_FALSE(status);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("ADDRESS_SANITIZER");
    EXPECT_FALSE(status);
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_TRUE(status);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("ADDRESS_SANITIZER");
    EXPECT_FALSE(status);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"pageSwitchLogEnable", "false"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_FALSE(status);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("ADDRESS_SANITIZER");
    EXPECT_FALSE(status);
}

/**
 * @tc.name: HiAppEventPolicyTest009
 * @tc.desc: test the SetEventPolicy func with invalid event.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest009, TestSize.Level0)
{
    SetTestContext();

    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("testPolicy", {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: HiAppEventPolicyTest010
 * @tc.desc: test the SetEventPolicy func with invalid params.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest010, TestSize.Level0)
{
    SetTestContext();

    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"invalidParam", "true"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy({{0, 0}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest011
 * @tc.desc: test the SetEventPolicy func with mainThreadJankPolicy.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest011, TestSize.Level0)
{
    SetTestContext();

    int result = EventPolicyMgr::GetInstance().SetEventPolicy("MAIN_THREAD_JANK_V2", {{"logType", "0"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("mainThreadJankPolicy", {{"logType", "2"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest012
 * @tc.desc: test the GetRunningId/SetRunningId func.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest012, TestSize.Level0)
{
    std::string res = EventPolicyMgr::GetInstance().GetRunningId();
    EXPECT_EQ(res, TEST_RUNNING_ID);
    EventPolicyMgr::GetInstance().SetRunningId("xxxxxx");
    res = EventPolicyMgr::GetInstance().GetRunningId();
    EXPECT_EQ(res, "xxxxxx");
}

/**
 * @tc.name: HiAppEventPolicyTest013
 * @tc.desc: test the GetEventPageSwitchStatus func with specifications.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest013, TestSize.Level0)
{
    SetTestContext();

    bool status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_FALSE(status);
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"pageSwitchLogEnable", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_TRUE(status);
    EventPolicyMgr::GetInstance().SetRunningId("xxxxxx");
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_TRUE(status);
    status = EventPolicyMgr::GetInstance().GetEventPageSwitchStatus("APP_CRASH");
    EXPECT_FALSE(status);
}
}  // OHOS