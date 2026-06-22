/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hiappevent_api_metric_test.h"

#include <iostream>

#define private public
#include "api_stats_mgr.h"
#include "api_stats_timer.h"
#undef private
#include "api_stats_types.h"
#include "app_api_metric.h"
#include "api_stats_aggregator.h"
#include "api_stats_storage.h"
#include "app_event_store.h"
#include "hiappevent_api_metric.h"
#include "hiappevent_base.h"
#include "hiappevent_facade.h"
#include "application_context.h"
#include "hiappevent_test_common.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiAppEvent;
namespace {
const std::string TEST_DIR = "/data/test/hiappevent/";
const std::string TEST_KIT = "test_kit";
const std::string TEST_API = "test_api";
const std::string TEST_KIT2 = "test_kit2";
const std::string TEST_API2 = "test_api2";
constexpr int TEST_ERR_CODE = 0;
constexpr int TEST_ERR_CODE2 = 1;
constexpr int TEST_DURATION = 100;
constexpr int TEST_DURATION2 = 200;
constexpr bool TEST_SUCCESSFUL = true;
constexpr bool TEST_SUCCESSFUL2 = false;
constexpr int32_t TEST_UID = 200000 * 100;
}

void HiAppEventApiMetricTest::SetUpTestCase()
{
    setuid(TEST_UID);
    AppEventConfigFacade::SetStorageDir(TEST_DIR);
    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (context != nullptr) {
        auto contextImpl = std::make_shared<TestContextImpl>("ohos.hiappevent.test");
        context->AttachContextImpl(contextImpl);
        std::cout << "set bundle name." << std::endl;
        return;
    }
    std::cout << "context is null." << std::endl;
}

void HiAppEventApiMetricTest::TearDownTestCase() {}

void HiAppEventApiMetricTest::SetUp()
{
    AppEventStore::GetInstance().InitDbStore();
}

void HiAppEventApiMetricTest::TearDown()
{
    AppEventStore::GetInstance().DestroyDbStore();
}

/**
 * @tc.name: ReportApiMetricTest001
 * @tc.desc: check the ReportApiMetric with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ReportApiMetricTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create valid ApiInfo and ApiMetric.
     * @tc.steps: step2. call ReportApiMetric().
     * @tc.steps: step3. check return value is success.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = TEST_API;

    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = true;

    HiAppEvent::ApiStatsManager ApiStatsMgr;
    HiAppEvent::ApiDescriptor descriptor(apiInfo.kit, apiInfo.api);
    ApiStatsMgr.AddRecord(descriptor, metric);
    ApiStatsMgr.ScheduleBackUp();
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> outBackUp;
    int ret = AppEventStore::GetInstance().QueryApiMetricInfoAll(outBackUp);
    EXPECT_EQ(ret, OHOS::NativeRdb::E_OK);
    EXPECT_EQ(outBackUp.size(), 1);

    
    ApiStatsMgr.ScheduleReport();
    std::map<std::pair<std::string, std::string>, std::vector<std::string>> outReport;
    ret = AppEventStore::GetInstance().QueryApiMetricInfoAll(outReport);
    EXPECT_EQ(ret, OHOS::NativeRdb::E_OK);
    EXPECT_EQ(outReport.size(), 0);

    ApiStatsMgr.ScheduleReport();
    ret = AppEventStore::GetInstance().QueryApiMetricInfoAll(outReport);
    EXPECT_EQ(ret, OHOS::NativeRdb::E_OK);
    EXPECT_EQ(outReport.size(), 0);
}

/**
 * @tc.name: ReportApiMetricTest002
 * @tc.desc: check the ReportApiMetric with non-app uid.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ReportApiMetricTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. set uid to non-app value.
     * @tc.steps: step2. call ReportApiMetric().
     * @tc.steps: step3. check return value is ERROR_NOT_APP.
     * @tc.steps: step4. restore app uid.
     */
    setuid(0); // 0 means root uid

    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = TEST_API;

    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = true;

    int ret = HiAppEvent::ReportApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_NOT_APP);

    // restore app uid
    setuid(TEST_UID);
}

