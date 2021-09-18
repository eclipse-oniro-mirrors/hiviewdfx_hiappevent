/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <climits>
#include <string>
#include <vector>

#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_pack.h"
#include "hiappevent_verify.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
    const std::string TEST_EVENT_NAME = "test_event";
    const std::string TEST_KEY_NAME = "test_key";
    constexpr int TEST_EVENT_TYPE = 2;
}

/**
 * @tc.name: HiAppEventNativeVerifyTest001
 * @tc.desc: check the event verification function
 * @tc.type: FUNC
 * @tc.require: AR000G2EJR
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNativeVerifyTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. build AppEventPack object.
     * @tc.steps: step2. check the result of event verification.
     */
    std::shared_ptr<AppEventPack> event = CreateEventPack(TEST_EVENT_NAME, TEST_EVENT_TYPE);

    // add base type params to AppEventPack
    AddEventParam(event, TEST_KEY_NAME);
    bool tbValue = true;
    AddEventParam(event, TEST_KEY_NAME, tbValue);
    bool fbValue = false;
    AddEventParam(event, TEST_KEY_NAME, fbValue);
    char cValue = 'c';
    AddEventParam(event, TEST_KEY_NAME, cValue);
    short sValue = SHRT_MIN;
    AddEventParam(event, TEST_KEY_NAME, sValue);
    int iValue = INT_MAX;
    AddEventParam(event, TEST_KEY_NAME, iValue);
    long lValue = LONG_MIN;
    AddEventParam(event, TEST_KEY_NAME, lValue);
    long long  llValue = LLONG_MAX;
    AddEventParam(event, TEST_KEY_NAME, llValue);
    double dValue = DBL_MIN;
    AddEventParam(event, TEST_KEY_NAME, dValue);
    std::string strValue = "sValue";
    AddEventParam(event, TEST_KEY_NAME, strValue);
    const char* cpValue = "cpValue";
    AddEventParam(event, TEST_KEY_NAME, cpValue);

    // add vector type params to AppEventPack
    std::vector<bool> bVec = {true, false};
    AddEventParam(event, TEST_KEY_NAME, bVec);
    std::vector<char> cVec = {'a', 'c'};
    AddEventParam(event, TEST_KEY_NAME, cVec);
    std::vector<short> sVec = {(short)1, (short)2};
    AddEventParam(event, TEST_KEY_NAME, sVec);
    std::vector<int> iVec = {1, 2, 3};
    AddEventParam(event, TEST_KEY_NAME, iVec);
    std::vector<long> lVec = {(long)3, (long)4};
    AddEventParam(event, TEST_KEY_NAME, lVec);
    std::vector<long long>  llVec = {(long long)5, (long long)6};
    AddEventParam(event, TEST_KEY_NAME, llVec);
    std::vector<double> dVec = {1.1f, 2.2f};
    AddEventParam(event, TEST_KEY_NAME, dVec);
    std::vector<const std::string> strVec = {"value1", "value2"};
    AddEventParam(event, TEST_KEY_NAME, strVec);
    std::vector<const char*> cpVec = {"value3", "value4"};
    AddEventParam(event, TEST_KEY_NAME, cpVec);

    // check verification result
    int verifyRes = VerifyAppEvent(event);
    ASSERT_EQ(verifyRes, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: HiAppEventNativeVerifyTest002
 * @tc.desc: check the event verification function
 * @tc.type: FUNC
 * @tc.require: AR000G2EJR
 */
HWTEST_F(HiAppEventNativeTest, HiAppEventNativeVerifyTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. build AppEventPack object.
     * @tc.steps: step2. disabling application event logging.
     * @tc.steps: step3. check the result of event verification.
     */
    std::shared_ptr<AppEventPack> event = CreateEventPack(TEST_EVENT_NAME, TEST_EVENT_TYPE);
    int verifyRes = VerifyAppEvent(event);
    ASSERT_EQ(verifyRes, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);

    // disabling event logging
    HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "true");
    verifyRes = VerifyAppEvent(event);
    ASSERT_EQ(verifyRes, ErrorCode::ERROR_HIAPPEVENT_DISABLE);

    // enabling event logging
    HiAppEventConfig::GetInstance().SetConfigurationItem("disable", "false");
    verifyRes = VerifyAppEvent(event);
    ASSERT_EQ(verifyRes, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}