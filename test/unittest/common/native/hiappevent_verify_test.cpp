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
#include <iostream>

#include <gtest/gtest.h>

#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_verify.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";

class HiAppEventVerifyTest : public testing::Test {
public:
    void SetUp();
    void TearDown() {}
};

void HiAppEventVerifyTest::SetUp()
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
}
}

/**
 * @tc.name: HiAppEventVerifyTest001
 * @tc.desc: check the param isValid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest001, TestSize.Level0)
{
    std::cout << "HiAppEventVerifyTest001 start" << std::endl;
    std::string testName = "testName";
    bool isValid = IsValidUserIdValue(testName);
    EXPECT_TRUE(isValid);
    isValid = IsValidUserPropValue(testName);
    EXPECT_TRUE(isValid);
    isValid = IsValidWatcherName(testName);
    EXPECT_TRUE(isValid);

    testName = "";
    isValid = IsValidUserIdValue(testName);
    EXPECT_FALSE(isValid);
    isValid = IsValidUserPropValue(testName);
    EXPECT_FALSE(isValid);
    isValid = IsValidWatcherName(testName);
    EXPECT_FALSE(isValid);

    isValid = IsValidEventType(0);  // 1-4: value range of event type
    EXPECT_FALSE(isValid);
    isValid = IsValidEventType(1);
    EXPECT_TRUE(isValid);
    std::cout << "HiAppEventVerifyTest001 end" << std::endl;
}

/**
 * @tc.name: HiAppEventVerifyTest002
 * @tc.desc: check the VerifyCustomEventParam func.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest002, TestSize.Level0)
{
    std::cout << "HiAppEventVerifyTest002 start" << std::endl;
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    bool setRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "true");
    EXPECT_TRUE(setRes);
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_HIAPPEVENT_DISABLE);

    setRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    EXPECT_TRUE(setRes);
    event->SetDomain("a%$321");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    event->SetDomain("testDomain");
    event->SetName("123456");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_EVENT_NAME);

    event->SetName("testName");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    event->AddParam("testParam");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    for (int i = 0; i < 64; i++) {
        event->AddParam(std::to_string(i));
    }
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM);
    std::cout << "HiAppEventVerifyTest002 end" << std::endl;
}

/**
 * @tc.name: HiAppEventVerifyTest003
 * @tc.desc: check the VerifyAppCustomEventParam func.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest003, TestSize.Level0)
{
    std::cout << "HiAppEventVerifyTest003 start" << std::endl;
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    bool setRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    EXPECT_TRUE(setRes);
    event->SetDomain("testDomain");
    event->SetName("testName");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    event->AddParam("testKey", "testValue\\1\2\b3\f4\n5\r6\t7");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    std::vector<std::string> strs(3, "testStr");
    event->AddParam("testVectorParam", strs);
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    event->AddParam("testParam");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    event->AddParam("testParam");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, 0);
    std::cout << "HiAppEventVerifyTest003 end" << std::endl;
}

/**
 * @tc.name: HiAppEventVerifyTest004
 * @tc.desc: check the VerifyAppCustomEventParam func.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest004, TestSize.Level0)
{
    std::cout << "HiAppEventVerifyTest004 start" << std::endl;
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    bool setRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    EXPECT_TRUE(setRes);
    event->SetDomain("testDomain");
    event->SetName("testName");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    event->AddParam("a%$321");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_NAME);
    std::cout << "HiAppEventVerifyTest004 end" << std::endl;
}