/**
 * @tc.name: ReportApiMetricTest003
 * @tc.desc: check the ReportApiMetric with negative duration.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ReportApiMetricTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiInfo and ApiMetric with negative duration.
     * @tc.steps: step2. call ReportApiMetric().
     * @tc.steps: step3. check return value is error.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = TEST_API;

    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = -1;
    metric.successful = true;

    int ret = HiAppEvent::ReportApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: ReportApiMetricTest004
 * @tc.desc: check the ReportApiMetric with empty kit.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ReportApiMetricTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiInfo with empty kit.
     * @tc.steps: step2. call ReportApiMetric().
     * @tc.steps: step3. check return value is error.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = "";
    apiInfo.api = TEST_API;

    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = true;

    int ret = HiAppEvent::ReportApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: ReportApiMetricTest005
 * @tc.desc: check the ReportApiMetric with empty api.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ReportApiMetricTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiInfo with empty api.
     * @tc.steps: step2. call ReportApiMetric().
     * @tc.steps: step3. check return value is error.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = "";

    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = true;

    int ret = HiAppEvent::ReportApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: ApiDescriptorTest001
 * @tc.desc: check the ApiDescriptor constructor.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiDescriptorTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiDescriptor with kit and api.
     * @tc.steps: step2. check the kit and api values.
     */
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    EXPECT_EQ(descriptor.KitName(), TEST_KIT);
    EXPECT_EQ(descriptor.ApiName(), TEST_API);
}

/**
 * @tc.name: ApiDescriptorTest002
 * @tc.desc: check the ApiDescriptor KitName and ApiName functions.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiDescriptorTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiDescriptor.
     * @tc.steps: step2. call KitName() and ApiName().
     */
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    std::string kitName = descriptor.KitName();
    std::string apiName = descriptor.ApiName();
    EXPECT_EQ(kitName, TEST_KIT);
    EXPECT_EQ(apiName, TEST_API);
}

/**
 * @tc.name: ApiDescriptorTest003
 * @tc.desc: check the ApiDescriptor Description function.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiDescriptorTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiDescriptor.
     * @tc.steps: step2. call Description().
     */
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    std::string desc = descriptor.Description();
    std::string expectedDesc = TEST_KIT + ":" + TEST_API;
    EXPECT_EQ(desc, expectedDesc);
}

/**
 * @tc.name: ApiDescriptorTest004
 * @tc.desc: check the ApiDescriptorComparator sorting logic.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiDescriptorTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create two ApiDescriptors.
     * @tc.steps: step2. compare them using ApiDescriptorComparator.
     */
    ApiDescriptor descriptor1("aaa", "api1");
    ApiDescriptor descriptor2("bbb", "api2");
    ApiDescriptor::ApiDescriptorComparator comparator;
    EXPECT_TRUE(comparator(descriptor1, descriptor2));
    EXPECT_FALSE(comparator(descriptor2, descriptor1));
}

/**
 * @tc.name: ApiStatsAggregatorTest001
 * @tc.desc: check the ApiStatsAggregator Record function with single record.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsAggregator.
     * @tc.steps: step2. call Record() with single metric.
     * @tc.steps: step3. check the result with GetApiMetrics().
     */
    ApiStatsAggregator aggregator;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    aggregator.Record(descriptor, metric);
    
    auto apiMetrics = aggregator.GetApiMetrics();
    EXPECT_EQ(apiMetrics.size(), 1);
    EXPECT_TRUE(apiMetrics.find(descriptor) != apiMetrics.end());
    EXPECT_EQ(apiMetrics[descriptor].size(), 1);
}

/**
 * @tc.name: ApiStatsAggregatorTest002
 * @tc.desc: check the ApiStatsAggregator Record function with multiple records for same API.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsAggregator.
     * @tc.steps: step2. call Record() multiple times for same API.
     * @tc.steps: step3. check the result size.
     */
    ApiStatsAggregator aggregator;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric1{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    ApiMetric metric2{TEST_ERR_CODE2, TEST_DURATION2, TEST_SUCCESSFUL2};
    aggregator.Record(descriptor, metric1);
    aggregator.Record(descriptor, metric2);
    
    auto apiMetrics = aggregator.GetApiMetrics();
    EXPECT_EQ(apiMetrics.size(), 1);
    EXPECT_EQ(apiMetrics[descriptor].size(), 2);
}

