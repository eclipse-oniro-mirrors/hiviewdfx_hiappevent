/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "app_event_db_cleaner.h"
#include "app_event_log_cleaner.h"
#include "app_event_stat.h"
#include "app_event_store.h"
#include "app_event_store_callback.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_write.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "time_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::AppEventCacheCommon;
namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";
const std::string TEST_INVALID_DIR1 = "";
const std::string TEST_INVALID_DIR2 = " ";
const std::string TEST_DB_PATH = "/data/test/hiappevent/databases/appevent.db";
const std::string TEST_OBSERVER_NAME = "test_observer";
const std::string TEST_EVENT_DOMAIN = "test_domain";
const std::string TEST_EVENT_NAME = "test_name";
constexpr int TEST_EVENT_TYPE = 1;
const std::string TEST_PACKAGE = "{\"domain_\":\"hiappevent\", \"name_\":\"testEvent\"}";
const std::string TEST_RUNNING_ID = "running_test";

std::shared_ptr<AppEventPack> CreateAppEventPack()
{
    return std::make_shared<AppEventPack>(TEST_EVENT_DOMAIN, TEST_EVENT_NAME, TEST_EVENT_TYPE);
}
}

void HiAppEventCacheTest::SetUp()
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
}

/**
 * @tc.name: HiAppEventDBTest001
 * @tc.desc: check the query result of DB operation.
 * @tc.type: FUNC
 * @tc.require: issueI5K0X6
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. try to open the invalid db.
     * @tc.steps: step2. open the valid db.
     * @tc.steps: step3. insert record to the tables.
     * @tc.steps: step4. query records from tables.
     */
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_INVALID_DIR1);
    int result = AppEventStore::GetInstance().InitDbStore();
    ASSERT_EQ(result, DB_FAILED);
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_INVALID_DIR2);
    result = AppEventStore::GetInstance().InitDbStore();
    ASSERT_EQ(result, DB_FAILED);

    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
    result = AppEventStore::GetInstance().InitDbStore();
    ASSERT_EQ(result, DB_SUCC);

    int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(CreateAppEventPack());
    ASSERT_GT(eventSeq, 0);
    int64_t observerSeq = AppEventStore::GetInstance().InsertObserver(AppEventCacheCommon::Observer(TEST_OBSERVER_NAME,
        0, ""));
    ASSERT_GT(observerSeq, 0);
    result = AppEventStore::GetInstance().InsertEventMapping({EventObserverInfo(eventSeq, observerSeq)});
    ASSERT_EQ(result, 0);

    std::vector<std::shared_ptr<AppEventPack>> events;
    result = AppEventStore::GetInstance().QueryEvents(events, observerSeq);
    ASSERT_EQ(result, 0);
    ASSERT_GT(events.size(), 0);
    ASSERT_EQ(events[0]->GetDomain(), TEST_EVENT_DOMAIN);
    ASSERT_EQ(events[0]->GetName(), TEST_EVENT_NAME);
    ASSERT_EQ(events[0]->GetType(), TEST_EVENT_TYPE);

    std::vector<int64_t> observerSeqs;
    result = AppEventStore::GetInstance().QueryObserverSeqs(TEST_OBSERVER_NAME, observerSeqs);
    ASSERT_EQ(result, 0);
    ASSERT_GT(observerSeqs.size(), 0);
    ASSERT_EQ(observerSeqs[0], observerSeq);

    std::string filters = "{\"OS\", {\"APP_CRASH\"}}";
    result = AppEventStore::GetInstance().UpdateObserver(observerSeq, filters);
    ASSERT_EQ(result, DB_SUCC);

    result = AppEventStore::GetInstance().DestroyDbStore();
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: HiAppEventDBTest002
 * @tc.desc: check the take result of DB operation.
 * @tc.type: FUNC
 * @tc.require: issueI5K0X6
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. insert record to the tables.
     * @tc.steps: step3. take records from tables.
     */
    int result = AppEventStore::GetInstance().InitDbStore();;
    ASSERT_EQ(result, 0);

    auto eventSeq = AppEventStore::GetInstance().InsertEvent(CreateAppEventPack());
    ASSERT_GT(eventSeq, 0);
    auto observerSeq = AppEventStore::GetInstance().InsertObserver(AppEventCacheCommon::Observer(TEST_OBSERVER_NAME,
        0, ""));
    ASSERT_GT(observerSeq, 0);
    result = AppEventStore::GetInstance().InsertEventMapping({EventObserverInfo(eventSeq, observerSeq)});
    ASSERT_EQ(result, 0);

    std::vector<std::shared_ptr<AppEventPack>> events;
    result = AppEventStore::GetInstance().TakeEvents(events, observerSeq);
    ASSERT_EQ(result, 0);
    ASSERT_GT(events.size(), 0);
    ASSERT_EQ(events[0]->GetDomain(), TEST_EVENT_DOMAIN);
    ASSERT_EQ(events[0]->GetName(), TEST_EVENT_NAME);
    ASSERT_EQ(events[0]->GetType(), TEST_EVENT_TYPE);

    events.clear();
    result = AppEventStore::GetInstance().QueryEvents(events, observerSeq);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(events.size(), 0);

    result = AppEventStore::GetInstance().DestroyDbStore();;
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: HiAppEventDBTest003
 * @tc.desc: check the delete result of DB operation.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOD
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest003, TestSize.Level0)
{
   /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. insert record to the tables.
     * @tc.steps: step3. delete records from tables.
     */
    int result = AppEventStore::GetInstance().InitDbStore();;
    ASSERT_EQ(result, 0);

    auto eventSeq = AppEventStore::GetInstance().InsertEvent(CreateAppEventPack());
    ASSERT_GT(eventSeq, 0);
    auto observerSeq = AppEventStore::GetInstance().InsertObserver(AppEventCacheCommon::Observer(TEST_OBSERVER_NAME,
        0, ""));
    ASSERT_GT(observerSeq, 0);
    result = AppEventStore::GetInstance().InsertEventMapping({EventObserverInfo(eventSeq, observerSeq)});
    ASSERT_EQ(result, 0);

    result = AppEventStore::GetInstance().DeleteObserver(observerSeq);
    ASSERT_EQ(result, 0);
    std::vector<int64_t> observerSeqs;
    result = AppEventStore::GetInstance().QueryObserverSeqs(TEST_OBSERVER_NAME, observerSeqs);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(observerSeqs.size(), 0);

    result = AppEventStore::GetInstance().DeleteEventMapping(observerSeq, {eventSeq});
    ASSERT_EQ(result, 0);
    std::vector<std::shared_ptr<AppEventPack>> events;
    result = AppEventStore::GetInstance().QueryEvents(events, observerSeq);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(events.size(), 0);
}

