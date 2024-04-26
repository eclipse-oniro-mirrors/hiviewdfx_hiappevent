/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "app_event.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::ErrorCode;
using namespace OHOS::HiviewDFX::HiAppEvent;

namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";
const std::string TEST_DOMAIN = "test_domain";
const std::string TEST_NAME = "test_domain";
const EventType TEST_TYPE = FAULT;
constexpr int32_t TEST_INT_VALUE = 1;
const std::string TEST_STR_VALUE = "test_value";

class HiAppEventAppEventTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventAppEventTest::SetUp()
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
}

void HiAppEventAppEventTest::TearDown()
{}
}

/**
 * @tc.name: HiAppEventAppEventTest001
 * @tc.desc: Test the writing of normal data.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest001, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest001 start" << std::endl;

    Event event(TEST_DOMAIN, TEST_NAME, FAULT);
    event.AddParam("Bool_key", true);
    constexpr int32_t num1 = 1;
    event.AddParam("int32_key", num1);
    constexpr int64_t num2 = 1;
    event.AddParam("$int64_key", num2);
    constexpr double num3 = 123.456;
    event.AddParam("dou_key1", num3);
    event.AddParam("str_key", TEST_STR_VALUE);

    const std::vector<bool> bs = {false, true};
    event.AddParam("bools_key", bs);
    const std::vector<int32_t> num1s = {INT32_MAX, INT32_MIN};
    event.AddParam("int32s_key", num1s);
    const std::vector<int64_t> num2s = {INT64_MAX, INT64_MIN};
    event.AddParam("$int64s_key", num2s);
    const std::vector<double> num3s = {123.456, 234.67890123};
    event.AddParam("double_key", num3s);
    const std::vector<std::string> strs = {TEST_STR_VALUE, TEST_STR_VALUE};
    event.AddParam("strs_key", strs);

    int ret = Write(event);
    ASSERT_EQ(ret, HIAPPEVENT_VERIFY_SUCCESSFUL);

    std::cout << "HiAppEventAppEventTest001 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest002
 * @tc.desc: Test the writing of invalid param name.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest002, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest001 start" << std::endl;

    Event event1(TEST_DOMAIN, TEST_NAME, FAULT);
    event1.AddParam("**", TEST_INT_VALUE);
    ASSERT_EQ(Write(event1), ERROR_INVALID_PARAM_NAME);
    Event event2(TEST_DOMAIN, TEST_NAME, STATISTIC);
    event2.AddParam("aa_", TEST_INT_VALUE);
    ASSERT_EQ(Write(event2), ERROR_INVALID_PARAM_NAME);
    Event event3(TEST_DOMAIN, TEST_NAME, SECURITY);
    event3.AddParam("", TEST_INT_VALUE);
    ASSERT_EQ(Write(event3), ERROR_INVALID_PARAM_NAME);
    Event event4(TEST_DOMAIN, TEST_NAME, BEHAVIOR);
    event4.AddParam("123AAA", TEST_INT_VALUE);
    ASSERT_EQ(Write(event4), ERROR_INVALID_PARAM_NAME);

    constexpr size_t limitLen = 32;
    Event event5(TEST_DOMAIN, TEST_NAME, BEHAVIOR);
    event5.AddParam(std::string(limitLen, 'a'), TEST_INT_VALUE);
    ASSERT_EQ(Write(event5), HIAPPEVENT_VERIFY_SUCCESSFUL);
    Event event6(TEST_DOMAIN, TEST_NAME, BEHAVIOR);
    event6.AddParam(std::string(limitLen + 1, 'a'), TEST_INT_VALUE);
    ASSERT_EQ(Write(event6), ERROR_INVALID_PARAM_NAME);
    Event event7(TEST_DOMAIN, TEST_NAME, BEHAVIOR);
    event7.AddParam(std::string(limitLen - 1, 'a') + "_", TEST_INT_VALUE);
    ASSERT_EQ(Write(event7), ERROR_INVALID_PARAM_NAME);

    std::cout << "HiAppEventAppEventTest002 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest003
 * @tc.desc: Test the writing of invalid length string.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest003, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest003 start" << std::endl;

    constexpr size_t maxStrLen = 8 * 1024;
    std::string longStr(maxStrLen, 'a');
    Event event1(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    event1.AddParam("str_key", longStr);
    ASSERT_EQ(Write(event1), HIAPPEVENT_VERIFY_SUCCESSFUL);

    std::string longInvalidStr(maxStrLen + 1, 'a');
    Event event2(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    event2.AddParam("str_key", longInvalidStr);
    ASSERT_EQ(Write(event2), ERROR_INVALID_PARAM_VALUE_LENGTH);

    Event event3(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    const std::vector<std::string> strs1 = {longStr};
    event3.AddParam("strs_key", strs1);
    ASSERT_EQ(Write(event3), HIAPPEVENT_VERIFY_SUCCESSFUL);

    Event event4(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    const std::vector<std::string> strs2 = {longInvalidStr};
    event4.AddParam("strs_key", strs2);
    ASSERT_EQ(Write(event4), ERROR_INVALID_PARAM_VALUE_LENGTH);

    std::cout << "HiAppEventAppEventTest003 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest004
 * @tc.desc: Test the writing of invalid number of params.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest004, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest004 start" << std::endl;

    constexpr size_t limitLen = 32;
    std::vector<std::string> keys(limitLen + 1);
    std::vector<std::string> values(limitLen + 1);
    Event event1(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    for (size_t i = 0; i <= limitLen; i++) {
        keys[i] = "key" + std::to_string(i);
        values[i] = "value" + std::to_string(i);
        event1.AddParam(keys[i], values[i]);
    }
    ASSERT_EQ(Write(event1), ERROR_INVALID_PARAM_NUM);

    Event event2(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event2), HIAPPEVENT_VERIFY_SUCCESSFUL);

    Event event3(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    for (size_t i = 0; i < limitLen; i++) {
        event3.AddParam(keys[i], values[i]);
    }
    ASSERT_EQ(Write(event3), HIAPPEVENT_VERIFY_SUCCESSFUL);

    std::cout << "HiAppEventAppEventTest004 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest005
 * @tc.desc: Test the writing of invalid event name.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest005, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest005 start" << std::endl;

    Event event(TEST_DOMAIN, "$event", TEST_TYPE);
    ASSERT_EQ(Write(event), HIAPPEVENT_VERIFY_SUCCESSFUL);

    Event event1(TEST_DOMAIN, "verify_**", TEST_TYPE);
    ASSERT_EQ(Write(event1), ERROR_INVALID_EVENT_NAME);
    Event event2(TEST_DOMAIN, "aa_", TEST_TYPE);
    ASSERT_EQ(Write(event1), ERROR_INVALID_EVENT_NAME);
    Event event3(TEST_DOMAIN, "123AAA", TEST_TYPE);
    ASSERT_EQ(Write(event3), ERROR_INVALID_EVENT_NAME);
    Event event4(TEST_DOMAIN, "", TEST_TYPE);
    ASSERT_EQ(Write(event4), ERROR_INVALID_EVENT_NAME);

    constexpr size_t limitLen = 48;
    Event event5(TEST_DOMAIN, std::string(limitLen, 'a'), BEHAVIOR);
    ASSERT_EQ(Write(event5), HIAPPEVENT_VERIFY_SUCCESSFUL);
    Event event6(TEST_DOMAIN, std::string(limitLen + 1, 'a'), BEHAVIOR);
    ASSERT_EQ(Write(event6), ERROR_INVALID_EVENT_NAME);

    std::cout << "HiAppEventAppEventTest005 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest006
 * @tc.desc: Test the writing of invalid domain name.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest006, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest006 start" << std::endl;

    Event event("domain_123_a", TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event), HIAPPEVENT_VERIFY_SUCCESSFUL);

    Event event1("Domain", TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event1), HIAPPEVENT_VERIFY_SUCCESSFUL);
    Event event2("123_domain", TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event2), ERROR_INVALID_EVENT_DOMAIN);
    Event event3("domain**", TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event3), ERROR_INVALID_EVENT_DOMAIN);
    Event event4("domain_", TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event4), ERROR_INVALID_EVENT_DOMAIN);

    constexpr size_t limitLen = 32;
    Event event5(std::string(limitLen, 'a'), TEST_NAME, BEHAVIOR);
    ASSERT_EQ(Write(event5), HIAPPEVENT_VERIFY_SUCCESSFUL);
    Event event6(std::string(limitLen + 1, 'a'), TEST_NAME, BEHAVIOR);
    ASSERT_EQ(Write(event6), ERROR_INVALID_EVENT_DOMAIN);
    Event event7("", TEST_NAME, BEHAVIOR);
    ASSERT_EQ(Write(event7), ERROR_INVALID_EVENT_DOMAIN);
    Event event8(std::string(limitLen - 1, 'a') + "_", TEST_NAME, BEHAVIOR);
    ASSERT_EQ(Write(event8), ERROR_INVALID_EVENT_DOMAIN);

    std::cout << "HiAppEventAppEventTest006 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest007
 * @tc.desc: Test the writing of invalid vector size.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest007, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest007 start" << std::endl;

    Event event1(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    constexpr size_t limitSize = 100;
    std::vector<int32_t> nums1(limitSize, TEST_INT_VALUE);
    event1.AddParam("valid_vec", nums1);
    ASSERT_EQ(Write(event1), HIAPPEVENT_VERIFY_SUCCESSFUL);

    Event event2(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    std::vector<int32_t> nums2(limitSize + 1, TEST_INT_VALUE);
    event2.AddParam("invalid_vec", nums2);
    ASSERT_EQ(Write(event2), ERROR_INVALID_LIST_PARAM_SIZE);

    std::cout << "HiAppEventAppEventTest007 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest008
 * @tc.desc: Test the writing of logging function is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest008, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest008 start" << std::endl;

    HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    Event event(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    ASSERT_EQ(Write(event), HIAPPEVENT_VERIFY_SUCCESSFUL);

    HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "true");
    ASSERT_EQ(Write(event), ERROR_HIAPPEVENT_DISABLE);

    HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    ASSERT_EQ(Write(event), HIAPPEVENT_VERIFY_SUCCESSFUL);

    std::cout << "HiAppEventAppEventTest008 end" << std::endl;
}

/**
 * @tc.name: HiAppEventAppEventTest009
 * @tc.desc: Test the writing of duplicate param.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventAppEventTest, HiAppEventAppEventTest009, TestSize.Level1)
{
    std::cout << "HiAppEventAppEventTest009 start" << std::endl;

    Event event1(TEST_DOMAIN, TEST_NAME, TEST_TYPE);
    std::vector<std::string> strs = {TEST_STR_VALUE};
    event1.AddParam("str_key", strs);
    ASSERT_EQ(Write(event1), HIAPPEVENT_VERIFY_SUCCESSFUL);
    event1.AddParam("str_key", TEST_STR_VALUE);
    ASSERT_EQ(Write(event1), ERROR_DUPLICATE_PARAM);

    std::cout << "HiAppEventAppEventTest009 end" << std::endl;
}
