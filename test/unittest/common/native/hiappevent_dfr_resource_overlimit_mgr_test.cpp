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
#include <gmock/gmock.h>

#include <unordered_set>

#include <unistd.h>
#include <sys/stat.h>
#define private public
#include "resource_overlimit_mgr.h"

#include <application_context.h>
#include <hilog/log.h>
#include "file_util.h"
#include "hiappevent_base.h"
#undef private

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::HiviewDFX;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
std::shared_ptr<AbilityRuntime::ApplicationContext> g_applicationContext = nullptr;
constexpr int RETURN_CNT_1 = 1;
constexpr int RETURN_CNT_2 = 2;
constexpr int RETURN_CNT_8 = 8;
constexpr int RETURN_CNT_14 = 14;
constexpr const char* const TEST_PATH = "/data/local/tmp/rawheap";

void CreateTestDir()
{
    std::string path = TEST_PATH;
    if (access(path.c_str(), F_OK) != 0) {
        constexpr mode_t defaultMode = 0755;
        if (mkdir(path.c_str(), defaultMode) != 0) {
            perror("mkdir failed");
        }
    }
}

void RemoveTestDir()
{
    std::string path = TEST_PATH;
    if (access(path.c_str(), F_OK) == 0) {
        if (rmdir(path.c_str()) != 0) {
            perror("rmdir failed");
        }
    }
}
}

namespace AbilityRuntime {
std::shared_ptr<ApplicationContext> Context::GetApplicationContext()
{
    return g_applicationContext;
}
}  // namespace AbilityRuntime

class ApplicationContextMock : public ApplicationContext {
public:
    MOCK_METHOD0(GetCacheDir, std::string());
};

namespace HiviewDFX {
class GetRawheapDirTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        CreateTestDir();
    }

    void TearDown() override
    {
        g_applicationContext = nullptr;
        RemoveTestDir();
    }
};

/**
 * @tc.name  : GetRawheapDirTest_ShouldReturnEmpty_WhenContextIsNullptr
 * @tc.number: GetRawheapDirTest_001
 * @tc.desc  : context == nullptr, GetRawheapDir 应返回 ""
 */
HWTEST_F(GetRawheapDirTest, GetRawheapDirTest_001, TestSize.Level0)
{
    g_applicationContext = nullptr;

    EXPECT_TRUE(ResourceOverlimitMgr::GetInstance().GetRawheapDir().empty());
}

/**
 * @tc.name  : GetRawheapDir_ShouldReturnEmpty_WhenContextIsNullptr
 * @tc.number: GetRawheapDirTest_002
 * @tc.desc  : GetCacheDir.empty()时, GetRawheapDir 应返回 ""
 */
HWTEST_F(GetRawheapDirTest, GetRawheapDirTest_002, TestSize.Level0)
{
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_1)
        .WillRepeatedly(::testing::Return(""));
    g_applicationContext.reset(contextMock);

    EXPECT_TRUE(ResourceOverlimitMgr::GetInstance().GetRawheapDir().empty());
}

/**
 * @tc.name  : GetRawheapDir_ShouldReturnEmpty_WhenPathNotExist
 * @tc.number: GetRawheapDirTest_003
 * @tc.desc  : 测试当rawheap时, GetRawheapDir 应返回 ""
 */
HWTEST_F(GetRawheapDirTest, GetRawheapDirTest_003, TestSize.Level0)
{
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_2)
        .WillRepeatedly(::testing::Return("/data/storage/el2/base/cache"));
    g_applicationContext.reset(contextMock);

    EXPECT_TRUE(ResourceOverlimitMgr::GetInstance().GetRawheapDir().empty());
}

/**
 * @tc.name  : GetRawheapDir_ShouldReturnPath_WhenPathExist
 * @tc.number: GetRawheapDirTest_004
 * @tc.desc  : 测试当rawheap存在时, GetRawheapDir 应返回 TEST_PATH
 */
HWTEST_F(GetRawheapDirTest, GetRawheapDirTest_004, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);

    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_2)
        .WillRepeatedly(::testing::Return("/data/local/tmp"));
    g_applicationContext.reset(contextMock);

    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().GetRawheapDir(), TEST_PATH);
}

