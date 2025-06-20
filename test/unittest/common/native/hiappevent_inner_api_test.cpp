/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <unistd.h>

#include <gtest/gtest.h>

#include "app_event_processor_mgr.h"
#include "app_event_observer_mgr.h"
#include "application_context.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_test_common.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::ErrorCode;
using namespace OHOS::HiviewDFX::HiAppEvent;
namespace {
const std::string TEST_PROCESSOR_NAME = "test_processor";
const std::string TEST_EVENT_DOMAIN = "test_domain";
const std::string TEST_EVENT_NAME = "test_name";
constexpr int TEST_EVENT_TYPE = 1;
constexpr int32_t TEST_UID = 200000 * 100;

void WriteEventOnce()
{
    auto event = std::make_shared<AppEventPack>(TEST_EVENT_DOMAIN, TEST_EVENT_NAME, TEST_EVENT_TYPE);
    constexpr int testInt = 1;
    event->AddParam("int_key", testInt);
    constexpr double testDou = 1.2;
    event->AddParam("double_key", testDou);
    constexpr bool testBool = false;
    event->AddParam("bool_key", testBool);
    const std::string testStr = "str";
    event->AddParam("str_key", testStr);
    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(event);
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    sleep(1); // 1s
}

void CheckRegisterObserver(const std::string& observer,
    std::shared_ptr<AppEventProcessor> processor, int64_t& processorSeq)
{
    ASSERT_EQ(AppEventProcessorMgr::RegisterProcessor(observer, processor), 0);
    processorSeq = AppEventObserverMgr::GetInstance().AddProcessor(observer);
    ASSERT_GT(processorSeq, 0);
}

void CheckRegisterObserverWithConfig(
    const std::string& observer,
    std::shared_ptr<AppEventProcessor> processor,
    const ReportConfig& config,
    int64_t& processorSeq)
{
    ASSERT_EQ(AppEventProcessorMgr::RegisterProcessor(observer, processor), 0);
    processorSeq = AppEventObserverMgr::GetInstance().AddProcessor(observer, config);
    ASSERT_GT(processorSeq, 0);
}

void CheckUnregisterObserver(const std::string& observer)
{
    ASSERT_EQ(AppEventProcessorMgr::UnregisterProcessor(observer), 0);
}

void CheckGetEmptyConfig(int64_t processorSeq)
{
    ReportConfig config;
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorSeq, config), 0);
    ASSERT_TRUE(config.name.empty());
    ASSERT_FALSE(config.debugMode);
    ASSERT_TRUE(config.routeInfo.empty());
    ASSERT_TRUE(config.appId.empty());
    ASSERT_EQ(config.triggerCond.row, 0);
    ASSERT_EQ(config.triggerCond.size, 0);
    ASSERT_EQ(config.triggerCond.timeout, 0);
    ASSERT_FALSE(config.triggerCond.onStartup);
    ASSERT_FALSE(config.triggerCond.onBackground);
    ASSERT_TRUE(config.userIdNames.empty());
    ASSERT_TRUE(config.userPropertyNames.empty());
    ASSERT_TRUE(config.eventConfigs.empty());
    ASSERT_EQ(config.configId, 0);
    ASSERT_TRUE(config.customConfigs.empty());
}

void CheckGetSeqs(const std::string& observer, const std::vector<int64_t>& expectSeqs)
{
    std::vector<int64_t> processorSeqs;
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorSeqs(observer, processorSeqs), 0);
    ASSERT_EQ(processorSeqs.size(), expectSeqs.size());

    // test repeated getting seqs
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorSeqs(observer, processorSeqs), 0);
    ASSERT_EQ(processorSeqs.size(), expectSeqs.size());
}