/**
 * @tc.name: ApiStatsAggregatorTest003
 * @tc.desc: check the ApiStatsAggregator Record function with different APIs.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsAggregator.
     * @tc.steps: step2. call Record() for different APIs.
     * @tc.steps: step3. check the result.
     */
    ApiStatsAggregator aggregator;
    ApiDescriptor descriptor1(TEST_KIT, TEST_API);
    ApiDescriptor descriptor2(TEST_KIT2, TEST_API2);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    aggregator.Record(descriptor1, metric);
    aggregator.Record(descriptor2, metric);
    
    auto apiMetrics = aggregator.GetApiMetrics();
    EXPECT_EQ(apiMetrics.size(), 2);
    EXPECT_EQ(apiMetrics[descriptor1].size(), 1);
    EXPECT_EQ(apiMetrics[descriptor2].size(), 1);
}

/**
 * @tc.name: ApiStatsAggregatorTest004
 * @tc.desc: check the ApiStatsAggregator ClearRecord function.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsEntity and add records.
     * @tc.steps: step2. call ClearRecord().
     * @tc.steps: step3. check GetApiMetrics() is empty.
     */
    ApiStatsAggregator aggregator;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    aggregator.Record(descriptor, metric);
    aggregator.ClearRecord();
    
    auto apiMetrics = aggregator.GetApiMetrics();
    EXPECT_EQ(apiMetrics.size(), 0);
}

/**
 * @tc.name: ApiStatsAggregatorTest005
 * @tc.desc: check the ApiStatsAggregator IsUpdatedAfterLastBackup initial state.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsAggregator.
     * @tc.steps: step2. check initial IsUpdatedAfterLastBackup state.
     */
    ApiStatsAggregator aggregator;
    EXPECT_FALSE(aggregator.IsUpdatedAfterLastBackup());
}

/**
 * @tc.name: ApiStatsAggregatorTest006
 * @tc.desc: check the ApiStatsAggregator IsUpdatedAfterLastBackup after Record.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsAggregator.
     * @tc.steps: step2. call Record().
     * @tc.steps: step3. check IsUpdatedAfterLastBackup is true.
     */
    ApiStatsAggregator aggregator;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    aggregator.Record(descriptor, metric);
    EXPECT_TRUE(aggregator.IsUpdatedAfterLastBackup());
}

/**
 * @tc.name: ApiStatsAggregatorTest007
 * @tc.desc: check the ApiStatsAggregator IsUpdatedAfterLastBackup after ClearRecord.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest007, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiStatsEntity and add records.
     * @tc.steps: step2. call ClearRecord().
     * @tc.steps: step3. check IsUpdatedAfterLastBackup is false.
     */
    ApiStatsAggregator aggregator;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    aggregator.Record(descriptor, metric);
    aggregator.ClearRecord();
    EXPECT_FALSE(aggregator.IsUpdatedAfterLastBackup());
}