// SetRunningId
class SetRunningIdTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : SetRunningIdTest_ShouldReturnEmpty_WhenInputEmpty
 * @tc.number: SetRunningIdTest_001
 * @tc.desc  : id == "", runningId_ 应等于 ""
 */
HWTEST_F(SetRunningIdTest, SetRunningIdTest_001, TestSize.Level0)
{
    ResourceOverlimitMgr::GetInstance().SetRunningId("");
    EXPECT_TRUE(ResourceOverlimitMgr::GetInstance().runningId_.empty());
    ResourceOverlimitMgr::GetInstance().SetRunningId("testValue");
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().runningId_, "testValue");
}

// IsValid
class IsValidTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : IsValidTest_ShouldReturnFalse_WhenKeyInvalid
 * @tc.number: IsValidTest_001
 * @tc.desc  : 当key invalid, return false
 */
HWTEST_F(IsValidTest, IsValidTest_001, TestSize.Level0)
{
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("invalid", "event"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("invalid", "event_rawheap"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("invalid", "invalid too"));
}

/**
 * @tc.name  : IsValidTest_ShouldReturnFalse_WhenKeyValidButValueInvalid
 * @tc.number: IsValidTest_002
 * @tc.desc  : key valid, value invalid, then return false
 */
HWTEST_F(IsValidTest, IsValidTest_002, TestSize.Level0)
{
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "invalid"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "invalid too"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "even"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "eventt"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "eevent"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "eventrawheap"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "rawheap"));
    EXPECT_FALSE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", ""));
}

/**
 * @tc.name  : IsValidTest_ShouldReturnFalse_WhenKeyValidAndValueValid
 * @tc.number: IsValidTest_003
 * @tc.desc  : key valid, value valid, then return true
 */
HWTEST_F(IsValidTest, IsValidTest_003, TestSize.Level0)
{
    EXPECT_TRUE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "event"));
    EXPECT_TRUE(ResourceOverlimitMgr::GetInstance().IsValid("js_heap_logtype", "event_rawheap"));
}

// UpdateProperty
class UpdatePropertyTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ResourceOverlimitMgr::GetInstance().runningId_ = "";
        CreateTestDir();
    }

    void TearDown() override
    {
        ResourceOverlimitMgr::GetInstance().runningId_ = "";
        RemoveTestDir();
        g_applicationContext = nullptr;
    }
};

/**
 * @tc.name  : UpdatePropertyTest_ShouldReturnERROR_UNKNOWN_WhenRunningId_Empty
 * @tc.number: UpdatePropertyTest_001
 * @tc.desc  : runningId_ empty, return ERROR_UNKNOWN
 */
HWTEST_F(UpdatePropertyTest, UpdatePropertyTest_001, TestSize.Level0)
{
    ResourceOverlimitMgr::GetInstance().runningId_ = "";
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("user.key", "value"), ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name  : UpdatePropertyTest_ShouldReturnERROR_UNKNOWN_WhenRawheapNotExist
 * @tc.number: UpdatePropertyTest_002
 * @tc.desc  : not exist rawheap dir, return ERROR_UNKNOWN
 */
HWTEST_F(UpdatePropertyTest, UpdatePropertyTest_002, TestSize.Level0)
{
    RemoveTestDir();
    ASSERT_NE(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";

    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("user.key", "value"), ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name  : UpdatePropertyTest_ShouldReturnERROR_UNKNOWN_WhenPropertyInvalid
 * @tc.number: UpdatePropertyTest_003
 * @tc.desc  : property invalid, return ERROR_UNKNOWN
 */
HWTEST_F(UpdatePropertyTest, UpdatePropertyTest_003, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_14)
        .WillRepeatedly(::testing::Return("/data/local/tmp"));
    g_applicationContext.reset(contextMock);

    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("key", "value"), ErrorCode::ERROR_UNKNOWN);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("key.key", "value"), ErrorCode::ERROR_UNKNOWN);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("1", "value"), ErrorCode::ERROR_UNKNOWN);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("js_heap_logtype", "1"), ErrorCode::ERROR_UNKNOWN);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("", ""), ErrorCode::ERROR_UNKNOWN);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("1", ""), ErrorCode::ERROR_UNKNOWN);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("", "1"), ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name  : UpdatePropertyTest_ShouldReturn0_WhenPropertyValid
 * @tc.number: UpdatePropertyTest_004
 * @tc.desc  : property valid, return 0
 */
