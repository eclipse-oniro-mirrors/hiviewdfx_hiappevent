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

#include "hiappevent/hiappevent.h"
#include "hiappevent/hiappevent_event.h"
#include "hiappevent_base.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace {
class HiAppEventConfigTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};
}

/**
 * @tc.name: SetConfigItemTest001
 * @tc.desc: check the interface of OH_HiAppEvent_SetConfigItem when config is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetConfigItemTest001, TestSize.Level0)
{
    int res = OH_HiAppEvent_SetConfigItem(nullptr, "testKey", "testValue");
    ASSERT_EQ(res, ErrorCode::ERROR_EVENT_CONFIG_IS_NULL);

    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    res = OH_HiAppEvent_SetConfigItem(config, "testKey", "testValue");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: SetConfigItemTest002
 * @tc.desc: check the interface of OH_HiAppEvent_SetConfigItem with different types of config itemName
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetConfigItemTest002, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();

    int res = OH_HiAppEvent_SetConfigItem(config, "", "testValue");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, "testKey", "testValue");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, nullptr, "testValue");
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetConfigItemTest003
 * @tc.desc: check the interface of OH_HiAppEvent_SetConfigItem with config different length itemName
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetConfigItemTest003, TestSize.Level0)
{
    int maxStrLen = 1024;
    std::string longStr(maxStrLen, 'a');
    std::string longInvalidStr(maxStrLen + 1, 'a');
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();

    int res = OH_HiAppEvent_SetConfigItem(config, longStr.c_str(), "testValue");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, longInvalidStr.c_str(), "testValue");
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetConfigItemTest004
 * @tc.desc: check the interface of OH_HiAppEvent_SetConfigItem with different types of config itemValue
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetConfigItemTest004, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();

    int res = OH_HiAppEvent_SetConfigItem(config, "testKey", "");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, "testKey", "testValue");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, "testKey", nullptr);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetConfigItemTest005
 * @tc.desc: check the interface of OH_HiAppEvent_SetConfigItem with config different length itemValue
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetConfigItemTest005, TestSize.Level0)
{
    int maxStrLen = 1024;
    std::string longStr(maxStrLen, 'a');
    std::string longInvalidStr(maxStrLen + 1, 'a');
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();

    int res = OH_HiAppEvent_SetConfigItem(config, "testName", longStr.c_str());
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, "testName", longInvalidStr.c_str());
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest001
 * @tc.desc: check the interface of SetEventConfig when name is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest001, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    int res = OH_HiAppEvent_SetConfigItem(config, "log_type", "0");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetEventConfig("", config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetEventConfig(nullptr, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetEventConfig("testName", config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    int maxStrLen = 1024;
    std::string longInvalidStr(maxStrLen + 1, 'a');
    res = OH_HiAppEvent_SetEventConfig(longInvalidStr.c_str(), config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest002
 * @tc.desc: check the interface of SetEventConfig for when the config is invalid
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest002, TestSize.Level0)
{
    int res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, nullptr);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest003
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK when the log type is not customizable type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest003, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();

    int res = OH_HiAppEvent_SetConfigItem(config, "log_type", "abc");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetConfigItem(config, "log_type", "");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetConfigItem(config, "log_type", nullptr);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest004
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK with different value of log type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest004, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();

    int res = OH_HiAppEvent_SetConfigItem(config, "log_type", "-1");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetConfigItem(config, "log_type", "100");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetConfigItem(config, "log_type", "0");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, "log_type", "2");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest005
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK when the log type is in format.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest005, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    int res = OH_HiAppEvent_SetConfigItem(config, "log_type", "1");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    res = OH_HiAppEvent_SetConfigItem(config, "ignore_startup_time", "10");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "sample_interval", "100");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "report_times_per_app", "3");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "sample_count", "21");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest006
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK when event params value is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest006, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    int res = OH_HiAppEvent_SetConfigItem(config, "log_type", "1");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "sample_count", "21");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "ignore_startup_time", "10");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "report_times_per_app", "3");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, "sample_interval", "50");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);  // sample_interval range is [50, 1000]
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, "sample_interval", "-1");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    res = OH_HiAppEvent_SetConfigItem(config, "sample_interval", "49");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    res = OH_HiAppEvent_SetConfigItem(config, "sample_interval", "aa");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);
    std::string maxValue = "92233720368547758079223372036854775807";
    res = OH_HiAppEvent_SetConfigItem(config, "sample_interval", maxValue.c_str());
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest007
 * @tc.desc: check the interface of SetEventConfig for EVENT_MAIN_THREAD_JANK_V2 when the log type is in format.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest007, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    int res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_LOG_TYPE, "1");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK_V2, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_IGNORE_STARTUP_TIME, "10");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_SAMPLE_INTERVAL, "100");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_REPORT_TIMES_PER_APP, "3");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_SAMPLE_COUNT, "21");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_AUTO_STOP_SAMPLING, "true");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK_V2, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    OH_HiAppEvent_DestroyConfig(config);
}

/**
 * @tc.name: SetEventConfigTest008
 * @tc.desc: check the interface of SetEventConfig for EVENT_MAIN_THREAD_JANK_V2 with boolean type param.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventConfigTest, SetEventConfigTest008, TestSize.Level0)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    int res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_AUTO_STOP_SAMPLING, "");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK_V2, config);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_SetConfigItem(config, MAIN_THREAD_JANK_PARAM_AUTO_STOP_SAMPLING, "invalidValue");
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_SetEventConfig(EVENT_MAIN_THREAD_JANK_V2, config);
    ASSERT_EQ(res, ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(config);
}
}  // OHOS