void CheckSameConfig(const ReportConfig& configA, const ReportConfig& configB)
{
    ASSERT_EQ(configA.name, configB.name);
    ASSERT_EQ(configA.debugMode, configB.debugMode);
    ASSERT_EQ(configA.routeInfo, configB.routeInfo);
    ASSERT_EQ(configA.appId, configB.appId);
    ASSERT_EQ(configA.triggerCond.row, configB.triggerCond.row);
    ASSERT_EQ(configA.triggerCond.size, configB.triggerCond.size);
    ASSERT_EQ(configA.triggerCond.timeout, configB.triggerCond.timeout);
    ASSERT_EQ(configA.triggerCond.onStartup, configB.triggerCond.onStartup);
    ASSERT_EQ(configA.triggerCond.onBackground, configB.triggerCond.onBackground);
    ASSERT_EQ(configA.userIdNames.size(), configB.userIdNames.size());
    ASSERT_EQ(configA.userPropertyNames.size(), configB.userPropertyNames.size());
    ASSERT_EQ(configA.eventConfigs.size(), configB.eventConfigs.size());
    ASSERT_EQ(configA.configId, configB.configId);
    ASSERT_EQ(configA.customConfigs.size(), configB.customConfigs.size());
}

void CheckSameConfig(int64_t processorSeq, const ReportConfig& testConfig)
{
    ReportConfig getConfig;
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorSeq, getConfig), 0);
    CheckSameConfig(testConfig, getConfig);
}

void CheckSetConfig(int64_t processorSeq)
{
    ReportConfig testConfig = {
        .name = "test_name",
        .debugMode = true,
        .routeInfo = "test_routeInfo",
        .appId = "test_appid",
        .triggerCond = {1, 1, 1, true, true},
        .userIdNames = {"test_id"},
        .userPropertyNames = {"test_property"},
        .eventConfigs = {{"test_domain", "test_name", true}},
        .configId = 1,
        .customConfigs = {{"str_key", "str_value"}},
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, testConfig), 0);
    ReportConfig getConfig;
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorSeq, getConfig), 0);
    CheckSameConfig(testConfig, getConfig);
}

void CheckSetRealTimeConfig(int64_t processorSeq)
{
    ReportConfig testConfig = {
        .eventConfigs = {{TEST_EVENT_DOMAIN, TEST_EVENT_NAME, true}},
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, testConfig), 0);
}

void CheckSetRowConfig(int64_t processorSeq)
{
    ReportConfig testConfig = {
        .triggerCond = {
            .row = 2, // 2 events
        },
        .eventConfigs = {{TEST_EVENT_DOMAIN, TEST_EVENT_NAME}},
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, testConfig), 0);
}

void CheckSetSizeConfig(int64_t processorSeq)
{
    ReportConfig testConfig = {
        .triggerCond = {
            .size = 300, // 300 byte, 2 events
        },
        .eventConfigs = {{TEST_EVENT_DOMAIN, TEST_EVENT_NAME}},
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, testConfig), 0);
}

void CheckSetTimeoutConfig(int64_t processorSeq)
{
    ReportConfig testConfig = {
        .triggerCond = {
            .timeout = 2, // 2s
        },
        .eventConfigs = {{TEST_EVENT_DOMAIN, TEST_EVENT_NAME}},
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, testConfig), 0);
}

void CheckSetOnBackgroundConfig(int64_t processorSeq)
{
    ReportConfig testConfig = {
        .triggerCond = {
            .onBackground = true,
        },
        .eventConfigs = {{TEST_EVENT_DOMAIN, TEST_EVENT_NAME}},
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, testConfig), 0);
}

void CheckOnReport(
    const std::vector<UserId>& userIds,
    const std::vector<UserProperty>& userProperties,
    const std::vector<AppEventInfo>& events)
{
    ASSERT_EQ(userIds.size(), 0);
    ASSERT_EQ(userProperties.size(), 0);
    ASSERT_GT(events.size(), 0);
    for (const auto& event : events) {
        ASSERT_EQ(event.domain, TEST_EVENT_DOMAIN);
        ASSERT_EQ(event.name, TEST_EVENT_NAME);
        ASSERT_EQ(event.eventType, TEST_EVENT_TYPE);
        ASSERT_GT(event.timestamp, 0);
        ASSERT_EQ(event.params, "{\"int_key\":1,\"double_key\":1.2,\"bool_key\":false,\"str_key\":\"str\"}\n");
    }
}
}

class HiAppEventInnerApiTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // set app uid
        setuid(TEST_UID);
        HiAppEventConfig::GetInstance().SetStorageDir("/data/test/hiappevent/");
        // set context bundle name
        auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
        if (context != nullptr) {
            auto contextImpl = std::make_shared<TestContextImpl>("ohos.hiappevent.innerapi.test");
            context->AttachContextImpl(contextImpl);
            std::cout << "set bundle name." << std::endl;
            return;
        }
        std::cout << "context is null." << std::endl;
    }

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

class AppEventProcessorTest : public AppEventProcessor {
public:
    int OnReport(
        int64_t processorSeq,
        const std::vector<UserId>& userIds,
        const std::vector<UserProperty>& userProperties,
        const std::vector<AppEventInfo>& events) override;
    int ValidateUserId(const UserId& userId) override;
    int ValidateUserProperty(const UserProperty& userProperty) override;
    int ValidateEvent(const AppEventInfo& event) override;
    int GetReportTimes() { return reportTimes_; }

private:
    int reportTimes_ = 0;
};

int AppEventProcessorTest::OnReport(
    int64_t processorSeq,
    const std::vector<UserId>& userIds,
    const std::vector<UserProperty>& userProperties,
    const std::vector<AppEventInfo>& events)
{
    reportTimes_++;

    std::cout << "UserId size=" << userIds.size() << std::endl;
    std::cout << "UserProperty size=" << userProperties.size() << std::endl;
    std::cout << "AppEventInfo size=" << events.size() << std::endl;
    for (const auto& event : events) {
        std::cout << "AppEventInfo.domain=" << event.domain << std::endl;
        std::cout << "AppEventInfo.name=" << event.name << std::endl;
        std::cout << "AppEventInfo.eventType=" << event.eventType << std::endl;
        std::cout << "AppEventInfo.timestamp=" << event.timestamp << std::endl;
        std::cout << "AppEventInfo.params=" << event.params << std::endl;
    }
    CheckOnReport(userIds, userProperties, events);
    return 0;
}

int AppEventProcessorTest::ValidateUserId(const UserId& userId)
{
    return (userId.name.find("test") == std::string::npos) ? -1 : 0;
}

int AppEventProcessorTest::ValidateUserProperty(const UserProperty& userProperty)
{
    return (userProperty.name.find("test") == std::string::npos) ? -1 : 0;
}

int AppEventProcessorTest::ValidateEvent(const AppEventInfo& event)
{
    return (event.domain.find("test") == std::string::npos) ? -1 : 0;
}