/**
 * @tc.name: ApiStatsAggregatorTest008
 * @tc.desc: check the ApiStatsAggregator AggregateStats with single metric.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest008, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create apiMetrics map with single metric.
     * @tc.steps: step2. call AggregateStats().
     * @tc.steps: step3. check the aggregated result.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    apiMetrics[descriptor].push_back(metric);
    
    auto reports = ApiStatsAggregator::AggregateStats(apiMetrics);
    EXPECT_EQ(reports.size(), 1);
    EXPECT_EQ(reports[0].kitName, TEST_KIT);
    EXPECT_EQ(reports[0].apiName, TEST_API);
    EXPECT_EQ(reports[0].callTimes, 1);
    EXPECT_EQ(reports[0].successTimes, 1);
    EXPECT_EQ(reports[0].maxCostTime, TEST_DURATION);
    EXPECT_EQ(reports[0].minCostTime, TEST_DURATION);
    EXPECT_EQ(reports[0].totalCostTime, TEST_DURATION);
}

/**
 * @tc.name: ApiStatsAggregatorTest009
 * @tc.desc: check the ApiStatsAggregator AggregateStats with multiple metrics.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest009, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create apiMetrics map with multiple metrics.
     * @tc.steps: step2. call AggregateStats().
     * @tc.steps: step3. check the aggregated statistics.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric1{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    ApiMetric metric2{TEST_ERR_CODE2, TEST_DURATION2, TEST_SUCCESSFUL2};
    apiMetrics[descriptor].push_back(metric1);
    apiMetrics[descriptor].push_back(metric2);
    
    auto reports = ApiStatsAggregator::AggregateStats(apiMetrics);
    EXPECT_EQ(reports.size(), 1);
    EXPECT_EQ(reports[0].callTimes, 2);
    EXPECT_EQ(reports[0].successTimes, 1);
    EXPECT_EQ(reports[0].maxCostTime, TEST_DURATION2);
    EXPECT_EQ(reports[0].minCostTime, TEST_DURATION);
    EXPECT_EQ(reports[0].totalCostTime, TEST_DURATION + TEST_DURATION2);
}

/**
 * @tc.name: ApiStatsAggregatorTest010
 * @tc.desc: check the ApiStatsAggregator AggregateStats error code statistics.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsAggregatorTest010, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create apiMetrics map with different error codes.
     * @tc.steps: step2. call AggregateStats().
     * @tc.steps: step3. check the error code statistics.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric1{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    ApiMetric metric2{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL2};
    ApiMetric metric3{TEST_ERR_CODE2, TEST_DURATION2, TEST_SUCCESSFUL2};
    apiMetrics[descriptor].push_back(metric1);
    apiMetrics[descriptor].push_back(metric2);
    apiMetrics[descriptor].push_back(metric3);
    
    auto reports = ApiStatsAggregator::AggregateStats(apiMetrics);
    EXPECT_EQ(reports.size(), 1);
    EXPECT_EQ(reports[0].errorCodeTypes.size(), 2);
    EXPECT_EQ(reports[0].errorCodeNum.size(), 2);
}

/**
 * @tc.name: ApiStatsStorageTest001
 * @tc.desc: check the ApiStatsStorage Backup with empty data.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsStorageTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create empty apiMetrics map.
     * @tc.steps: step2. call Backup().
     * @tc.steps: step3. check return value.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    int ret = ApiStatsStorage::GetInstance().Backup(apiMetrics);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ApiStatsStorageTest002
 * @tc.desc: check the ApiStatsStorage Backup with single record.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsStorageTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create apiMetrics map with single record.
     * @tc.steps: step2. call Backup().
     * @tc.steps: step3. check return value.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    apiMetrics[descriptor].push_back(metric);
    
    int ret = ApiStatsStorage::GetInstance().Backup(apiMetrics);
    EXPECT_EQ(ret, 0);
    
    ApiStatsStorage::GetInstance().Clear();
}

/**
 * @tc.name: ApiStatsStorageTest003
 * @tc.desc: check the ApiStatsStorage QueryAll after Backup.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsStorageTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Backup records to storage.
     * @tc.steps: step2. call QueryAll().
     * @tc.steps: step3. check the queried data.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    apiMetrics[descriptor].push_back(metric);
    
    ApiStatsStorage::GetInstance().Backup(apiMetrics);
    
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> queriedMetrics;
    int ret = ApiStatsStorage::GetInstance().QueryAll(queriedMetrics);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(queriedMetrics.size(), 1);
    
    ApiStatsStorage::GetInstance().Clear();
}

/**
 * @tc.name: ApiStatsStorageTest004
 * @tc.desc: check the ApiStatsStorage Clear function.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsStorageTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Backup records to storage.
     * @tc.steps: step2. call Clear().
     * @tc.steps: step3. QueryAll and check empty result.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor(TEST_KIT, TEST_API);
    ApiMetric metric{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    apiMetrics[descriptor].push_back(metric);
    
    ApiStatsStorage::GetInstance().Backup(apiMetrics);
    ApiStatsStorage::GetInstance().Clear();
    
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> queriedMetrics;
    int ret = ApiStatsStorage::GetInstance().QueryAll(queriedMetrics);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(queriedMetrics.size(), 0);
}

/**
 * @tc.name: ApiStatsStorageTest005
 * @tc.desc: check the ApiStatsStorage Backup+Query+Clear workflow.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsStorageTest005, TestSize.Level0)
{
    /**
     * @tc.steps: step1. Backup multiple records.
     * @tc.steps: step2. QueryAll and verify data.
     * @tc.steps: step3. Clear and verify empty.
     */
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> apiMetrics;
    ApiDescriptor descriptor1(TEST_KIT, TEST_API);
    ApiDescriptor descriptor2(TEST_KIT2, TEST_API2);
    ApiMetric metric1{TEST_ERR_CODE, TEST_DURATION, TEST_SUCCESSFUL};
    ApiMetric metric2{TEST_ERR_CODE2, TEST_DURATION2, TEST_SUCCESSFUL2};
    apiMetrics[descriptor1].push_back(metric1);
    apiMetrics[descriptor2].push_back(metric2);
    apiMetrics[descriptor2].push_back(metric2);
    
    EXPECT_EQ(ApiStatsStorage::GetInstance().Backup(apiMetrics), 0);
    
    std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator> queriedMetrics;
    EXPECT_EQ(ApiStatsStorage::GetInstance().QueryAll(queriedMetrics), 0);
    EXPECT_EQ(queriedMetrics.size(), 2);
    
    EXPECT_EQ(ApiStatsStorage::GetInstance().Clear(), 0);
    
    queriedMetrics.clear();
    EXPECT_EQ(ApiStatsStorage::GetInstance().QueryAll(queriedMetrics), 0);
    EXPECT_EQ(queriedMetrics.size(), 0);
}

