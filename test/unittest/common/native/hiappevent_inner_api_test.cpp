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

#include <gtest/gtest.h>

#include "app_event_processor_mgr.h"
#include "app_event_watcher_mgr.h"
#include "hiappevent_base.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiAppEvent;
namespace {
void WriteEventOnce()
{
    constexpr int testType = 1;
    auto event = std::make_shared<AppEventPack>("test_domain", "test_name", testType);
    constexpr int testInt = 1;
    event->AddParam("int_key", testInt);
    constexpr double testDou = 1.2;
    event->AddParam("double_key", testDou);
    constexpr bool testBool = false;
    event->AddParam("bool_key", testBool);
    const std::string testStr = "str";
    event->AddParam("str_key", testStr);
    AppEventWatcherMgr::GetInstance()->HandleEvent(event);
}
}

class HiAppEventInnerApiTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};


class AppEventProcessorTest : public AppEventProcessor {
public:
    int OnReport(
        const std::vector<UserId>& userIds,
        const std::vector<UserProperty>& userProperties,
        const std::vector<AppEventInfo>& events) override;
};

int AppEventProcessorTest::OnReport(
    const std::vector<UserId>& userIds,
    const std::vector<UserProperty>& userProperties,
    const std::vector<AppEventInfo>& events)
{
    std::cout << "UserId size=" << userIds.size() << std::endl;
    std::cout << "UserProperty size=" << userProperties.size() << std::endl;
    std::cout << "AppEventInfo size=" << events.size() << std::endl;
    if (events.size() == 0) {
        return 0;
    }
    for (auto& event : events) {
        std::cout << "AppEventInfo.domain=" << event.domain << std::endl;
        std::cout << "AppEventInfo.name=" << event.name << std::endl;
        std::cout << "AppEventInfo.eventType=" << event.eventType << std::endl;
        std::cout << "AppEventInfo.timestamp=" << event.timestamp << std::endl;
        std::cout << "AppEventInfo.params=" << event.params << std::endl;
    }
    return 0;
}

/**
 * @tc.name: HiAppEventInnerApiTest001
 * @tc.desc: check the api AppEventProcessorMgr.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create an AppEventProcessorTest object.
     * @tc.steps: step2. Register the AppEventProcessorTest object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessorTest object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    auto ret = AppEventProcessorMgr::RegisterProcessor("test_processor", processor);
    ASSERT_EQ(ret, 0);

    WriteEventOnce();

    ret = AppEventProcessorMgr::UnregisterProcessor("test_processor");
    ASSERT_EQ(ret, 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest002
 * @tc.desc: check the api AppEventProcessorMgr.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create an AppEventProcessorTest object.
     * @tc.steps: step2. Register the AppEventProcessorTest object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessorTest object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    auto ret = AppEventProcessorMgr::RegisterProcessor("test_processor", processor);
    ASSERT_EQ(ret, 0);
    ret = AppEventProcessorMgr::RegisterProcessor("test_processor", processor);
    ASSERT_EQ(ret, -1);
    ret = AppEventProcessorMgr::UnregisterProcessor("test_processor");
    ASSERT_EQ(ret, 0);
    ret = AppEventProcessorMgr::UnregisterProcessor("test_processor");
    ASSERT_EQ(ret, -1);
}