/**
 * @tc.name: HiAppEventInnerApiTest001
 * @tc.desc: check the api AppEventProcessorMgr.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create an AppEventProcessor object.
     * @tc.steps: step2. Register the AppEventProcessor object.
     * @tc.steps: step3. Get processor sequence by name.
     * @tc.steps: step4. Get processor config by sequence.
     * @tc.steps: step5. Set processor config by sequence.
     * @tc.steps: step6. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq = 0;
    CheckRegisterObserver(TEST_PROCESSOR_NAME, processor, processorSeq);
    CheckGetSeqs(TEST_PROCESSOR_NAME, {processorSeq});
    CheckGetEmptyConfig(processorSeq);
    CheckSetConfig(processorSeq);
    WriteEventOnce();
    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest002
 * @tc.desc: check the api AppEventProcessorMgr.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Create an AppEventProcessor object.
     * @tc.steps: step2. Register the AppEventProcessor object.
     * @tc.steps: step3. Register the same AppEventProcessor object.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     * @tc.steps: step5. Unregister the same AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    auto ret = AppEventProcessorMgr::RegisterProcessor(TEST_PROCESSOR_NAME, processor);
    ASSERT_EQ(ret, 0);
    ret = AppEventProcessorMgr::RegisterProcessor(TEST_PROCESSOR_NAME, processor);
    ASSERT_EQ(ret, -1);
    ret = AppEventProcessorMgr::UnregisterProcessor(TEST_PROCESSOR_NAME);
    ASSERT_EQ(ret, 0);
    ret = AppEventProcessorMgr::UnregisterProcessor(TEST_PROCESSOR_NAME);
    ASSERT_EQ(ret, -1);
}

/**
 * @tc.name: HiAppEventInnerApiTest003
 * @tc.desc: check the real-time event callback AppEventProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Register an AppEventProcessor object.
     * @tc.steps: step2. Set config to the AppEventProcessor object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq = 0;
    CheckRegisterObserver(TEST_PROCESSOR_NAME, processor, processorSeq);
    CheckSetRealTimeConfig(processorSeq);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 1);
    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest004
 * @tc.desc: check the row callback AppEventProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Register an AppEventProcessor object.
     * @tc.steps: step2. Set config to the AppEventProcessor object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq = 0;
    CheckRegisterObserver(TEST_PROCESSOR_NAME, processor, processorSeq);
    CheckSetRowConfig(processorSeq);

    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 1);

    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest005
 * @tc.desc: check the size callback AppEventProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Register an AppEventProcessor object.
     * @tc.steps: step2. Set config to the AppEventProcessor object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq = 0;
    CheckRegisterObserver(TEST_PROCESSOR_NAME, processor, processorSeq);
    CheckSetSizeConfig(processorSeq);

    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 1);

    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest006
 * @tc.desc: check the timeout callback AppEventProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Register an AppEventProcessor object.
     * @tc.steps: step2. Set config to the AppEventProcessor object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq = 0;
    CheckRegisterObserver(TEST_PROCESSOR_NAME, processor, processorSeq);
    CheckSetTimeoutConfig(processorSeq);

    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 0);
    sleep(HiAppEvent::TIMEOUT_STEP + 1); // 31s
    ASSERT_EQ(processor->GetReportTimes(), 1);

    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest007
 * @tc.desc: check the background callback AppEventProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Register an AppEventProcessor object.
     * @tc.steps: step2. Set config to the AppEventProcessor object.
     * @tc.steps: step3. Write an test event.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq = 0;
    CheckRegisterObserver(TEST_PROCESSOR_NAME, processor, processorSeq);
    CheckSetOnBackgroundConfig(processorSeq);

    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 0);
    AppEventObserverMgr::GetInstance().HandleBackground();
    sleep(1); // 1s
    ASSERT_EQ(processor->GetReportTimes(), 1);

    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest008
 * @tc.desc: check the startup callback AppEventProcessor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Register an AppEventProcessor object.
     * @tc.steps: step2. Write an test event.
     * @tc.steps: step3. Register an AppEventProcessor object with same configuration.
     * @tc.steps: step4. Unregister the AppEventProcessor object.
     */
    auto processor = std::make_shared<AppEventProcessorTest>();
    int64_t processorSeq1 = 0;
    ReportConfig config = {
        .name = TEST_PROCESSOR_NAME,
        .triggerCond = {
            .onStartup = true,
        },
        .eventConfigs = {{TEST_EVENT_DOMAIN, TEST_EVENT_NAME}},
    };
    CheckRegisterObserverWithConfig(TEST_PROCESSOR_NAME, processor, config, processorSeq1);

    ASSERT_EQ(processor->GetReportTimes(), 0);
    WriteEventOnce();
    ASSERT_EQ(processor->GetReportTimes(), 0);

    int64_t processorSeq2 = AppEventObserverMgr::GetInstance().AddProcessor(TEST_PROCESSOR_NAME, config);
    ASSERT_EQ(processorSeq1, processorSeq2);
    ASSERT_EQ(processor->GetReportTimes(), 0);

    CheckUnregisterObserver(TEST_PROCESSOR_NAME);
}

