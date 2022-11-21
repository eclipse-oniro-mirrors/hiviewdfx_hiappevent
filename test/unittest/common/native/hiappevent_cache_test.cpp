/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "hiappevent_cache_test.h"

#include "app_event_cache_common.h"
#include "app_event_cache.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_write.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::AppEventCacheCommon;
namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";
const std::string TEST_DB_DIR = "/data/test/hiappevent/databases/";
const std::string TEST_BLOCK = "testBlock";
const std::string TEST_PACKAGE = "{\"domain_\":\"hiappevent\", \"name_\":\"testEvent\"}";
}

void HiAppEventCacheTest::TearDown()
{
    AppEventCache::GetInstance()->Close();
}

/**
 * @tc.name: HiAppEventDBTest001
 * @tc.desc: check the successful result of DB operation.
 * @tc.type: FUNC
 * @tc.require: issueI5K0X6
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. create block table.
     * @tc.steps: step3. add record to the block table.
     */
    int result = AppEventCache::GetInstance()->Open(TEST_DIR);
    ASSERT_EQ(result, DB_SUCC);

    result = AppEventCache::GetInstance()->CreateBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_SUCC);

    auto block = AppEventCache::GetInstance()->GetBlock(TEST_BLOCK);
    ASSERT_NE(block, nullptr);

    std::map<std::string, std::pair<int, int64_t>> blocksStat;
    result = AppEventCache::GetInstance()->GetBlocksStat(blocksStat);
    ASSERT_EQ(result, DB_SUCC);
    ASSERT_EQ(blocksStat.size(), 1);

    result = AppEventCache::GetInstance()->CreateBlock("testBlock2");
    ASSERT_EQ(result, DB_SUCC);
    std::map<std::string, std::pair<int, int64_t>> blocksStat2;
    result = AppEventCache::GetInstance()->GetBlocksStat(blocksStat2);
    ASSERT_EQ(result, DB_SUCC);
    ASSERT_EQ(blocksStat2.size(), 2);

    result = AppEventCache::GetInstance()->DestroyBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_SUCC);
    result = AppEventCache::GetInstance()->DestroyBlock("testBlock2");
    ASSERT_EQ(result, DB_SUCC);

    result = AppEventCache::GetInstance()->Close();
    ASSERT_EQ(result, DB_SUCC);
}

/**
 * @tc.name: HiAppEventDBTest002
 * @tc.desc: check the failed result of DB operation.
 * @tc.type: FUNC
 * @tc.require: issueI5K0X6
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. create block table.
     * @tc.steps: step3. destroy the block table.
     * @tc.steps: step4. close the db.
     */
    int result = AppEventCache::GetInstance()->Open("");
    ASSERT_EQ(result, DB_FAILED);

    result = AppEventCache::GetInstance()->CreateBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_FAILED);

    auto block = AppEventCache::GetInstance()->GetBlock(TEST_BLOCK);
    ASSERT_EQ(block, nullptr);

    std::map<std::string, std::pair<int, int64_t>> blocksStat;
    result = AppEventCache::GetInstance()->GetBlocksStat(blocksStat);
    ASSERT_EQ(result, DB_FAILED);
    ASSERT_TRUE(blocksStat.empty());

    result = AppEventCache::GetInstance()->DestroyBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_FAILED);

    result = AppEventCache::GetInstance()->Close();
    ASSERT_EQ(result, DB_SUCC);
}

