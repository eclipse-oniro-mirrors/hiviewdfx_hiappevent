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

#include <gtest/gtest.h>

#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_read.h"
#include "hiappevent_write.h"
#include "time_util.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
const std::string TEST_NAME = "test_name";
constexpr unsigned int TEST_TYPE = 1;
const std::string TEST_PATH = "/data/test/hiappevent/";
const std::string TEST_EVENT = R"~({"domain_":"hiappevent", "name_":"testEvent"})~";
constexpr int64_t INVALID_TIME = -1;
constexpr int64_t INVALID_COUNT = -1;
}

class HiAppEventReadTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HiAppEventReadTest::SetUp()
{}

void HiAppEventReadTest::TearDown()
{}

/**
 * @tc.name: HiAppEventReadTest001
 * @tc.desc: Check the real time log update function.
 * @tc.type: FUNC
 * @tc.require: issueI621G6
 */
HWTEST_F(HiAppEventReadTest, HiAppEventReadTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Regist an event log real time listener.
     * @tc.steps: step2. Update an event.
     * @tc.steps: step3. Remove event log listeners.
     */
    std::cout << "HiAppEventReadTest001 start" << std::endl;

    auto testListener = [](const std::string& log) {
        std::cout << log << std::endl;
        ASSERT_EQ(log, TEST_EVENT);
    };
    RegRealTimeAppLogListener(testListener);
    RealTimeAppLogUpdate(TEST_EVENT);
    RemoveAllListeners();

    std::cout << "HiAppEventReadTest001 end" << std::endl;
}

/**
 * @tc.name: HiAppEventReadTest002
 * @tc.desc: Check the history log update function.
 * @tc.type: FUNC
 * @tc.require: issueI621G6
 */
HWTEST_F(HiAppEventReadTest, HiAppEventReadTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. Regist an event log history listener.
     * @tc.steps: step2. Persist a self-constructed event log to local file.
     * @tc.steps: step3. Remove event log listeners.
     */
    std::cout << "HiAppEventReadTest002 start" << std::endl;

    auto testListener = [](const std::vector<std::string>& logs) {
        std::for_each(logs.cbegin(), logs.cend(), [](const std::string& log) {
            std::cout << log << std::endl;
        });
        ASSERT_GT(logs.size(), 0);
    };
    RegHistoryAppLogListener(testListener);

    HiAppEventConfig::GetInstance().SetStorageDir(TEST_PATH);
    UpdateHiAppEventLogDir("/data/test");
    WriteEvent(std::make_shared<AppEventPack>(TEST_NAME, TEST_TYPE));

    // read all files
    PullEventHistoryLog(INVALID_TIME, INVALID_TIME, INVALID_COUNT);

    // read files up to current time
    int64_t curTime = static_cast<int64_t>(TimeUtil::GetMilliseconds());
    constexpr int eventCount = 1;
    PullEventHistoryLog(INVALID_TIME, curTime, eventCount);

    // read files greater than 0 but less than current time
    PullEventHistoryLog(0, curTime, eventCount);

    // read files greater than 0
    PullEventHistoryLog(0, INVALID_TIME, eventCount);

    RemoveAllListeners();
    (void)FileUtil::ForceRemoveDirectory(TEST_PATH, true);
    std::cout << "HiAppEventReadTest002 end" << std::endl;
}
