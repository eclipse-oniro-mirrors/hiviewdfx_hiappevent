/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "app_event_watcher.h"
#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiAppEvent;

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

std::shared_ptr<AppEventPack> CreateAppEventPack(const std::string& domain = TEST_DOMAIN)
{
    return std::make_shared<AppEventPack>(domain, TEST_NAME, TEST_TYPE);
}

class HiAppEventWatcherTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventWatcherTest::SetUp()
{
    HiAppEventConfig::GetInstance().SetStorageDir(TEST_DIR);
    (void)AppEventStore::GetInstance().InitDbStore();
}

void HiAppEventWatcherTest::TearDown()
{
    (void)AppEventStore::GetInstance().DestroyDbStore();
}

class AppEventWatcherTest : public AppEventWatcher {
public:
    AppEventWatcherTest(
        const std::string& name,
        const std::vector<AppEventFilter>& filters,
        TriggerCondition cond)
        : AppEventWatcher(name, filters, cond) {}

    void OnTrigger(const TriggerCondition& triggerCond) override
    {
        std::cout << GetName() << " onTrigger, row=" << triggerCond.row << ", size=" << triggerCond.size << std::endl;
        triggerTimes++;
    }

    int GetTriggerTimes()
    {
        return triggerTimes;
    }

    void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) override
    {
        std::cout << GetName() << " OnEvents size=" << events.size() << std::endl;
        for (const auto& event : events) {
            std::cout << "domain=" << event->GetDomain() << ", eventName=" << event->GetName()
                << ", eventType=" << event->GetType() << std::endl;
            std::cout << "params=" << event->GetParamStr() << std::endl;
        }
    }

private:
    int triggerTimes = 0;
};

void BuildSimpleFilters(std::vector<AppEventFilter>& filters)
{
    filters.emplace_back(AppEventFilter(TEST_DOMAIN, 0xff)); // 0xff means all types
}

void BuildSimpleOsFilters(std::vector<AppEventFilter>& filters)
{
    filters.emplace_back(AppEventFilter("OS", {"APP_CRASH"}));
}

TriggerCondition BuildCondition(int row, int size, int timeout)
{
    TriggerCondition cond = {
        .row = row,
        .size = size,
        .timeout = timeout,
    };
    return cond;
}

std::shared_ptr<AppEventWatcherTest> BuildSimpleWatcher()
{
    std::vector<AppEventFilter> filters;
    TriggerCondition cond = BuildCondition(0, 0, 0);
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithRow()
{
    std::vector<AppEventFilter> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(1, 0, 0); // row is 1
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER_ROW, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithSize()
{
    std::vector<AppEventFilter> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(0, 10, 0); // size is 10 byte
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER_SIZE, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithTimeout()
{
    std::vector<AppEventFilter> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(0, 0, 1); // timeout is 1
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER_TIMEOUT, filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildWatcherWithTimeout2()
{
    std::vector<AppEventFilter> filters;
    BuildSimpleFilters(filters);
    TriggerCondition cond = BuildCondition(0, 0, 1); // timeout is 1
    return std::make_shared<AppEventWatcherTest>("watcher_time2", filters, cond);
}

std::shared_ptr<AppEventWatcherTest> BuildSimpleOsWatcher()
{
    std::vector<AppEventFilter> filters;
    BuildSimpleOsFilters(filters);
    TriggerCondition cond = BuildCondition(0, 0, 0);
    return std::make_shared<AppEventWatcherTest>(TEST_WATCHER, filters, cond);
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
     * @tc.steps: step2. add the watcher to AppEventObserverMgr.
     */
    std::cout << "HiAppEventWatcherTest001 start" << std::endl;

    auto watcher1 = BuildSimpleWatcher();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher1);
    auto watcher2 = BuildWatcherWithRow();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher2);
    auto watcher3 = BuildWatcherWithSize();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher3);
    auto watcher4 = BuildWatcherWithTimeout();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher4);
    auto watcher5 = BuildWatcherWithTimeout2();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher5);

    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(CreateAppEventPack());
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    ASSERT_EQ(watcher1->GetTriggerTimes(), 0);
    ASSERT_EQ(watcher2->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher3->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher4->GetTriggerTimes(), 0);

    events.clear();
    events.emplace_back(CreateAppEventPack("invalid_domain"));
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    ASSERT_EQ(watcher1->GetTriggerTimes(), 0);
    ASSERT_EQ(watcher2->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher3->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher4->GetTriggerTimes(), 0);

    AppEventObserverMgr::GetInstance().HandleTimeout();
    ASSERT_EQ(watcher4->GetTriggerTimes(), 1);
    ASSERT_EQ(watcher5->GetTriggerTimes(), 1);

    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher1->GetName());
    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher2->GetName());
    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher3->GetName());
    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher4->GetName());
    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher5->GetName());
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
     * @tc.steps: step2. add the watcher to AppEventObserverMgr.
     */
    std::cout << "HiAppEventWatcherTest002 start" << std::endl;
    (void)AppEventStore::GetInstance().DestroyDbStore();

    auto watcher = BuildSimpleWatcher();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher);
    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(CreateAppEventPack());
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    ASSERT_EQ(watcher->GetTriggerTimes(), 0);

    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher->GetName());
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
     * @tc.steps: step2. add the watcher to AppEventObserverMgr.
     */
    std::cout << "HiAppEventWatcherTest003 start" << std::endl;

    auto watcher1 = BuildWatcherWithRow();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher1);
    auto watcher2 = BuildWatcherWithRow();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher2);

    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(CreateAppEventPack());
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    ASSERT_EQ(watcher1->GetTriggerTimes(), 0);
    ASSERT_EQ(watcher2->GetTriggerTimes(), 1);

    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher1->GetName());
    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher2->GetName());
    std::cout << "HiAppEventWatcherTest003 end" << std::endl;
}

/**
 * @tc.name: HiAppEventWatcherTest004
 * @tc.desc: Test to add watcher onReceive.
 * @tc.type: FUNC
 * @tc.require: issueI5LB4N
 */
HWTEST_F(HiAppEventWatcherTest, HiAppEventWatcherTest004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create AppEventWatcher object.
     * @tc.steps: step2. add the watcher to AppEventObserverMgr.
     */
    std::cout << "HiAppEventWatcherTest004 start" << std::endl;

    auto watcher = BuildSimpleOsWatcher();
    AppEventObserverMgr::GetInstance().RegisterObserver(watcher);
    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(std::make_shared<AppEventPack>("OS", "APP_CRASH", TEST_TYPE));
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    ASSERT_EQ(watcher->GetTriggerTimes(), 0);

    AppEventObserverMgr::GetInstance().UnregisterObserver(watcher->GetName());
    std::cout << "HiAppEventWatcherTest004 end" << std::endl;
}