/**
 * @tc.name: ApiStatsTimerTest001
 * @tc.desc: check the ApiStatsTimer time constants.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsTimerTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. check BACKUP_TIME_MS equals 10 seconds.
     * @tc.steps: step2. check REPORT_TIME_MS equals 60 seconds.
     */
    EXPECT_EQ(ApiStatsTimer::BACKUP_TIME_MS, 10 * 1000);
    EXPECT_EQ(ApiStatsTimer::REPORT_TIME_MS, 60 * 1000);
    EXPECT_EQ(ApiStatsTimer::MILLI_TO_MICRO, 1000);
}

/**
 * @tc.name: ApiStatsTimerTest002
 * @tc.desc: check the ApiStatsTimer SetBackUpCallback SetReportCallback nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsTimerTest002, TestSize.Level0)
{
    ApiStatsTimer timer;
    std::atomic<int> backupCount{0};
    std::atomic<int> reportCount{0};

    timer.SetBackUpCallback([&backupCount]() { backupCount++; });
    timer.SetReportCallback([&reportCount]() { reportCount++; });
    timer.Start();

    timer.ExecuteBackUpCallback();
    timer.ExecuteReportCallback();
    EXPECT_EQ(backupCount.load(), 1);
    EXPECT_EQ(reportCount.load(), 1);

    int backupBeforeNull = backupCount.load();
    int reportBeforeNull = reportCount.load();

    timer.SetBackUpCallback(nullptr);
    timer.SetReportCallback(nullptr);
    timer.ExecuteBackUpCallback();
    timer.ExecuteReportCallback();
    EXPECT_EQ(backupCount.load(), backupBeforeNull);
    EXPECT_EQ(reportCount.load(), reportBeforeNull);

    timer.Stop();
}

/**
 * @tc.name: ApiStatsTimerTest003
 * @tc.desc: check the ApiStatsTimer Stop.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsTimerTest003, TestSize.Level0)
{
    ApiStatsTimer timer;
    std::atomic<int> backupCount{0};
    std::atomic<int> reportCount{0};

    timer.SetBackUpCallback([&backupCount]() { backupCount++; });
    timer.SetReportCallback([&reportCount]() { reportCount++; });
    timer.Start();

    timer.ExecuteBackUpCallback();
    timer.ExecuteReportCallback();
    EXPECT_EQ(backupCount.load(), 1);
    EXPECT_EQ(reportCount.load(), 1);

    int backupBeforeStop = backupCount.load();
    int reportBeforeStop = reportCount.load();

    timer.Stop();
    timer.ExecuteBackUpCallback();
    timer.ExecuteReportCallback();
    EXPECT_EQ(backupCount.load(), backupBeforeStop);
    EXPECT_EQ(reportCount.load(), reportBeforeStop);
}

/**
 * @tc.name: ProcessApiMetricTest001
 * @tc.desc: check the ProcessApiMetric with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ProcessApiMetricTest001, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create valid ApiInfo and ApiMetric.
     * @tc.steps: step2. call ProcessApiMetric().
     * @tc.steps: step3. check return value is success.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = TEST_API;
    
    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = TEST_SUCCESSFUL;
    
    int ret = ApiMetricProcessor::GetInstance().ProcessApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL);
}

/**
 * @tc.name: ProcessApiMetricTest002
 * @tc.desc: check the ProcessApiMetric with negative duration.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ProcessApiMetricTest002, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiInfo and ApiMetric with negative duration.
     * @tc.steps: step2. call ProcessApiMetric().
     * @tc.steps: step3. check return value is error.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = TEST_API;
    
    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = -1;
    metric.successful = TEST_SUCCESSFUL;
    
    int ret = ApiMetricProcessor::GetInstance().ProcessApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: ProcessApiMetricTest003
 * @tc.desc: check the ProcessApiMetric with empty kit.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ProcessApiMetricTest003, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiInfo with empty kit.
     * @tc.steps: step2. call ProcessApiMetric().
     * @tc.steps: step3. check return value is error.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = "";
    apiInfo.api = TEST_API;
    
    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = TEST_SUCCESSFUL;
    
    int ret = ApiMetricProcessor::GetInstance().ProcessApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: ProcessApiMetricTest004
 * @tc.desc: check the ProcessApiMetric with empty api.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ProcessApiMetricTest004, TestSize.Level0)
{
    /**
     * @tc.steps: step1. create ApiInfo with empty api.
     * @tc.steps: step2. call ProcessApiMetric().
     * @tc.steps: step3. check return value is error.
     */
    HiAppEvent::ApiInfo apiInfo;
    apiInfo.kit = TEST_KIT;
    apiInfo.api = "";
    
    HiAppEvent::ApiMetric metric;
    metric.errCode = TEST_ERR_CODE;
    metric.duration = TEST_DURATION;
    metric.successful = TEST_SUCCESSFUL;
    
    int ret = ApiMetricProcessor::GetInstance().ProcessApiMetric(apiInfo, metric);
    EXPECT_EQ(ret, ErrorCode::ERROR_INVALID_PARAM_VALUE);
}

