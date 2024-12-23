/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "hiappevent_native_test.h"

#include <cstring>
#include <string>
#include <vector>

#include "application_context.h"
#include "hiappevent/hiappevent.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_test_common.h"
#include "hiappevent_userinfo.h"
#include "ndk_app_event_processor_service.h"
#include "time_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_STORAGE_PATH = "/data/test/hiappevent/";
const char* TEST_DOMAIN_NAME = "test_domain";
const char* TEST_EVENT_NAME = "test_event";
const char* TEST_EVENT_PARAM_KEY = "test_param_key";
const char* TEST_EVENT_PARAM = "{\"test_param_key\":1}";
constexpr int TEST_EVENT_PARAM_LENGTH = 20;
constexpr int TEST_EVENT_NUM = 2;

const char* TEST_PROCESSOR_NAME = "test_processor";
constexpr int32_t TEST_UID = 200000 * 100;

static void WriteEvent()
{
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt16Param(list, TEST_EVENT_PARAM_KEY, 1);
    OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
}

static void OnReceive(const char* domain, const struct HiAppEvent_AppEventGroup* appEventGroups, uint32_t groupSize)
{
    ASSERT_EQ(strcmp(domain, TEST_DOMAIN_NAME), 0);
    ASSERT_EQ(groupSize, 1);
    ASSERT_TRUE(appEventGroups);
    ASSERT_EQ(appEventGroups[0].infoLen, 1);
    ASSERT_TRUE(appEventGroups[0].appEventInfos);
    ASSERT_EQ(strcmp(appEventGroups[0].appEventInfos[0].name, TEST_EVENT_NAME), 0);
    ASSERT_EQ(strcmp(appEventGroups[0].appEventInfos[0].domain, TEST_DOMAIN_NAME), 0);
    ASSERT_EQ(appEventGroups[0].appEventInfos[0].type, SECURITY);
    ASSERT_EQ(strncmp(appEventGroups[0].appEventInfos[0].params, TEST_EVENT_PARAM, TEST_EVENT_PARAM_LENGTH), 0);
}

static void OnTrigger(int32_t row, int32_t size)
{
    ASSERT_EQ(row, TEST_EVENT_NUM);
    ASSERT_GT(size, 0);
}

static void OnTake(const char* const *events, uint32_t eventLen)
{
    ASSERT_TRUE(events != nullptr);
    ASSERT_EQ(eventLen, TEST_EVENT_NUM);
}

std::string GetStorageFilePath()
{
    return "app_event_" + TimeUtil::GetDate() + ".log";
}
}

void HiAppEventNativeTest::SetUpTestCase()
{
    // set app uid
    setuid(TEST_UID);
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_STORAGE_PATH);
    // set context bundle name
    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (context != nullptr) {
        auto contextImpl = std::make_shared<TestContextImpl>("ohos.hiappevent.native.test");
        context->AttachContextImpl(contextImpl);
        std::cout << "set bundle name." << std::endl;
        return;
    }
    std::cout << "context is null." << std::endl;
}

