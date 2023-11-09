/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <map>

#include <gtest/gtest.h>

#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_userinfo.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";
const std::string TEST_USER_ID_NAME = "testUserName";
const std::string TEST_USER_ID_VALUE = "testUserId";
const std::string TEST_USER_PROP_NAME = "testUserName";
const std::string TEST_USER_PROP_VALUE = "testUserProperty";

class HiAppEventUserInfoTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventUserInfoTest::SetUp()
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
    (void)AppEventStore::GetInstance().InitDbStore();
    HiAppEvent::UserInfo::GetInstance().RemoveUserId(TEST_USER_ID_NAME);
    HiAppEvent::UserInfo::GetInstance().RemoveUserProperty(TEST_USER_PROP_NAME);
}

void HiAppEventUserInfoTest::TearDown()
{
    (void)AppEventStore::GetInstance().DestroyDbStore();
}
}

/**
 * @tc.name: HiAppEventUserInfoTest001
 * @tc.desc: Test to set and get user id.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest001, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest001 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserId(TEST_USER_ID_NAME, TEST_USER_ID_VALUE);
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserId;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserId(TEST_USER_ID_NAME, strUserId);
        ASSERT_EQ(ret, 0);
        ASSERT_NE(strUserId, "");
    }

    std::cout << "HiAppEventUserInfoTest001 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest002
 * @tc.desc: Test failed to get user id.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest002, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest002 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserId(TEST_USER_ID_NAME, TEST_USER_ID_VALUE);
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserId;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserId("", strUserId);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(strUserId, "");
    }

    std::cout << "HiAppEventUserInfoTest002 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest003
 * @tc.desc: Test to replace and get user id.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest003, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest003 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserId(TEST_USER_ID_NAME, TEST_USER_ID_VALUE);
        ASSERT_EQ(ret, 0);

        ret = HiAppEvent::UserInfo::GetInstance().SetUserId(TEST_USER_ID_NAME, "testUserId1");
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserId;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserId(TEST_USER_ID_NAME, strUserId);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(strUserId, "testUserId1");
    }

    std::cout << "HiAppEventUserInfoTest003 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest004
 * @tc.desc: Test to remove and get user id.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest004, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest004 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserId(TEST_USER_ID_NAME, TEST_USER_ID_VALUE);
        ASSERT_EQ(ret, 0);
        
        ret = HiAppEvent::UserInfo::GetInstance().SetUserId(TEST_USER_ID_NAME, "");
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserId;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserId(TEST_USER_ID_NAME, strUserId);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(strUserId, "");
    }

    std::cout << "HiAppEventUserInfoTest004 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest005
 * @tc.desc: Test to set and get user property.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest005, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest005 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserProperty(TEST_USER_PROP_NAME, TEST_USER_PROP_VALUE);
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserProperty;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserProperty(TEST_USER_PROP_NAME, strUserProperty);
        ASSERT_EQ(ret, 0);
        ASSERT_NE(strUserProperty, "");
    }

    std::cout << "HiAppEventUserInfoTest005 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest006
 * @tc.desc: Test failed to get user property.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest006, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest006 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserProperty(TEST_USER_PROP_NAME, TEST_USER_PROP_VALUE);
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserProperty;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserProperty("", strUserProperty);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(strUserProperty, "");
    }

    std::cout << "HiAppEventUserInfoTest006 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest007
 * @tc.desc: Test to set and get user property.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest007, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest007 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserProperty(TEST_USER_PROP_NAME, TEST_USER_PROP_VALUE);
        ASSERT_EQ(ret, 0);

        ret = HiAppEvent::UserInfo::GetInstance().SetUserProperty(TEST_USER_PROP_NAME, "testUserProperty1");
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserProperty;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserProperty(TEST_USER_PROP_NAME, strUserProperty);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(strUserProperty, "testUserProperty1");
    }

    std::cout << "HiAppEventUserInfoTest007 end" << std::endl;
}

/**
 * @tc.name: HiAppEventUserInfoTest008
 * @tc.desc: Test to remove and get user property.
 * @tc.type: FUNC
 * @tc.require: issueI8EOLQ
 */
HWTEST_F(HiAppEventUserInfoTest, HiAppEventUserInfoTest008, TestSize.Level3)
{
    std::cout << "HiAppEventUserInfoTest008 start" << std::endl;

    {
        int ret = HiAppEvent::UserInfo::GetInstance().SetUserProperty(TEST_USER_PROP_NAME, TEST_USER_PROP_VALUE);
        ASSERT_EQ(ret, 0);

        ret = HiAppEvent::UserInfo::GetInstance().SetUserProperty(TEST_USER_PROP_NAME, "");
        ASSERT_EQ(ret, 0);
    }
    {
        std::string strUserProperty;
        int ret = HiAppEvent::UserInfo::GetInstance().GetUserProperty(TEST_USER_PROP_NAME, strUserProperty);
        ASSERT_EQ(ret, 0);
        ASSERT_EQ(strUserProperty, "");
    }

    std::cout << "HiAppEventUserInfoTest008 end" << std::endl;
}