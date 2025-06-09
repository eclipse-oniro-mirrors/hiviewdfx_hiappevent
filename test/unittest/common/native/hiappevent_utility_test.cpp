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
#include "cJSON.h"

#include "event_json_util.h"
#include "file_util.h"
#include "hiappevent_config.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";

class HiAppEventUtilityTest : public testing::Test {
public:
    void SetUp();
    void TearDown() {}
};

void HiAppEventUtilityTest::SetUp()
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
}
}

/**
 * @tc.name: HiAppEventJsonUtil001
 * @tc.desc: test the event json util ParseUInt32.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventUtilityTest, HiAppEventJsonUtil001, TestSize.Level1)
{
    std::cout << "HiAppEventJsonUtil001 start" << std::endl;
    cJSON *root = cJSON_CreateObject();
    ASSERT_TRUE(root);
    uint32_t result = EventJsonUtil::ParseUInt32(root, "testKey");
    EXPECT_EQ(result, 0u);

    cJSON_AddItemToObject(root, "testKey", cJSON_CreateNull());
    result = EventJsonUtil::ParseUInt32(root, "testKey");
    EXPECT_EQ(result, 0u);

    cJSON_DeleteItemFromObject(root, "testKey");
    cJSON_AddNumberToObject(root, "testKey", 123.45);
    result = EventJsonUtil::ParseUInt32(root, "testKey");
    EXPECT_EQ(result, 0u);

    cJSON_DeleteItemFromObject(root, "testKey");
    cJSON_AddNumberToObject(root, "testKey", 123u);
    result = EventJsonUtil::ParseUInt32(root, "testKey");
    EXPECT_EQ(result, 123u);
    std::cout << "HiAppEventJsonUtil001 end" << std::endl;
    cJSON_Delete(root);
}

/**
 * @tc.name: HiAppEventJsonUtil002
 * @tc.desc: test the event json util ParseInt.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventUtilityTest, HiAppEventJsonUtil002, TestSize.Level1)
{
    std::cout << "HiAppEventJsonUtil002 start" << std::endl;
    cJSON *root = cJSON_CreateObject();
    ASSERT_TRUE(root);
    int result = EventJsonUtil::ParseInt(root, "testKey");
    EXPECT_EQ(result, 0);

    cJSON_AddItemToObject(root, "testKey", cJSON_CreateNull());
    result = EventJsonUtil::ParseInt(root, "testKey");
    EXPECT_EQ(result, 0);

    cJSON_DeleteItemFromObject(root, "testKey");
    cJSON_AddNumberToObject(root, "testKey", 123.45);
    result = EventJsonUtil::ParseInt(root, "testKey");
    EXPECT_EQ(result, 0);

    cJSON_DeleteItemFromObject(root, "testKey");
    cJSON_AddNumberToObject(root, "testKey", 123);
    result = EventJsonUtil::ParseInt(root, "testKey");
    EXPECT_EQ(result, 123);
    std::cout << "HiAppEventJsonUtil002 end" << std::endl;
    cJSON_Delete(root);
}

/**
 * @tc.name: HiAppEventJsonUtil003
 * @tc.desc: test the event json util ParseString.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventUtilityTest, HiAppEventJsonUtil003, TestSize.Level1)
{
    std::cout << "HiAppEventJsonUtil003 start" << std::endl;
    cJSON *root = cJSON_CreateObject();
    ASSERT_TRUE(root);
    std::string result = EventJsonUtil::ParseString(root, "testKey");
    EXPECT_EQ(result, "");

    cJSON_AddItemToObject(root, "testKey", cJSON_CreateNull());
    result = EventJsonUtil::ParseString(root, "testKey");
    EXPECT_EQ(result, "");

    cJSON_DeleteItemFromObject(root, "testKey");
    cJSON_AddNumberToObject(root, "testKey", 123.45);
    result = EventJsonUtil::ParseString(root, "testKey");
    EXPECT_EQ(result, "");

    cJSON_DeleteItemFromObject(root, "testKey");
    cJSON_AddStringToObject(root, "testKey", "testStr");
    result = EventJsonUtil::ParseString(root, "testKey");
    EXPECT_EQ(result, "testStr");
    std::cout << "HiAppEventJsonUtil003 end" << std::endl;
    cJSON_Delete(root);
}

/**
 * @tc.name: HiAppEventJsonUtil004
 * @tc.desc: test the event json util ParseStrings.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventUtilityTest, HiAppEventJsonUtil004, TestSize.Level1)
{
    std::cout << "HiAppEventJsonUtil004 start" << std::endl;
    cJSON *root = cJSON_CreateObject();
    ASSERT_TRUE(root);
    std::unordered_set<std::string> strs;

    EventJsonUtil::ParseStrings(root, "testKey", strs);
    EXPECT_TRUE(strs.empty());

    cJSON *array = cJSON_CreateArray();
    ASSERT_TRUE(array);
    cJSON_AddItemToArray(array, cJSON_CreateString("string1"));
    cJSON_AddItemToArray(array, cJSON_CreateString("string2"));
    cJSON_AddItemToObject(root, "testKey", array);
    EventJsonUtil::ParseStrings(root, "testKey", strs);
    EXPECT_TRUE(strs.find("string1") != strs.end());
    EXPECT_TRUE(strs.find("string2") != strs.end());
    std::cout << "HiAppEventJsonUtil004 end" << std::endl;
    cJSON_Delete(root);
}

/**
 * @tc.name: HiAppEventFileUtil001
 * @tc.desc: test the FileUtil.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventUtilityTest, HiAppEventFileUtil001, TestSize.Level1)
{
    std::cout << "HiAppEventFileUtil001 start" << std::endl;
    bool isDir = FileUtil::IsDirectory("");
    EXPECT_FALSE(isDir);
    std::string testDir = TEST_DIR + "test";
    bool makeDirRes = FileUtil::ForceCreateDirectory(testDir);
    EXPECT_TRUE(makeDirRes);
    isDir = FileUtil::IsDirectory(testDir);
    EXPECT_TRUE(isDir);

    bool setRes = FileUtil::SetDirXattr(testDir, "user.appevent", "testValue");
    EXPECT_TRUE(setRes);
    std::string testValue;
    bool getRes = FileUtil::GetDirXattr(testDir, "user.appevent", testValue);
    EXPECT_TRUE(getRes);
    EXPECT_EQ(testValue, "testValue");

    std::string filePath = FileUtil::GetFilePathByDir("", "test.txt");
    EXPECT_EQ(filePath, "test.txt");
    filePath = FileUtil::GetFilePathByDir(TEST_DIR, "test.txt");
    bool makeFileRes = FileUtil::CreateFile(filePath, S_IRUSR | S_IWUSR);
    EXPECT_TRUE(makeFileRes);
    bool writeFileRes = FileUtil::SaveStringToFile(filePath, "", true);
    EXPECT_TRUE(writeFileRes);
    writeFileRes = FileUtil::SaveStringToFile(filePath, filePath, true);
    EXPECT_TRUE(writeFileRes);
    std::vector<std::string> lines;
    bool loadRes = FileUtil::LoadLinesFromFile(filePath, lines);
    EXPECT_TRUE(loadRes);
    EXPECT_EQ(lines[0], filePath);

    bool removeDirRes = FileUtil::ForceRemoveDirectory("", true);
    EXPECT_FALSE(removeDirRes);
    removeDirRes = FileUtil::ForceRemoveDirectory(testDir, true);
    EXPECT_TRUE(removeDirRes);
    isDir = FileUtil::IsDirectory(testDir);
    EXPECT_FALSE(isDir);
    std::cout << "HiAppEventFileUtil001 end" << std::endl;
}