/**
 * @tc.name: HiAppEventNDKTest001
 * @tc.desc: check the logging function
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    bool boolean = true;
    OH_HiAppEvent_AddBoolParam(list, "bool_key", boolean);
    bool booleans[] = {true, false, true};
    OH_HiAppEvent_AddBoolArrayParam(list, "bool_arr_key", booleans, sizeof(booleans) / sizeof(booleans[0]));
    int8_t num1 = 1;
    OH_HiAppEvent_AddInt8Param(list, "int8_key", num1);
    int8_t nums1[] = {1, INT8_MIN, INT8_MAX};
    OH_HiAppEvent_AddInt8ArrayParam(list, "int8_arr_key", nums1, sizeof(nums1) / sizeof(nums1[0]));
    int16_t num2 = 1;
    OH_HiAppEvent_AddInt16Param(list, "int16_key", num2);
    int16_t nums2[] = {1, INT16_MAX, INT16_MIN};
    OH_HiAppEvent_AddInt16ArrayParam(list, "int16_arr_key", nums2, sizeof(nums2) / sizeof(nums2[0]));
    int32_t num3 = 1;
    OH_HiAppEvent_AddInt32Param(list, "int32_key", num3);
    int32_t nums3[] = {1, INT32_MAX, INT32_MIN};
    OH_HiAppEvent_AddInt32ArrayParam(list, "int32_arr_key", nums3, sizeof(nums3) / sizeof(nums3[0]));
    int64_t num4 = 1;
    OH_HiAppEvent_AddInt64Param(list, "int64_key", num4);
    int64_t nums4[] = {1LL, INT64_MAX, INT64_MIN};
    OH_HiAppEvent_AddInt64ArrayParam(list, "int64_arr_key", nums4, sizeof(nums4) / sizeof(nums4[0]));
    float num5 = 465.1234;
    OH_HiAppEvent_AddFloatParam(list, "float_key", num5);
    float nums5[] = {123.22f, num5, 131312.46464f};
    OH_HiAppEvent_AddFloatArrayParam(list, "float_arr_key", nums5, sizeof(nums5) / sizeof(nums5[0]));
    double num6 = 465.1234;
    OH_HiAppEvent_AddDoubleParam(list, "double_key", num6);
    double nums6[] = {123.22, num6, 131312.46464};
    OH_HiAppEvent_AddDoubleArrayParam(list, "double_arr_key", nums6, sizeof(nums6) / sizeof(nums6[0]));
    char str1[] = "hello";
    OH_HiAppEvent_AddStringParam(list, "str_key", str1);
    char str2[] = "world";
    char* strs[] = {str1, str2};
    OH_HiAppEvent_AddStringArrayParam(list, "str_arr_key", strs, sizeof(strs) / sizeof(strs[0]));

    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, BEHAVIOR, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventNDKTest002
 * @tc.desc: check the overwriting function of the same param name.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params with the same name to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    int8_t num1 = 1;
    OH_HiAppEvent_AddInt8Param(list, "int_key", num1);
    int8_t nums1[] = {1, INT8_MIN, INT8_MAX};
    OH_HiAppEvent_AddInt8ArrayParam(list, "int8_arr_key", nums1, sizeof(nums1) / sizeof(nums1[0]));
    int16_t num2 = 1;
    OH_HiAppEvent_AddInt16Param(list, "int16_key", num2);
    int16_t nums2[] = {1, INT16_MAX, INT16_MIN};
    OH_HiAppEvent_AddInt16ArrayParam(list, "int16_key", nums2, sizeof(nums2) / sizeof(nums2[0]));

    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, BEHAVIOR, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res, ErrorCode::ERROR_DUPLICATE_PARAM);
}

/**
 * @tc.name: HiAppEventNDKTest003
 * @tc.desc: check the logging function when the input value is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params with the nullptr value to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, FAULT, nullptr);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    ParamList list = OH_HiAppEvent_CreateParamList();
    bool boolean = true;
    OH_HiAppEvent_AddBoolParam(list, nullptr, boolean);
    OH_HiAppEvent_AddBoolArrayParam(list, "bool_arr_key", nullptr, 0);
    int8_t num1 = 1;
    OH_HiAppEvent_AddInt8Param(list, nullptr, num1);
    OH_HiAppEvent_AddInt8ArrayParam(list, "int8_arr_key", nullptr, 0);
    int16_t num2 = 1;
    OH_HiAppEvent_AddInt16Param(list, nullptr, num2);
    OH_HiAppEvent_AddInt16ArrayParam(list, "int16_arr_key", nullptr, 1);
    int32_t num3 = 1;
    OH_HiAppEvent_AddInt32Param(list, nullptr, num3);
    OH_HiAppEvent_AddInt32ArrayParam(list, "int32_arr_key", nullptr, 2);
    int64_t num4 = 1;
    OH_HiAppEvent_AddInt64Param(list, nullptr, num4);
    OH_HiAppEvent_AddInt64ArrayParam(list, "int64_arr_key", nullptr, 3);
    float num5 = 465.1234;
    OH_HiAppEvent_AddFloatParam(list, nullptr, num5);
    OH_HiAppEvent_AddFloatArrayParam(list, "float_arr_key", nullptr, -1);
    double num6 = 465.1234;
    OH_HiAppEvent_AddDoubleParam(list, nullptr, num6);
    OH_HiAppEvent_AddDoubleArrayParam(list, "double_arr_key", nullptr, 0);
    char str1[] = "hello";
    OH_HiAppEvent_AddStringParam(list, nullptr, str1);
    OH_HiAppEvent_AddStringParam(list, nullptr, nullptr);
    OH_HiAppEvent_AddStringArrayParam(list, "str_arr_key", nullptr, 0);
    char* strs[] = {str1, nullptr};
    OH_HiAppEvent_AddStringArrayParam(list, "str_arr_null_key", strs, sizeof(strs) / sizeof(strs[0]));

    res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, STATISTIC, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventNDKTest004
 * @tc.desc: check the verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params with the invalid name to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    char key1[] = "**";
    int8_t num1 = 1;
    OH_HiAppEvent_AddInt8Param(list, key1, num1);
    char key2[] = "HH22";
    int16_t num2 = 1;
    OH_HiAppEvent_AddInt16Param(list, key2, num2);
    char key3[] = "aa_";
    int32_t num3 = 1;
    OH_HiAppEvent_AddInt32Param(list, key3, num3);
    char key4[] = "";
    int64_t num4 = 1;
    OH_HiAppEvent_AddInt64Param(list, key4, num4);

    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_PARAM_NAME);
}

/**
 * @tc.name: HiAppEventNDKTest005
 * @tc.desc: check the verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params with too long string length to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    int maxStrLen = 8 * 1024;
    std::string longStr(maxStrLen, 'a');
    std::string longInvalidStr(maxStrLen + 1, 'a');
    const char* strs[] = {"hello", longStr.c_str()};
    const char* strIns[] = {"hello", longInvalidStr.c_str()};

    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddStringParam(list, "long_s_key", longStr.c_str());
    OH_HiAppEvent_AddStringArrayParam(list, "long_s_a_key", strs, sizeof(strs) / sizeof(strs[0]));
    OH_HiAppEvent_AddStringParam(list, "long_s_i_key", longInvalidStr.c_str());
    OH_HiAppEvent_AddStringArrayParam(list, "long_s_a_i_key", strIns, sizeof(strIns) / sizeof(strIns[0]));

    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH);
}

/**
 * @tc.name: HiAppEventNDKTest006
 * @tc.desc: check the verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add too many params to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    // max len is 32
    int len = 33;
    std::vector<std::string> keys(len);
    std::vector<std::string> values(len);
    ParamList list = OH_HiAppEvent_CreateParamList();
    for (int i = 0; i < len; i++) {
        keys[i] = "key" + std::to_string(i);
        values[i] = "value" + std::to_string(i);
        OH_HiAppEvent_AddStringParam(list, keys[i].c_str(), values[i].c_str());
    }

    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_PARAM_NUM);
}

/**
 * @tc.name: HiAppEventNDKTest007
 * @tc.desc: check the verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params to the ParamList.
     * @tc.steps: step3. write event with invalid event name to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 1);
    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, "verify_**", SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_NAME);

    list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 2);
    res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, "verify_TEST_", SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_NAME);

    list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 3);
    res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, "", SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_NAME);
}

/**
 * @tc.name: HiAppEventNDKTest008
 * @tc.desc: check the verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params to the ParamList.
     * @tc.steps: step3. write event with nullptr event name to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 1);

    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, nullptr, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_NAME);
}

/**
 * @tc.name: HiAppEventNDKTest009
 * @tc.desc: check the verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest009, TestSize.Level0)
{
    /**
     * @tc.steps: step1. disable the logging function.
     * @tc.steps: step2. create a ParamList pointer.
     * @tc.steps: step3. add params to the ParamList.
     * @tc.steps: step4. write event to the file.
     * @tc.steps: step5. check the result of logging.
     * @tc.steps: step6. destroy the ParamList pointer.
     */
    OH_HiAppEvent_Configure(DISABLE, "true");
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 1);
    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::ERROR_HIAPPEVENT_DISABLE);

    OH_HiAppEvent_Configure(DISABLE, "false");
    list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddStringParam(list, "str_key", "test");
    res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventNDKTest010
 * @tc.desc: check the configuration function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest010, TestSize.Level0)
{
    /**
     * @tc.steps: step1. config with invalid params.
     * @tc.steps: step2. check the result of config.
     */
    bool res = OH_HiAppEvent_Configure(nullptr, nullptr);
    ASSERT_FALSE(res);

    res = OH_HiAppEvent_Configure("key", "true");
    ASSERT_FALSE(res);

    res = OH_HiAppEvent_Configure(DISABLE, "xxx");
    ASSERT_FALSE(res);

    res = OH_HiAppEvent_Configure(MAX_STORAGE, "xxx");
    ASSERT_FALSE(res);

    res = OH_HiAppEvent_Configure("", "100M");
    ASSERT_FALSE(res);
}