/**
 * @tc.name: HiAppEventDBTest004
 * @tc.desc: revisit the DB after destroying it.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. create block table.
     * @tc.steps: step3. add record to the block table.
     * @tc.steps: step4. config the max storage size.
     * @tc.steps: step5. trigger cleanup.
     * @tc.steps: step6. close the db.
     */
    int result = AppEventStore::GetInstance().DestroyDbStore();;
    ASSERT_EQ(result, 0);

    int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(CreateAppEventPack());
    ASSERT_GT(eventSeq, 0);
    int64_t observerSeq = AppEventStore::GetInstance().InsertObserver(AppEventCacheCommon::Observer(TEST_OBSERVER_NAME,
        0, ""));
    ASSERT_GT(observerSeq, 0);
    result = AppEventStore::GetInstance().InsertEventMapping({EventObserverInfo(eventSeq, observerSeq)});
    ASSERT_EQ(result, 0);

    std::vector<std::shared_ptr<AppEventPack>> events;
    result = AppEventStore::GetInstance().QueryEvents(events, observerSeq);
    ASSERT_EQ(result, 0);
    std::vector<int64_t> observerSeqs;
    result = AppEventStore::GetInstance().QueryObserverSeqs(TEST_OBSERVER_NAME, observerSeqs);
    ASSERT_EQ(result, 0);
    result = AppEventStore::GetInstance().TakeEvents(events, observerSeq);
    ASSERT_EQ(result, 0);

    result = AppEventStore::GetInstance().DeleteObserver(observerSeq);
    ASSERT_EQ(result, 0);
    result = AppEventStore::GetInstance().DeleteEventMapping(observerSeq, {eventSeq});
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: HiAppEventDBTest005
 * @tc.desc: check the result of clear data.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create log file.
     * @tc.steps: step2. create db file.
     * @tc.steps: step3. clear the data.
     */
    WriteEvent(std::make_shared<AppEventPack>("name", 1));
    std::vector<std::string> files;
    FileUtil::GetDirFiles(TEST_DIR, files);
    ASSERT_FALSE(files.empty());
    ASSERT_TRUE(FileUtil::IsFileExists(TEST_DIR));
    ASSERT_TRUE(FileUtil::IsFileExists(TEST_DB_PATH));

    HiAppEventClean::ClearData(TEST_DIR);
    ASSERT_TRUE(FileUtil::IsFileExists(TEST_DB_PATH));
    ASSERT_TRUE(FileUtil::IsFileExists(TEST_DIR));
    for (const auto& file : files) {
        ASSERT_FALSE(FileUtil::IsFileExists(file));
    }
}