/**
 * @tc.name: HiAppEventInnerApiTest009
 * @tc.desc: Adding a processor that does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest009, TestSize.Level1)
{
    ReportConfig config = {
        .name = "invalid_processor",
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_EQ(processorId, -1);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest010
 * @tc.desc: Adding an existing processor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest010, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .debugMode = true,
        .routeInfo = "test_routeInfo",
        .appId = "test_appid",
        .triggerCond = {
            .row = 1,
            .timeout = 0,
            .onStartup = false,
            .onBackground = false,
        },
        .userIdNames = {"test_id"},
        .userPropertyNames = {"test_property"},
        .eventConfigs = {{"test_domain", "test_name", true}, {"test_domain", "test_realtime"}},
        .configId = 1,
        .customConfigs = {{"str_key", "str_value"}, {"str_key1", "str_value1"}},
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, config);
    WriteEventOnce();
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest011
 * @tc.desc: Adding an invalid processor with invalid name.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest011, TestSize.Level1)
{
    ReportConfig config = {
        .name = "",
    };
    ASSERT_EQ(AppEventProcessorMgr::AddProcessor(config), -1);

    constexpr size_t limitLen = 256;
    config.name = std::string(limitLen, 'a');
    ASSERT_EQ(AppEventProcessorMgr::AddProcessor(config), -1);

    config.name = "***dddd";
    ASSERT_EQ(AppEventProcessorMgr::AddProcessor(config), -1);

    config.name = "999aaaa";
    ASSERT_EQ(AppEventProcessorMgr::AddProcessor(config), -1);
}

/**
 * @tc.name: HiAppEventInnerApiTest012
 * @tc.desc: Adding an processor with invalid routeInfo.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest012, TestSize.Level1)
{
    constexpr size_t limitLen = 8 * 1024;
    ReportConfig config = {
        .name = "test_processor",
        .routeInfo = std::string(limitLen, 'a'),
    };
    int64_t processorId1 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId1, 0);
    CheckSameConfig(processorId1, config);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId1), 0);

    config.routeInfo = std::string(limitLen + 1, 'a');
    int64_t processorId2 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId2, 0);

    ReportConfig expectConfig = {
        .name = "test_processor",
        .routeInfo = "", // default routeInfo value
    };
    CheckSameConfig(processorId2, expectConfig);

    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId2), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest013
 * @tc.desc: Adding an processor with invalid appId.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest013, TestSize.Level1)
{
    constexpr size_t limitLen = 8 * 1024;
    ReportConfig config = {
        .name = "test_processor",
        .appId = std::string(limitLen, 'a'),
    };
    int64_t processorId1 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId1, 0);
    CheckSameConfig(processorId1, config);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId1), 0);

    config.appId = std::string(limitLen + 1, 'a');
    int64_t processorId2 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId2, 0);

    ReportConfig expectConfig = {
        .name = "test_processor",
        .appId = "", // default appId value
    };
    CheckSameConfig(processorId2, expectConfig);

    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId2), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest014
 * @tc.desc: Adding an processor with invalid triggerCond.row.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest014, TestSize.Level1)
{
    constexpr int limitRow = 1000;
    ReportConfig config = {
        .name = "test_processor",
        .triggerCond = {
            .row = limitRow,
        },
    };
    int64_t processorId1 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId1, 0);
    CheckSameConfig(processorId1, config);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId1), 0);

    config.triggerCond.row = -1;
    int64_t processorId2 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId2, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .triggerCond.row = 0, // default row value
    };
    CheckSameConfig(processorId2, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId2), 0);

    config.triggerCond.row = limitRow + 1;
    int64_t processorId3 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId3, 0);
    CheckSameConfig(processorId3, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId3), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest015
 * @tc.desc: Adding an processor with invalid triggerCond.timeout.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest015, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .triggerCond = {
            .timeout = -1,
        },
    };
    int64_t processorId1 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId1, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .triggerCond.timeout = 0, // default timeout value
    };
    CheckSameConfig(processorId1, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId1), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest016
 * @tc.desc: Adding an processor with invalid userIdNames.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest016, TestSize.Level1)
{
    constexpr size_t limitLen = 256;
    ReportConfig config = {
        .name = "test_processor",
        .userIdNames = {
            std::string(limitLen, 'a'), "id1"
        },
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, config);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.userIdNames = {"***xxxx", "id1"};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .userIdNames = {}, // default userIdNames value
    };
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.userIdNames = {"", "id1"};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.userIdNames = {std::string(limitLen + 1, 'a'), "id1"};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest017
 * @tc.desc: Adding an processor with invalid userIdNames.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest017, TestSize.Level1)
{
    constexpr size_t limitLen = 256;
    ReportConfig config = {
        .name = "test_processor",
        .userPropertyNames = {
            std::string(limitLen, 'a'), "id1"
        },
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, config);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.userPropertyNames = {"***xxxx", "id1"};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .userPropertyNames = {}, // default userPropertyNames value
    };
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.userPropertyNames = {"", "id1"};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.userPropertyNames = {std::string(limitLen + 1, 'a'), "id1"};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest018
 * @tc.desc: Adding an processor with invalid eventConfigs.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest018, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .eventConfigs = {
            {"***domain", "name"}
        },
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .eventConfigs = {}, // default eventConfigs value
    };
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.eventConfigs = {{"domain", "***name"}};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.eventConfigs = {{"", "", true}, {"test_domain", "test_name"}};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest019
 * @tc.desc: Adding an existing processor with empty domain.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest019, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .eventConfigs = {
            {"", TEST_EVENT_NAME, true}
        },
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, config);
    WriteEventOnce();
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest020
 * @tc.desc: Adding an existing processor with empty name.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest020, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .eventConfigs = {
            {TEST_EVENT_DOMAIN, "", true}
        },
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, config);
    WriteEventOnce();
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest021
 * @tc.desc: Adding an processor with same processorId.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest021, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configId = 1,
    };
    int64_t processorId1 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId1, 0);

    config.eventConfigs = {{"test_domain", "test_name", true}, {"test_domain", "test_realtime"}};
    int64_t processorId2 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId2, 0);
    ASSERT_EQ(processorId1, processorId2);

    config.configId = 0;
    int64_t processorId3 = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId3, 0);
    ASSERT_NE(processorId2, processorId3);

    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId1), 0);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId2), 0);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId3), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest022
 * @tc.desc: Adding an processor with invalid configId.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest022, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configId = -1,
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .configId = 0, // default configId
    };
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest023
 * @tc.desc: Adding an processor with invalid customConfigs.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest023, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .customConfigs = {{"", "test_str"}},
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    ReportConfig expectConfig = {
        .name = "test_processor",
        .customConfigs = {}, // default customConfigs value
    };
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    config.customConfigs = {{"**name", "value"}};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);

    constexpr size_t limitLen = 1024;
    config.customConfigs = {{"test_name", std::string(limitLen + 1, 'a')}};
    processorId = AppEventProcessorMgr::AddProcessor(config);
    ASSERT_GT(processorId, 0);
    CheckSameConfig(processorId, expectConfig);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}

/**
 * @tc.name: HiAppEventInnerApiTest024
 * @tc.desc: check the api AppEventProcessorMgr not app.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest024, TestSize.Level0)
{
    setuid(0); // 0 means root uid

    auto processor = std::make_shared<AppEventProcessorTest>();
    ASSERT_EQ(AppEventProcessorMgr::RegisterProcessor(TEST_PROCESSOR_NAME, processor), ERROR_NOT_APP);
    ASSERT_EQ(AppEventProcessorMgr::UnregisterProcessor(TEST_PROCESSOR_NAME), ERROR_NOT_APP);

    std::vector<int64_t> processorSeqs;
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorSeqs(TEST_PROCESSOR_NAME, processorSeqs), ERROR_NOT_APP);

    int64_t processorSeq = 0;
    ReportConfig config = {
        .name = TEST_PROCESSOR_NAME,
    };
    ASSERT_EQ(AppEventProcessorMgr::SetProcessorConfig(processorSeq, config), ERROR_NOT_APP);
    ASSERT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorSeq, config), ERROR_NOT_APP);

    ASSERT_EQ(AppEventProcessorMgr::AddProcessor(config), ERROR_NOT_APP);
    ASSERT_EQ(AppEventProcessorMgr::RemoveProcessor(processorSeq), ERROR_NOT_APP);

    // set app uid
    setuid(TEST_UID);
}

/**
 * @tc.name: HiAppEventInnerApiTest025
 * @tc.desc: test ReportConfig ToString func.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest025, TestSize.Level0)
{
    ReportConfig config = {
        .name = "test_processor",
        .debugMode = true,
        .routeInfo = "test_routeInfo",
        .appId = "test_appid",
        .triggerCond = {
            .row = 1,
            .timeout = 0,
            .onStartup = false,
            .onBackground = false,
        },
        .userIdNames = {"test_id"},
        .userPropertyNames = {"test_property"},
        .eventConfigs = {{"test_domain", "test_name", true}, {"test_domain", "test_realtime"}},
        .configId = 1,
        .customConfigs = {{"str_key", "str_value"}, {"str_key1", "str_value1"}},
        .configName = "test_configName",
    };
    std::string expectStr = std::string("{test_processor,1,test_routeInfo,test_appid,{1,0,0,0,0},[test_id],") +
        "[test_property],[{test_domain,test_name,1},{test_domain,test_realtime,0}],1,[{str_key1,str_value1}," +
        "{str_key,str_value}],test_configName}";
    EXPECT_TRUE(config.ToString() == expectStr);
}

/**
 * @tc.name: HiAppEventInnerApiTest026
 * @tc.desc: Adding a processor with valid configName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest026, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configName = "SDK_OCG",
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    EXPECT_GT(processorId, 0);
 
    EXPECT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}
 
/**
 * @tc.name: HiAppEventInnerApiTest027
 * @tc.desc: Adding an invalid processor with invalid configName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest027, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configName = "undefine",
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    EXPECT_GT(processorId, 0);
 
    ReportConfig realConfig;
    EXPECT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorId, realConfig), 0);
    std::string expectStr = "{test_processor,0,,,{0,0,0,0,0},[],[],[],0,[],undefine}";
    EXPECT_EQ(realConfig.ToString(), expectStr);
 
    EXPECT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}
 
/**
 * @tc.name: HiAppEventInnerApiTest028
 * @tc.desc: Adding an invalid processor with invalid configName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest028, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configName = "",
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    EXPECT_GT(processorId, 0);
 
    ReportConfig realConfig;
    EXPECT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorId, realConfig), 0);
    std::string expectStr = "{test_processor,0,,,{0,0,0,0,0},[],[],[],0,[],}";
    EXPECT_EQ(realConfig.ToString(), expectStr);
 
    EXPECT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}
 
/**
 * @tc.name: HiAppEventInnerApiTest029
 * @tc.desc: Adding an invalid processor with invalid configName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest029, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configName = "xxx***",
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    EXPECT_GT(processorId, 0);
 
    ReportConfig realConfig;
    EXPECT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorId, realConfig), 0);
    std::string expectStr = "{test_processor,0,,,{0,0,0,0,0},[],[],[],0,[],}";
    EXPECT_EQ(realConfig.ToString(), expectStr);
 
    EXPECT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}
 
/**
 * @tc.name: HiAppEventInnerApiTest030
 * @tc.desc: Adding an invalid processor with invalid configName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest030, TestSize.Level1)
{
    ReportConfig config = {
        .name = "test_processor",
        .configName = "123_processor",
    };
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    EXPECT_GT(processorId, 0);
 
    ReportConfig realConfig;
    EXPECT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorId, realConfig), 0);
    std::string expectStr = "{test_processor,0,,,{0,0,0,0,0},[],[],[],0,[],}";
    EXPECT_EQ(realConfig.ToString(), expectStr);
 
    EXPECT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}
 
/**
 * @tc.name: HiAppEventInnerApiTest031
 * @tc.desc: Adding an invalid processor with invalid configName.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventInnerApiTest, HiAppEventInnerApiTest031, TestSize.Level1)
{
    constexpr size_t limitLen = 256 + 1;
    ReportConfig config = {
        .name = "test_processor",
        .configName = std::string(limitLen, 'a'),
    };
    
    int64_t processorId = AppEventProcessorMgr::AddProcessor(config);
    EXPECT_GT(processorId, 0);
 
    ReportConfig realConfig;
    EXPECT_EQ(AppEventProcessorMgr::GetProcessorConfig(processorId, realConfig), 0);
    std::string expectStr = "{test_processor,0,,,{0,0,0,0,0},[],[],[],0,[],}";
    EXPECT_EQ(realConfig.ToString(), expectStr);
 
    EXPECT_EQ(AppEventProcessorMgr::RemoveProcessor(processorId), 0);
}