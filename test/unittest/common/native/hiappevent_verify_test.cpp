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
 * @tc.desc: check the param when it is Valid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest001, TestSize.Level0)
{
    std::string testName = "testName";
    bool isValid = IsValidUserIdValue(testName);
    EXPECT_TRUE(isValid);
    isValid = IsValidUserPropValue(testName);
    EXPECT_TRUE(isValid);
    isValid = IsValidWatcherName(testName);
    EXPECT_TRUE(isValid);
}

/**
 * @tc.name: HiAppEventVerifyTest002
 * @tc.desc: check the param when it is empty.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest002, TestSize.Level0)
{
    std::string testName = "";
    bool isValid = IsValidUserIdValue(testName);
    EXPECT_FALSE(isValid);
    isValid = IsValidUserPropValue(testName);
    EXPECT_FALSE(isValid);
    isValid = IsValidWatcherName(testName);
    EXPECT_FALSE(isValid);
}

/**
 * @tc.name: HiAppEventVerifyTest003
 * @tc.desc: check the eventType param.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest003, TestSize.Level0)
{
    bool isValid = IsValidEventType(0);  // 1-4: value range of event type
    EXPECT_FALSE(isValid);
    isValid = IsValidEventType(1);
    EXPECT_TRUE(isValid);
}

/**
 * @tc.name: HiAppEventVerifyTest004
 * @tc.desc: check the VerifyCustomEventParam func when hiappevent is disable.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest004, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    bool setRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "true");
    EXPECT_TRUE(setRes);
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_HIAPPEVENT_DISABLE);
    setRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    EXPECT_TRUE(setRes);
}

/**
 * @tc.name: HiAppEventVerifyTest005
 * @tc.desc: check the VerifyCustomEventParam func with different domain.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest005, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("a%$321");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_EVENT_DOMAIN);
}

/**
 * @tc.name: HiAppEventVerifyTest006
 * @tc.desc: check the VerifyCustomEventParam func with different name.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest006, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");

    event->SetName("");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    event->SetName("123456");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_EVENT_NAME);

    event->SetName("testName");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventVerifyTest007
 * @tc.desc: check the VerifyCustomEventParam func with different size of params.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest007, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    event->AddParam("testParam");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    for (int i = 0; i < 64; i++) {
        event->AddParam(std::to_string(i));
    }
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM);
}

/**
 * @tc.name: HiAppEventVerifyTest008
 * @tc.desc: check the VerifyAppCustomEventParam func with repeated paramName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest008, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    event->AddParam("testParam");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    event->AddParam("testParam", "testValue");
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventVerifyTest009
 * @tc.desc: check the VerifyAppCustomEventParam func with invalid paramName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest009, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    event->AddParam("a%$321");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_NAME);
}

/**
 * @tc.name: HiAppEventVerifyTest010
 * @tc.desc: check the VerifyAppCustomEventParam func with Invalid string parameter value length.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest010, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    std::string invalidValue(1025, 'a');
    event->AddParam("testParam", invalidValue);
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH);
}

/**
 * @tc.name: HiAppEventVerifyTest011
 * @tc.desc: check the VerifyAppCustomEventParam func with empty strVec.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest011, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    std::vector<std::string> strs;
    event->AddParam("testVectorParam", strs);
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventVerifyTest012
 * @tc.desc: check the VerifyAppCustomEventParam func with strVec item number is over size.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest012, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    std::vector<std::string> strs(1025, "testValue");
    event->AddParam("testVectorParam", strs);
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH);
}

/**
 * @tc.name: HiAppEventVerifyTest013
 * @tc.desc: check the VerifyAppCustomEventParam func with valid param.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest013, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    event->AddParam("testKey", "testValue\\\1\2\b3\f4\n5\r6\t7");
    int result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    std::vector<std::string> strs(3, "testStr");
    event->AddParam("testVectorParam", strs);
    result = VerifyCustomEventParams(event);
    EXPECT_EQ(result, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventVerifyTest014
 * @tc.desc: check the VerifyAppEvent func with param item number out of range.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest014, TestSize.Level0)
{
    std::shared_ptr<AppEventPack> event = std::make_shared<AppEventPack>();
    event->SetDomain("testDomain");
    event->SetName("testName");

    std::vector<bool> bs(101, true);
    std::vector<char> cs(101, 'a');
    std::vector<int16_t> shs(101, 1);
    std::vector<int> is(101, 1);
    std::vector<int64_t> lls(101, 1);
    std::vector<float> fs(101, 1.1);
    std::vector<double> ds(101, 1.1);
    std::vector<std::string> strs(101, "testStr");

    event->AddParam("testBoolVec", bs);
    event->AddParam("testCharVec", cs);
    event->AddParam("testInt16Vec", shs);
    event->AddParam("testIntVec", is);
    event->AddParam("testInt64Vec", lls);
    event->AddParam("testFloatVec", fs);
    event->AddParam("testDoubleVec", ds);
    event->AddParam("testStrVec", strs);
    int result = VerifyAppEvent(event);
    EXPECT_EQ(result, ErrorCode::ERROR_INVALID_LIST_PARAM_SIZE);
}

/**
 * @tc.name: HiAppEventVerifyTest015
 * @tc.desc: check the IsApp func.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest015, TestSize.Level0)
{
    bool isApp = IsApp();
    EXPECT_FALSE(isApp);
}

/**
 * @tc.name: HiAppEventVerifyTest016
 * @tc.desc: check the IsValidProcessorName func.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventVerifyTest, HiAppEventVerifyTest016, TestSize.Level0)
{
    EXPECT_TRUE(IsValidProcessorName("name"));
    EXPECT_TRUE(IsValidProcessorName("_name"));
    EXPECT_TRUE(IsValidProcessorName("$name"));
    EXPECT_FALSE(IsValidProcessorName("1name"));
    EXPECT_TRUE(IsValidProcessorName("n_ame"));
    EXPECT_TRUE(IsValidProcessorName("n$ame"));
}