/**
 * @tc.name: ApiStatsStorageTest006
 * @tc.desc: check the ApiStatsStorage QueryAll with invalid json in database
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventApiMetricTest, ApiStatsStorageTest006, TestSize.Level0)
{
    /**
     * @tc.steps: step1. init db store
     * @tc.steps: step2. insert invalid json metric directly
     * @tc.steps: step3. call QueryAll() to trigger parse failure
     * @tc.steps: step4. verify invalid json is skipped
     */
    AppEventStore::GetInstance().InitDbStore();

    std::string invalidJson = "{invalid json";
    AppEventStore::GetInstance().InsertApiMetricInfo(TEST_KIT, TEST_API, invalidJson);

    std::string invalidJsonValue = "{\"errCode\":0,\"duration\":-1,\"successful\":true}";
    AppEventStore::GetInstance().InsertApiMetricInfo(TEST_KIT, TEST_API, invalidJsonValue);

    std::string validJson = "{\"errCode\":0,\"duration\":100,\"successful\":true}";
    AppEventStore::GetInstance().InsertApiMetricInfo(TEST_KIT, TEST_API, validJson);

    std::string invalidErrCode = "{\"errCode\":\"validVlue\",\"duration\":100,\"successful\":true}";
    std::string invalidDuration = "{\"errCode\":0,\"duration\":\"validVlue\",\"successful\":true}";
    std::string invalidSuccessful = "{\"errCode\":0,\"duration\":100,\"successful\":\"validVlue\"}";
    AppEventStore::GetInstance().InsertApiMetricInfo(TEST_KIT, TEST_API, invalidErrCode);
    AppEventStore::GetInstance().InsertApiMetricInfo(TEST_KIT, TEST_API, invalidDuration);
    AppEventStore::GetInstance().InsertApiMetricInfo(TEST_KIT, TEST_API, invalidSuccessful);

    ApiMetricsMap queriedMetrics;
    int ret = ApiStatsStorage::GetInstance().QueryAll(queriedMetrics);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(queriedMetrics.size(), 1);

    ApiStatsStorage::GetInstance().Clear();
    AppEventStore::GetInstance().DestroyDbStore();
}
