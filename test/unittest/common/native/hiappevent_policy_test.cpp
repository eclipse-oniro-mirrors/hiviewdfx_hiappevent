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
#define private public
#include "event_policy_utils.h"
#undef private
#include "file_util.h"
#include "hiappevent_base.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
const std::string TEST_DIR = "/data/test/policy";
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
    EventPolicyUtils::GetInstance().runningId_ = "123456";
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
 * @tc.name: HiAppEventPolicyTest003_1
 * @tc.desc: test the SetEventPolicy func for appCrash with pageSwitchLogEnable.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest003_1, TestSize.Level0)
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
 * @tc.name: HiAppEventPolicyTest003_2
 * @tc.desc: test the SetEventPolicy func for appCrash with valid crash log config.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest003_2, TestSize.Level0)
{
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"extendPcLrPrinting", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"logFileCutoffSzBytes", "1024"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"simplifyVmaPrinting", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"extend_pc_lr_printing", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"log_file_cutoff_sz_bytes", "1024"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"simplify_vma_printing", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"merge_cppcrash_app_log", "true"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest003_3
 * @tc.desc: test the SetEventPolicy func for appCrash with invalid crash log config.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest003_3, TestSize.Level0)
{
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"extendPcLrPrinting", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"logFileCutoffSzBytes", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"simplifyVmaPrinting", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"extend_pc_lr_printing", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"log_file_cutoff_sz_bytes", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"simplify_vma_printing", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"merge_cppcrash_app_log", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", {{"testKey", "!@#$"}});
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: HiAppEventPolicyTest003_4
 * @tc.desc: test the SetEventPolicy func for appCrash with specifications.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest003_4, TestSize.Level0)
{
    std::map<std::string, std::string> configMap = {{"pageSwitchLogEnable", "true"}, {"extendPcLrPrinting", "true"}};
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);

    SetTestContext();

    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    configMap = {{"pageSwitchLogEnable", "true"}, {"extendPcLrPrinting", "!@#$"}};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    configMap = {{"extendPcLrPrinting", "!@#$"}, {"logFileCutoffSzBytes", "1024"}, {"simplifyVmaPrinting", "true"}};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    configMap = {{"extendPcLrPrinting", "!@#$"}, {"logFileCutoffSzBytes", "!@#$"}, {"simplifyVmaPrinting", "!@#$"}};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    configMap = {};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("appCrashPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest004
 * @tc.desc: test the SetEventPolicy func for appFreeze with pageSwitchLogEnable.
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
 * @tc.name: HiAppEventPolicyTest005_1
 * @tc.desc: test the SetEventPolicy func for resourceOverlimit with pageSwitchLogEnable.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest005_1, TestSize.Level0)
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
 * @tc.name: HiAppEventPolicyTest005_2
 * @tc.desc: test the SetEventPolicy func for resourceOverlimit with jsHeapLogtype.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest005_2, TestSize.Level0)
{
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"jsHeapLogtype", "event"}});
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);

    SetTestContext();

    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"jsHeapLogtype", "event"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"js_heap_logtype", "event"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy",
        {{"jsHeapLogtype", "event_rawheap"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy",
        {{"js_heap_logtype", "event_rawheap"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"invalid", "event"}});
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"invalid", "event_rawheap"}});
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"invalid", "invalid too"}});
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);

    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"jsHeapLogtype", "invalid"}});
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", {{"js_heap_logtype", "invalid"}});
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name: HiAppEventPolicyTest005_3
 * @tc.desc: test the SetEventPolicy func for resourceOverlimit with specifications.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventPolicyTest, HiAppEventPolicyTest005_3, TestSize.Level0)
{
    SetTestContext();

    std::map<std::string, std::string> configMap = {{"pageSwitchLogEnable", "true"}, {"jsHeapLogtype", "event"}};
    int result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    configMap = {{"pageSwitchLogEnable123", "true"}, {"jsHeapLogtype", "event"}};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    
    configMap = {{"pageSwitchLogEnable", "true"}, {"jsHeapLogtype", "true"}};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::ERROR_UNKNOWN);

    configMap = {};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("resourceOverlimitPolicy", configMap);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventPolicyTest006
 * @tc.desc: test the SetEventPolicy func for addressSanitizer with pageSwitchLogEnable.
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
    std::map<uint8_t, uint32_t> configMap = {{0, 0}};
    result = EventPolicyMgr::GetInstance().SetEventPolicy("APP_CRASH", configMap);
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
    result = EventPolicyMgr::GetInstance().SetEventPolicy("MAIN_THREAD_JANK", {{"log_type", "0"}});
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
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
}
}  // OHOS