/**
 * @tc.name: HiAppEventNDKTest011
 * @tc.desc: check the configuration function of event logging.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest011, TestSize.Level0)
{
    /**
     * @tc.steps: step1. config the storage directory quota of the logging function.
     * @tc.steps: step2. check the result of config.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     */
    bool res = OH_HiAppEvent_Configure(MAX_STORAGE, "1k");
    ASSERT_TRUE(res);

    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 1);
    int result = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(result,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_Configure(MAX_STORAGE, "100M");
    ASSERT_TRUE(res);
}

/**
 * @tc.name: HiAppEventNDKTest012
 * @tc.desc: check the event logging function with predefined events.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest012, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params with predefined param name to the ParamList.
     * @tc.steps: step3. write event with predefined event name to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. destroy the ParamList pointer.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, PARAM_USER_ID, 123);
    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, EVENT_USER_LOGIN, BEHAVIOR, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddStringParam(list, PARAM_USER_ID, "123456");
    res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, EVENT_USER_LOGOUT, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddStringParam(list, PARAM_DISTRIBUTED_SERVICE_NAME, "hiview");
    OH_HiAppEvent_AddStringParam(list, PARAM_DISTRIBUTED_SERVICE_INSTANCE_ID, "123");
    res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, EVENT_DISTRIBUTED_SERVICE_START, SECURITY, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventNDKTest013
 * @tc.desc: check the local file.
 * @tc.type: FUNC
 * @tc.require: AR000GIKMA
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest013, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create a ParamList pointer.
     * @tc.steps: step2. add params to the ParamList.
     * @tc.steps: step3. write event to the file.
     * @tc.steps: step4. check the result of logging.
     * @tc.steps: step5. check the file.
     */
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddInt32Param(list, "int_key", 123);
    int res = OH_HiAppEvent_Write(TEST_DOMAIN_NAME, TEST_EVENT_NAME, BEHAVIOR, list);
    OH_HiAppEvent_DestroyParamList(list);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    sleep(1); // wait 1s for WriteEvent complete
    std::string filePath = TEST_STORAGE_PATH + GetStorageFilePath();
    ASSERT_EQ(access(filePath.c_str(), F_OK), 0);
}