HWTEST_F(UpdatePropertyTest, UpdatePropertyTest_004, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_8)
        .WillRepeatedly(::testing::Return("/data/local/tmp"));
    g_applicationContext.reset(contextMock);

    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("user.js_heap_logtype", "value"), 0);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("user.event_config.js_heap_logtype", "event_rawheap"),
              0);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("user.event_config.js_heap_logtype", "event"), 0);
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().UpdateProperty("user.event_config.js_heap_logtype", "event_invalid"),
              0);
}

// SetEventConfig
class SetEventConfigTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        ResourceOverlimitMgr::GetInstance().runningId_ = "";
        CreateTestDir();
    }

    void TearDown() override
    {
        ResourceOverlimitMgr::GetInstance().runningId_ = "";
        RemoveTestDir();
        g_applicationContext = nullptr;
    }
};

/**
 * @tc.name  : SetEventConfigTest_ShouldReturnERROR_UNKNOWN_WhenConfigMapSizeis1AndInValid
 * @tc.number: SetEventConfigTest_001
 * @tc.desc  : configMap.size() == 1 invalid, return ERROR_UNKNOWN
 */
HWTEST_F(SetEventConfigTest, SetEventConfigTest_001, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    g_applicationContext.reset(contextMock);

    std::map<std::string, std::string> configMap = {{"invalid_key", "invalid_value"}};
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().SetEventConfig(configMap), ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name  : SetEventConfigTest_ShouldReturn0_WhenConfigMapSizeis1AndValid
 * @tc.number: SetEventConfigTest_002
 * @tc.desc  : configMap.size() == 1, value valid, return 0
 */
HWTEST_F(SetEventConfigTest, SetEventConfigTest_002, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_2)
        .WillRepeatedly(::testing::Return("/data/local/tmp"));
    g_applicationContext.reset(contextMock);

    std::map<std::string, std::string> configMap = {{"js_heap_logtype", "event"}};
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().SetEventConfig(configMap), 0);
}

/**
 * @tc.name  : SetEventConfigTest_ShouldReturnERROR_UNKNOWN_WhenConfigMapSizeis0
 * @tc.number: SetEventConfigTest_003
 * @tc.desc  : configMap.size() == 0, return ERROR_UNKNOWN
 */
HWTEST_F(SetEventConfigTest, SetEventConfigTest_003, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    g_applicationContext.reset(contextMock);

    std::map<std::string, std::string> configMap;
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().SetEventConfig(configMap), ErrorCode::ERROR_UNKNOWN);
}

/**
 * @tc.name  : SetEventConfigTest_ShouldReturn0_WhenConfigMapSizeis2And1valid1invalid
 * @tc.number: SetEventConfigTest_004
 * @tc.desc  : configMap.size() == 2, configMap[0] valid, configMap[1] invalid, return 0
 */
HWTEST_F(SetEventConfigTest, SetEventConfigTest_004, TestSize.Level0)
{
    ASSERT_EQ(access(TEST_PATH, F_OK), 0);
    ResourceOverlimitMgr::GetInstance().runningId_ = "5555";
    ApplicationContextMock *contextMock = new ApplicationContextMock();
    ASSERT_NE(contextMock, nullptr);
    EXPECT_CALL(*contextMock, GetCacheDir())
        .Times(RETURN_CNT_2)
        .WillRepeatedly(::testing::Return("/data/local/tmp"));
    g_applicationContext.reset(contextMock);

    std::map<std::string, std::string> configMap {
        {"js_heap_logtype", "event"},
        {"invalid", "invalid"},
    };
    EXPECT_EQ(ResourceOverlimitMgr::GetInstance().SetEventConfig(configMap), 0);
}
}  // HiviewDFX
}  // namespace OHOS