/**
 * @tc.name: HiAppEventDBTest006
 * @tc.desc: check the query result of DB operation.
 * @tc.type: FUNC
 * @tc.require: issueI5K0X6
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDBTest006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. open the db.
     * @tc.steps: step2. insert record to the tables, insert custom param.
     * @tc.steps: step3. query records from tables.
     * @tc.steps: step3. delete custom param, query records from tables.
     */
    int result = AppEventStore::GetInstance().InitDbStore();;
    ASSERT_EQ(result, 0);

    auto event = CreateAppEventPack();
    event->SetRunningId(TEST_RUNNING_ID);
    int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(event);
    ASSERT_GT(eventSeq, 0);
    int64_t observerSeq = AppEventStore::GetInstance().InsertObserver(AppEventCacheCommon::Observer(TEST_OBSERVER_NAME,
        0, ""));
    ASSERT_GT(observerSeq, 0);
    result = AppEventStore::GetInstance().InsertEventMapping({EventObserverInfo(eventSeq, observerSeq)});
    ASSERT_EQ(result, 0);
    auto eventParams = CreateAppEventPack();
    eventParams->SetRunningId(TEST_RUNNING_ID);
    eventParams->AddParam("custom_data", "value_old_str");
    result = AppEventStore::GetInstance().InsertCustomEventParams(eventParams);
    ASSERT_EQ(result, 0);
    eventParams->AddParam("custom_data", "value_str");
    result = AppEventStore::GetInstance().InsertCustomEventParams(eventParams);
    ASSERT_EQ(result, 0);
    eventParams->AddParam("custom_data2", "value2_str");
    result = AppEventStore::GetInstance().InsertCustomEventParams(eventParams);
    ASSERT_EQ(result, 0);

    std::vector<std::shared_ptr<AppEventPack>> events;
    result = AppEventStore::GetInstance().QueryEvents(events, observerSeq);
    ASSERT_EQ(result, 0);
    ASSERT_GT(events.size(), 0);
    ASSERT_EQ(events[0]->GetDomain(), TEST_EVENT_DOMAIN);
    ASSERT_EQ(events[0]->GetName(), TEST_EVENT_NAME);
    ASSERT_EQ(events[0]->GetType(), TEST_EVENT_TYPE);
    ASSERT_EQ(events[0]->GetRunningId(), TEST_RUNNING_ID);
    ASSERT_EQ(events[0]->GetParamStr(), "{\"custom_data2\":\"value2_str\",\"custom_data\":\"value_str\"}\n");

    // delete custom params
    AppEventStore::GetInstance().DeleteCustomEventParams();
    std::vector<std::shared_ptr<AppEventPack>> events1;
    result = AppEventStore::GetInstance().QueryEvents(events1, observerSeq, 1);
    ASSERT_EQ(result, 0);
    ASSERT_EQ(events1.size(), 1);
    ASSERT_EQ(events1[0]->GetDomain(), TEST_EVENT_DOMAIN);
    ASSERT_EQ(events1[0]->GetName(), TEST_EVENT_NAME);
    ASSERT_EQ(events1[0]->GetType(), TEST_EVENT_TYPE);
    ASSERT_EQ(events1[0]->GetRunningId(), TEST_RUNNING_ID);
    ASSERT_EQ(events1[0]->GetParamStr(), "{}\n");

    result = AppEventStore::GetInstance().DestroyDbStore();;
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: HiAppEventCleanTest001
 * @tc.desc: test the DB cleaner operation.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventCleanTest001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. init the db and insert event to the table.
     * @tc.steps: step2. clear event db.
     */
    int result = AppEventStore::GetInstance().InitDbStore();
    EXPECT_EQ(result, 0);
    int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(CreateAppEventPack());
    EXPECT_GT(eventSeq, 0);

    AppEventDbCleaner dbCleaner(TEST_DIR);
    uint64_t curSize = dbCleaner.GetFilesSize();
    EXPECT_GT(curSize, 0);
    uint64_t clearResult = dbCleaner.ClearSpace(curSize, curSize);
    EXPECT_EQ(clearResult, curSize);
    uint64_t clearHistoryResult = dbCleaner.ClearSpace(curSize, 0);
    EXPECT_EQ(clearHistoryResult, 0);

    result = AppEventStore::GetInstance().DestroyDbStore();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: HiAppEventCleanTest002
 * @tc.desc: test the log cleaner operation.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventCleanTest002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create a example log.
     * @tc.steps: step2. clear log space.
     */
    std::string filePath = TEST_DIR + "test.txt";
    bool makeFileResult = FileUtil::CreateFile(filePath, S_IRUSR | S_IWUSR);
    EXPECT_TRUE(makeFileResult);
    bool writeFileResult = FileUtil::SaveStringToFile(filePath, filePath, false);
    EXPECT_TRUE(writeFileResult);

    AppEventLogCleaner logCleaner(TEST_DIR);
    uint64_t curSize = logCleaner.GetFilesSize();
    EXPECT_GT(curSize, 0);
    uint64_t clearResult = logCleaner.ClearSpace(curSize, curSize);
    EXPECT_EQ(clearResult, curSize);
    uint64_t clearHistoryResult = logCleaner.ClearSpace(curSize, 0);
    EXPECT_EQ(clearHistoryResult, 0);

    bool removeResult = FileUtil::RemoveFile(filePath);
    EXPECT_TRUE(removeResult);
}

