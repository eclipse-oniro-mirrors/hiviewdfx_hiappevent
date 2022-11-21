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
#include <iostream>
#include <map>

#include <gtest/gtest.h>

#include "app_event_cache.h"
#include "app_event_watcher_mgr.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";
const std::string TEST_WATCHER = "test_watcher";
const std::string TEST_WATCHER_ROW = "watcher_row";
const std::string TEST_WATCHER_SIZE = "watcher_size";
const std::string TEST_WATCHER_TIMEOUT = "watcher_time";
const std::string TEST_DOMAIN = "test_domain";
const std::string TEST_NAME = "test_name";
constexpr unsigned int TEST_TYPE = 1;
const std::string TEST_EVENT = R"~({"domain_":"hiappevent", "name_":"testEvent"})~";

class HiAppEventWatcherTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventWatcherTest::SetUp()
{
    AppEventCache::GetInstance()->Open(TEST_DIR);
}

void HiAppEventWatcherTest::TearDown()
{
    AppEventCache::GetInstance()->Close();
}

class AppEventWatcherTest : public AppEventWatcher {
public:
    AppEventWatcherTest(const std::string& name, const std::map<std::string, unsigned int>& filters,
        TriggerCondition cond) : AppEventWatcher(name, filters, cond) {}

    void OnTrigger(int row, int size) override
    {
        std::cout << GetName() << " onTrigger, row=" << row << ", size=" << size << std::endl;
        triggerTimes++;
    }

    int GetTriggerTimes()
    {
        return triggerTimes;
    }

private:
    int triggerTimes = 0;
};

void BuildSimpleFilters(std::map<std::string, unsigned int>& filters)
{
    filters.insert(std::make_pair(TEST_DOMAIN, 0xff)); // 0xff means all types
}

TriggerCondition BuildCondition(int row, int size, int timeOut)
{
    TriggerCondition cond = {
        .row = row,
        .size = size,
        .timeOut = timeOut,
    };
    return cond;
}

std::shared_ptr<AppEventWatcherTest> BuildSimpleWatcher()
{
    std::map<std::string, unsigned int> filters;
    TriggerCondition cond = BuildCondition(0, 0, 0);
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithRow()
{
    std::map<std::string, unsigned int> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(1, 0, 0); // row is 1
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER_ROW, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithSize()
{
    std::map<std::string, unsigned int> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(0, 10, 0); // size is 10 byte
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER_SIZE, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithTimeout()
{
    std::map<std::string, unsigned int> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(0, 0, 1); // timeout is 1
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER_TIMEOUT, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithTimeout2()
{
    std::map<std::string, unsigned int> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(0, 0, 1); // timeout is 1
    return std::make_shared<AppEventWatcherTest>("watcher_time2", filters, cond);
}
}

/**
 * @tc.name: HiAppEventWatcherTest001
 * @tc.desc: Test to add watcher with no condition.
 * @tc.type: FUNC
 * @tc.require: issueI5LB4N
 */
HWTEST_F(HiAppEventWatcherTest, HiAppEventWatcherTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create AppEventWatcher object.
     * @tc.steps: step2. add the watcher to AppEventWatcherMgr.
     */
    std::cout << "HiAppEventWatcherTest001 start" << std::endl;
    auto watcherMgr = AppEventWatcherMgr::GetInstance();
    ASSERT_NE(watcherMgr, nullptr);

    auto watcher1 = BuildSimpleWatcher();
    watcherMgr->AddWatcher(watcher1);
    auto watcher2 = BuildWatcherWithRow();
    watcherMgr->AddWatcher(watcher2);
    auto watcher3 = BuildWatcherWithSize();
    watcherMgr->AddWatcher(watcher3);
    auto watcher4 = BuildWatcherWithTimeout();
    watcherMgr->AddWatcher(watcher4);
    auto watcher5 = BuildWatcherWithTimeout2();
    watcherMgr->AddWatcher(watcher5);

    watcherMgr->HandleEvent(TEST_DOMAIN, TEST_TYPE, TEST_EVENT);
    ASSERT_EQ(watcher1->GetTriggerTimes(), 0);
    ASSERT_EQ(watcher2->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher3->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher4->GetTriggerTimes(), 0);

    watcherMgr->HandleEvent("invalid_domain", TEST_TYPE, TEST_EVENT);
    ASSERT_EQ(watcher1->GetTriggerTimes(), 0);
    ASSERT_EQ(watcher2->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher3->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher4->GetTriggerTimes(), 0);

    watcherMgr->HandleTimeOut();
    ASSERT_EQ(watcher4->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher5->GetTriggerTimes(), 1);

    watcherMgr->RemoveWatcher(watcher1->GetName());
    watcherMgr->RemoveWatcher(watcher2->GetName());
    watcherMgr->RemoveWatcher(watcher3->GetName());
    watcherMgr->RemoveWatcher(watcher4->GetName());
    watcherMgr->RemoveWatcher(watcher5->GetName());
    std::cout << "HiAppEventWatcherTest001 end" << std::endl;
}

/**
 * @tc.name: HiAppEventWatcherTest002
 * @tc.desc: Test failed to add watcher.
 * @tc.type: FUNC
 * @tc.require: issueI5LB4N
 */
HWTEST_F(HiAppEventWatcherTest, HiAppEventWatcherTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create AppEventWatcher object.
     * @tc.steps: step2. add the watcher to AppEventWatcherMgr.
     */
    std::cout << "HiAppEventWatcherTest002 start" << std::endl;
    AppEventCache::GetInstance()->Close();

    auto watcher = BuildSimpleWatcher();
    auto watcherMgr = AppEventWatcherMgr::GetInstance();
    ASSERT_NE(watcherMgr, nullptr);
    watcherMgr->AddWatcher(watcher);
    watcherMgr->HandleEvent(TEST_DOMAIN, TEST_TYPE, TEST_EVENT);
    ASSERT_EQ(watcher->GetTriggerTimes(), 0);

    watcherMgr->RemoveWatcher(watcher->GetName());
    std::cout << "HiAppEventWatcherTest002 end" << std::endl;
}

/**
 * @tc.name: HiAppEventWatcherTest003
 * @tc.desc: Test to add watcher repeatedly.
 * @tc.type: FUNC
 * @tc.require: issueI5LB4N
 */
HWTEST_F(HiAppEventWatcherTest, HiAppEventWatcherTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create AppEventWatcher object.
     * @tc.steps: step2. add the watcher to AppEventWatcherMgr.
     */
    std::cout << "HiAppEventWatcherTest003 start" << std::endl;

    auto watcher1 = BuildWatcherWithRow();
    auto watcherMgr = AppEventWatcherMgr::GetInstance();
    ASSERT_NE(watcherMgr, nullptr);
    watcherMgr->AddWatcher(watcher1);
    auto watcher2 = BuildWatcherWithRow();
    watcherMgr->AddWatcher(watcher2);

    watcherMgr->HandleEvent(TEST_DOMAIN, TEST_TYPE, TEST_EVENT);
    ASSERT_EQ(watcher1->GetTriggerTimes(), 0);
    ASSERT_EQ(watcher2->GetTriggerTimes(), 1);

    watcherMgr->RemoveWatcher(watcher1->GetName());
    watcherMgr->RemoveWatcher(watcher2->GetName());
    std::cout << "HiAppEventWatcherTest003 end" << std::endl;
}