/**
 * @tc.name: HiAppEventNDKTest014
 * @tc.desc: check the domain verification function of event logging.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest014, TestSize.Level0)
{
    int res = OH_HiAppEvent_Write(nullptr, TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    res = OH_HiAppEvent_Write("", TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    constexpr size_t limitLen = 32;
    res = OH_HiAppEvent_Write(std::string(limitLen, 'a').c_str(), TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    res = OH_HiAppEvent_Write(std::string(limitLen + 1, 'a').c_str(), TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    std::string invalidDomain = std::string(limitLen - 1, 'a') + "_";
    res = OH_HiAppEvent_Write(invalidDomain.c_str(), TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    res = OH_HiAppEvent_Write("AAAaaa", TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_Write("abc***", TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    res = OH_HiAppEvent_Write("domain_", TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::ERROR_INVALID_EVENT_DOMAIN);

    res = OH_HiAppEvent_Write("a", TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    res = OH_HiAppEvent_Write("a1", TEST_EVENT_NAME, SECURITY, nullptr);
    ASSERT_EQ(res,  ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventNDKTest015
 * @tc.desc: check the function of OH_HiAppEvent_CreateWatcher.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest015, TestSize.Level0)
{
    ASSERT_TRUE(OH_HiAppEvent_CreateWatcher(nullptr) == nullptr);
    g_onReceiveWatcher = OH_HiAppEvent_CreateWatcher("test_onReceiver_watcher");
    ASSERT_TRUE(g_onReceiveWatcher != nullptr);
    g_onTriggerWatcher = OH_HiAppEvent_CreateWatcher("test_onTrigger_watcher");
    ASSERT_TRUE(g_onTriggerWatcher != nullptr);
}

/**
 * @tc.name: HiAppEventNDKTest016
 * @tc.desc:  check the function of OH_HiAppEvent_SetAppEventFilter.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest016, TestSize.Level0)
{
    const char* filterNames[] =  {TEST_EVENT_NAME};
    const char* filterNamesWithNullptr[] =  {nullptr};
    constexpr int namsLen = 1;
    ASSERT_EQ(OH_HiAppEvent_SetAppEventFilter(nullptr, TEST_DOMAIN_NAME, 0, filterNames, namsLen),
              ErrorCode::ERROR_INVALID_WATCHER);
    ASSERT_EQ(OH_HiAppEvent_SetAppEventFilter(g_onReceiveWatcher, nullptr, 0, filterNames, namsLen),
              ErrorCode::ERROR_INVALID_EVENT_DOMAIN);
    ASSERT_EQ(OH_HiAppEvent_SetAppEventFilter(g_onReceiveWatcher, TEST_DOMAIN_NAME, 0, nullptr, namsLen),
              ErrorCode::ERROR_INVALID_EVENT_NAME);
    ASSERT_EQ(OH_HiAppEvent_SetAppEventFilter(g_onReceiveWatcher, TEST_DOMAIN_NAME, 0, filterNamesWithNullptr, namsLen),
              ErrorCode::ERROR_INVALID_EVENT_NAME);
    ASSERT_EQ(OH_HiAppEvent_SetAppEventFilter(g_onReceiveWatcher, TEST_DOMAIN_NAME, 0, filterNames, namsLen), 0);
    ASSERT_EQ(OH_HiAppEvent_SetAppEventFilter(g_onTriggerWatcher, TEST_DOMAIN_NAME, 0, filterNames, namsLen), 0);
}

/**
 * @tc.name: HiAppEventNDKTest017
 * @tc.desc:  check the function of OH_HiAppEvent_SetWatcherOnReceiver.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest017, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_SetWatcherOnReceive(nullptr, OnReceive), ErrorCode::ERROR_INVALID_WATCHER);
    ASSERT_EQ(OH_HiAppEvent_SetWatcherOnReceive(g_onReceiveWatcher, OnReceive), 0);
}

/**
 * @tc.name: HiAppEventNDKTest018
 * @tc.desc: check the function of OH_HiAppEvent_SetTriggerCondition.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest018, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_SetTriggerCondition(nullptr, TEST_EVENT_NUM, 0, 0), ErrorCode::ERROR_INVALID_WATCHER);
    ASSERT_EQ(OH_HiAppEvent_SetTriggerCondition(g_onTriggerWatcher, TEST_EVENT_NUM, 0, 0), 0);
}

/**
 * @tc.name: HiAppEventNDKTest019
 * @tc.desc:  check the function of OH_HiAppEvent_SetWatcherOnTrigger.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest019, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_SetWatcherOnTrigger(nullptr, OnTrigger), ErrorCode::ERROR_INVALID_WATCHER);
    ASSERT_EQ(OH_HiAppEvent_SetWatcherOnTrigger(g_onTriggerWatcher, OnTrigger), 0);
}

/**
 * @tc.name: HiAppEventNDKTest020
 * @tc.desc:  check the function of OH_HiAppEvent_AddWatcher.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest020, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_AddWatcher(nullptr), ErrorCode::ERROR_INVALID_WATCHER);
    ASSERT_EQ(OH_HiAppEvent_AddWatcher(g_onTriggerWatcher), 0);
    ASSERT_EQ(OH_HiAppEvent_AddWatcher(g_onReceiveWatcher), 0);
    for (int i = 0; i < TEST_EVENT_NUM; ++i) {
        WriteEvent();
    }
}

/**
 * @tc.name: HiAppEventNDKTest021
 * @tc.desc:  check the function of OH_HiAppEvent_TakeWatcherData.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest021, TestSize.Level0)
{
    constexpr uint32_t takeNum = 10;
    sleep(1); // wait 1s for WriteEvent complete
    ASSERT_EQ(OH_HiAppEvent_TakeWatcherData(nullptr, takeNum, OnTake), ErrorCode::ERROR_INVALID_WATCHER);
    ASSERT_EQ(OH_HiAppEvent_TakeWatcherData(g_onTriggerWatcher, takeNum, OnTake), 0);
}

/**
 * @tc.name: HiAppEventNDKTest022
 * @tc.desc: check the function of OH_HiAppEvent_ClearData.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest022, TestSize.Level0)
{
    OH_HiAppEvent_ClearData();
    ASSERT_EQ(HiAppEvent::UserInfo::GetInstance().GetUserIdVersion(), 0);
    ASSERT_EQ(HiAppEvent::UserInfo::GetInstance().GetUserPropertyVersion(), 0);
}

/**
 * @tc.name: HiAppEventNDKTest023
 * @tc.desc: check the function of OH_HiAppEvent_RemoveWatcher.
 * @tc.type: FUNC
 * @tc.require: issueI8OY2U
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest023, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_RemoveWatcher(g_onTriggerWatcher), 0);
    ASSERT_EQ(OH_HiAppEvent_RemoveWatcher(g_onTriggerWatcher), ErrorCode::ERROR_WATCHER_NOT_ADDED);
    ASSERT_EQ(OH_HiAppEvent_RemoveWatcher(g_onReceiveWatcher), 0);
    ASSERT_EQ(OH_HiAppEvent_RemoveWatcher(g_onReceiveWatcher), ErrorCode::ERROR_WATCHER_NOT_ADDED);
    OH_HiAppEvent_DestroyWatcher(g_onTriggerWatcher);
    g_onTriggerWatcher = nullptr;
    OH_HiAppEvent_DestroyWatcher(g_onReceiveWatcher);
    g_onReceiveWatcher = nullptr;
}

/**
 * @tc.name: HiAppEventNDKTest024
 * @tc.desc: check ndk interface of AddProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest024, TestSize.Level0)
{
    setuid(0); // 0 means root uid
    ASSERT_EQ(CreateProcessor(TEST_PROCESSOR_NAME), nullptr);
    ASSERT_EQ(SetReportRoute(nullptr, nullptr, nullptr), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(SetReportPolicy(nullptr, 0, 0, false, false), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(SetReportEvent(nullptr, nullptr, nullptr, false), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(SetCustomConfig(nullptr, nullptr, nullptr), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(SetConfigId(nullptr, 0), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(SetReportUserId(nullptr, nullptr, 0), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(SetReportUserProperty(nullptr, nullptr, 0), ErrorCode::ERROR_NOT_APP);
    ASSERT_EQ(AddProcessor(nullptr), ErrorCode::ERROR_NOT_APP);
    DestoryProcessor(nullptr);
    ASSERT_EQ(RemoveProcessor(0), ErrorCode::ERROR_NOT_APP);

    // set app uid
    setuid(TEST_UID);

    ASSERT_EQ(CreateProcessor(nullptr), nullptr);
    ASSERT_EQ(SetReportRoute(nullptr, nullptr, nullptr), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(SetReportPolicy(nullptr, 0, 0, false, false), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(SetReportEvent(nullptr, nullptr, nullptr, false), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(SetCustomConfig(nullptr, nullptr, nullptr), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(SetConfigId(nullptr, 0), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(SetReportUserId(nullptr, nullptr, 0), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(SetReportUserProperty(nullptr, nullptr, 0), ErrorCode::ERROR_INVALID_PROCESSOR);
    ASSERT_EQ(AddProcessor(nullptr), ErrorCode::ERROR_INVALID_PROCESSOR);
    DestoryProcessor(nullptr);
    ASSERT_EQ(RemoveProcessor(0), ErrorCode::ERROR_PROCESSOR_NOT_ADDED);
}

/**
 * @tc.name: HiAppEventNDKTest025
 * @tc.desc: check ndk interface of AddProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest025, TestSize.Level0)
{
    ASSERT_EQ(CreateProcessor(""), nullptr);
    auto processor = CreateProcessor(TEST_PROCESSOR_NAME);
    ASSERT_TRUE(processor != nullptr);
    ASSERT_EQ(SetReportRoute(processor, nullptr, nullptr), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(SetReportRoute(processor, "", ""), 0);
    ASSERT_EQ(SetReportPolicy(processor, -1, 0, false, false), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(SetReportEvent(processor, nullptr, nullptr, false), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(SetCustomConfig(processor, nullptr, nullptr), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(SetCustomConfig(processor, "", ""), ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH);
    ASSERT_EQ(SetConfigId(processor, -1), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(SetReportUserId(processor, nullptr, 0), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    const char* userStrs[] = {"aaa", ""};
    ASSERT_EQ(SetReportUserId(processor, userStrs, 0), 0);
    ASSERT_EQ(SetReportUserProperty(processor, nullptr, 0), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(SetReportUserProperty(processor, userStrs, 0), 0);
    int64_t seq = AddProcessor(processor);
    ASSERT_GT(seq, 0);
    DestoryProcessor(processor);
    ASSERT_EQ(RemoveProcessor(seq), 0);
}

/**
 * @tc.name: HiAppEventNDKTest026
 * @tc.desc: check ndk interface of AddProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest026, TestSize.Level0)
{
    auto processor = CreateProcessor(TEST_PROCESSOR_NAME);
    ASSERT_TRUE(processor != nullptr);
    ASSERT_EQ(SetReportRoute(processor, "test_appid", "test_routeInfo"), 0);
    ASSERT_EQ(SetReportPolicy(processor, 2, 2, false, false), 0);
    ASSERT_EQ(SetReportEvent(processor, "test_domain", "test_name", false), 0);
    ASSERT_EQ(SetCustomConfig(processor, "str_key", "str_value"), 0);
    ASSERT_EQ(SetConfigId(processor, 1), 0);
    const char* userIds[] = {"test_id"};
    ASSERT_EQ(SetReportUserId(processor, userIds, 1), 0);
    const char* userProperties[] = {"test_property"};
    ASSERT_EQ(SetReportUserProperty(processor, userProperties, 1), 0);
    int64_t seq = AddProcessor(processor);
    ASSERT_GT(seq, 0);
    DestoryProcessor(processor);
    ASSERT_EQ(RemoveProcessor(seq), 0);
}

/**
 * @tc.name: HiAppEventNDKTest027
 * @tc.desc: check the interface of SetEventItem.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest027, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(nullptr, "testName", "testValue"), ErrorCode::ERROR_EVENT_CONFIG_IS_NULL);

    int maxStrLen = 1024;
    std::string longStr(maxStrLen, 'a');
    std::string longInvalidStr(maxStrLen + 1, 'a');
    HiAppEvent_Config* configDemo = OH_HiAppEvent_CreateConfig();
    
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "", "testValue"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, longStr.c_str(), "testValue"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, nullptr, "testValue"), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, longInvalidStr.c_str(), "testValue"),
              ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "testName", ""), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "testName", longStr.c_str()),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "testName", nullptr), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "testName", longInvalidStr.c_str()),
              ErrorCode::ERROR_INVALID_PARAM_VALUE);
    
    OH_HiAppEvent_DestroyConfig(configDemo);
}

/**
 * @tc.name: HiAppEventNDKTest028
 * @tc.desc: check the interface of SetEventConfig when name is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest028, TestSize.Level0)
{
    HiAppEvent_Config* configDemo = OH_HiAppEvent_CreateConfig();
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "0"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig(nullptr, configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("testName", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    int maxStrLen = 1024;
    std::string longInvalidStr(maxStrLen + 1, 'a');
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig(longInvalidStr.c_str(), configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(configDemo);
}

/**
 * @tc.name: HiAppEventNDKTest029
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK when the log type is not customizable.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest029, TestSize.Level0)
{
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", nullptr), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    HiAppEvent_Config* configDemo = OH_HiAppEvent_CreateConfig();
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "-1"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "abc"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", ""), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", nullptr), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "100"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "0"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "2"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    OH_HiAppEvent_DestroyConfig(configDemo);
}

/**
 * @tc.name: HiAppEventNDKTest030
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK when the log type is customizable.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest030, TestSize.Level0)
{
    HiAppEvent_Config* configDemo = OH_HiAppEvent_CreateConfig();
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "1"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "ignore_startup_interval", "10"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_interval", "100"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "report_times_per_app", "3"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_count", "21"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    OH_HiAppEvent_DestroyConfig(configDemo);
}

/**
 * @tc.name: HiAppEventNDKTest031
 * @tc.desc: check the interface of SetEventConfig for MAIN_THREAD_JANK when event params value is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNDKTest031, TestSize.Level0)
{
    HiAppEvent_Config* configDemo = OH_HiAppEvent_CreateConfig();
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "log_type", "1"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_count", "21"), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "ignore_startup_interval", "10"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "report_times_per_app", "3"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_interval", "50"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);  // sample_interval range is [50, 1000]

    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_interval", "-1"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_interval", "49"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_interval", "aa"),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);
    
    std::string maxValue = "92233720368547758079223372036854775807";
    ASSERT_EQ(OH_HiAppEvent_SetConfigItem(configDemo, "sample_interval", maxValue.c_str()),
              ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
    ASSERT_EQ(OH_HiAppEvent_SetEventConfig("MAIN_THREAD_JANK", configDemo), ErrorCode::ERROR_INVALID_PARAM_VALUE);

    OH_HiAppEvent_DestroyConfig(configDemo);
}