/**
 * @tc.name: HiAppEventBlockTest001
 * @tc.desc: check the successful result of block operation.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOD
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventBlockTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. create block table.
     * @tc.steps: step3. add record to the block table.
     * @tc.steps: step4. delete record from the block table.
     * @tc.steps: step5. close the db.
     */
    int result = AppEventCache::GetInstance()->Open(TEST_DIR);
    ASSERT_EQ(result, DB_SUCC);
    result = AppEventCache::GetInstance()->CreateBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_SUCC);
    auto block = AppEventCache::GetInstance()->GetBlock(TEST_BLOCK);
    ASSERT_NE(block, nullptr);

    // put 5 records to the block
    const int ADD_NUM = 5;
    for (int i = 0; i < ADD_NUM; ++i) {
        result = block->Add(TEST_PACKAGE);
        ASSERT_EQ(result, DB_SUCC);
    }

    std::map<std::string, std::pair<int, int64_t>> blocksStat;
    result = AppEventCache::GetInstance()->GetBlocksStat(blocksStat);
    ASSERT_EQ(result, DB_SUCC);
    ASSERT_EQ(blocksStat.size(), 1);
    ASSERT_EQ(blocksStat[TEST_BLOCK].first, ADD_NUM);
    ASSERT_EQ(blocksStat[TEST_BLOCK].second, TEST_PACKAGE.size() * ADD_NUM);

    // take one record from the block
    std::vector<std::string> packages;
    result = block->Take(TEST_PACKAGE.size(), packages);
    ASSERT_EQ(result, TEST_PACKAGE.size());
    ASSERT_EQ(packages.size(), 1);

    // remove one record from the block
    result = block->Remove(1);
    ASSERT_EQ(result, DB_SUCC);
    std::pair<int, int64_t> statPair;
    result = block->GetStat(statPair);
    ASSERT_EQ(result, DB_SUCC);
    ASSERT_EQ(statPair.first, ADD_NUM - 2);
    ASSERT_EQ(statPair.second, TEST_PACKAGE.size() * (ADD_NUM - 2));

    result = AppEventCache::GetInstance()->DestroyBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_SUCC);
    result = AppEventCache::GetInstance()->Close();
    ASSERT_EQ(result, DB_SUCC);
}

/**
 * @tc.name: HiAppEventCleanTest001
 * @tc.desc: check the cleaning function of DB.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventCleanTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. create block table.
     * @tc.steps: step3. add record to the block table.
     * @tc.steps: step4. config the max storage size.
     * @tc.steps: step5. trigger cleanup.
     * @tc.steps: step6. close the db.
     */
    int result = AppEventCache::GetInstance()->Open(TEST_DIR);
    ASSERT_EQ(result, DB_SUCC);

    std::string blockNames[] = { "testBlock1", "testBlock2", "testBlock3" };
    const int ADD_NUM = 10;
    for (auto& blockName : blockNames) {
        result = AppEventCache::GetInstance()->CreateBlock(blockName);
        ASSERT_EQ(result, DB_SUCC);
        auto block = AppEventCache::GetInstance()->GetBlock(blockName);
        ASSERT_NE(block, nullptr);
        for (int i = 0; i < ADD_NUM; ++i) {
            result = block->Add(TEST_PACKAGE);
            ASSERT_EQ(result, DB_SUCC);
        }
    }

    // trigger cleanup
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
    HiAppEventConfig::GetInstance().SetConfigurationItem("max_storage", "1024b");
    WriteEvent(std::make_shared<AppEventPack>("name", 1));

    std::map<std::string, std::pair<int, int64_t>> blocksStat;
    result = AppEventCache::GetInstance()->GetBlocksStat(blocksStat);
    ASSERT_EQ(result, DB_SUCC);
    ASSERT_EQ(blocksStat.size(), 3);
    for (auto& blockStat : blocksStat) {
        auto statPair = blockStat.second;
        ASSERT_TRUE(statPair.first < ADD_NUM);
        ASSERT_TRUE(statPair.second < (TEST_PACKAGE.size() * ADD_NUM));
    }

    result = AppEventCache::GetInstance()->Close();
    ASSERT_EQ(result, DB_SUCC);
}

/**
 * @tc.name: HiAppEventCleanTest002
 * @tc.desc: check the result of clear data.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventCleanTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create log file.
     * @tc.steps: step2. create db file.
     * @tc.steps: step3. clear the data.
     */
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
    HiAppEventConfig::GetInstance().SetConfigurationItem("max_storage", "10M");
    WriteEvent(std::make_shared<AppEventPack>("name", 1));

    int result = AppEventCache::GetInstance()->Open(TEST_DIR);
    ASSERT_EQ(result, DB_SUCC);
    result = AppEventCache::GetInstance()->CreateBlock(TEST_BLOCK);
    ASSERT_EQ(result, DB_SUCC);
    auto block = AppEventCache::GetInstance()->GetBlock(TEST_BLOCK);
    ASSERT_NE(block, nullptr);
    result = block->Add(TEST_PACKAGE);
    ASSERT_EQ(result, DB_SUCC);

    ASSERT_TRUE(FileUtil::IsFileExists(TEST_DIR));
    ASSERT_TRUE(FileUtil::IsFileExists(TEST_DB_DIR));

    HiAppEventClean::ClearData(TEST_DIR);
    ASSERT_FALSE(FileUtil::IsFileExists(TEST_DB_DIR));
    ASSERT_FALSE(FileUtil::IsFileExists(TEST_DIR));
}