/**
 * @tc.name: HiAppEventCleanTest003
 * @tc.desc: test the log cleaner operation.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventCleanTest003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. create a example log.
     * @tc.steps: step2. clear log space.
     */
    OHOS::HiviewDFX::HiAppEventClean::CheckStorageSpace();
    OHOS::HiviewDFX::HiAppEventClean::CheckStorageSpace();
    EXPECT_FALSE(OHOS::HiviewDFX::HiAppEventClean::IsStorageSpaceFull("", 0));
}

/**
 * @tc.name: HiAppEventStat001
 * @tc.desc: test the WriteApiEndEventAsync func of app event stat.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventStat001, TestSize.Level1)
{
    std::string apiName = "testApi";
    uint64_t beginTime = TimeUtil::GetMilliSecondsTimestamp(CLOCK_REALTIME);
    AppEventStat::WriteApiEndEventAsync(apiName, beginTime, AppEventStat::SUCCESS, AppEventStat::SUCCESS);
    AppEventStat::WriteApiEndEventAsync(apiName, -beginTime, AppEventStat::SUCCESS, AppEventStat::SUCCESS);
    EXPECT_GT(beginTime, 0);
}

/**
 * @tc.name: SetConfigurationItem001
 * @tc.desc: test the SetConfigurationItem func of app event stat.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, SetConfigurationItem001, TestSize.Level1)
{
    HiAppEventConfig::GetInstance().GetRunningId();
    HiAppEventConfig::GetInstance().RefreshFreeSize();

    bool ret = HiAppEventConfig::GetInstance().SetConfigurationItem("", "");
    EXPECT_FALSE(ret);
    std::string name = "test";
    ret = HiAppEventConfig::GetInstance().SetConfigurationItem(name, "");
    EXPECT_FALSE(ret);
    name = "disable";
    ret = HiAppEventConfig::GetInstance().SetConfigurationItem(name, "false");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: HiAppEventDbOnUpgrade001
 * @tc.desc: test the OnUpgrade func of class AppEventStoreCallback.
 * @tc.type: FUNC
 * @tc.require: issueI5NTOS
 */
HWTEST_F(HiAppEventCacheTest, HiAppEventDbOnUpgrade001, TestSize.Level1)
{
    int ret = OHOS::NativeRdb::E_OK;
    const int oldVersion = 1;
    const int dbVersion = 3;
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
    AppEventStore::GetInstance().InitDbStore();
    OHOS::NativeRdb::RdbStoreConfig config(TEST_DB_PATH);
    config.SetSecurityLevel(OHOS::NativeRdb::SecurityLevel::S1);
    AppEventStoreCallback callback;
    auto store = OHOS::NativeRdb::RdbHelper::GetRdbStore(config, dbVersion, callback, ret);
    // Only test upgrade DB from version 1 to 2, or from 2 to 3 in unit test.
    EXPECT_NE(callback.OnUpgrade(*store, oldVersion, oldVersion + 1), OHOS::NativeRdb::E_OK);
    EXPECT_NE(callback.OnUpgrade(*store, oldVersion + 1, dbVersion), OHOS::NativeRdb::E_OK);
    EXPECT_EQ(callback.OnUpgrade(*store, dbVersion, dbVersion + 1), OHOS::NativeRdb::E_OK);

    ret = AppEventStore::GetInstance().DestroyDbStore();
    EXPECT_EQ(ret, DB_SUCC);
}