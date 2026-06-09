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

#include "api_stats_timer.h"

#include "ffrt.h"
#include "ffrt_inner.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ApiStatsTimer"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

class ApiStatsTimer::ApiStatsTimerImpl : public std::enable_shared_from_this<ApiStatsTimerImpl> {
public:
    ApiStatsTimerImpl() = default;
    ~ApiStatsTimerImpl()
    {
        Stop();
    }

    void Start()
    {
        if (isRunning_) {
            return;
        }
        HILOG_DEBUG(LOG_CORE, "ApiStatsTimer Start");
        isRunning_ = true;
        ScheduleBackUpTask();
        ScheduleReportTask();
    }

    void Stop()
    {
        HILOG_DEBUG(LOG_CORE, "ApiStatsTimer Stop");
        isRunning_ = false;
        backUpCallback_ = nullptr;
        reportCallback_ = nullptr;
    }

    void SetBackUpCallback(std::function<void()> callback)
    {
        backUpCallback_ = callback;
    }

    void SetReportCallback(std::function<void()> callback)
    {
        reportCallback_ = callback;
    }

    void ExecuteBackUpCallback()
    {
        HILOG_DEBUG(LOG_CORE, "ScheduleBackUpTask executing");
        if (backUpCallback_) {
            backUpCallback_();
            ScheduleBackUpTask();
        }
    }

    void ExecuteReportCallback()
    {
        HILOG_DEBUG(LOG_CORE, "ScheduleReportTask executing");
        if (reportCallback_) {
            reportCallback_();
            ScheduleReportTask();
        }
    }

private:
    void ScheduleBackUpTask()
    {
        HILOG_DEBUG(LOG_CORE, "ScheduleBackUpTask scheduled");
        std::weak_ptr<ApiStatsTimerImpl> weakImpl = shared_from_this();
        ffrt::submit([weakImpl] {
            auto impl = weakImpl.lock();
            if (!impl || !impl->isRunning_) {
                return;
            }
            impl->ExecuteBackUpCallback();
            }, ffrt::task_attr().name("flush_backup").delay(BACKUP_TIME_MS * MILLI_TO_MICRO));
    }

    void ScheduleReportTask()
    {
        HILOG_DEBUG(LOG_CORE, "ScheduleReportTask scheduled");
        std::weak_ptr<ApiStatsTimerImpl> weakImpl = shared_from_this();
        ffrt::submit([weakImpl] {
            auto impl = weakImpl.lock();
            if (!impl || !impl->isRunning_) {
                return;
            }
            impl->ExecuteReportCallback();
            }, ffrt::task_attr().name("flush_report").delay(REPORT_TIME_MS * MILLI_TO_MICRO));
    }

    bool isRunning_ = false;
    std::function<void()> backUpCallback_;
    std::function<void()> reportCallback_;
};

ApiStatsTimer::ApiStatsTimer() : impl_(std::make_shared<ApiStatsTimerImpl>()) {}

ApiStatsTimer::~ApiStatsTimer() = default;

void ApiStatsTimer::Start()
{
    impl_->Start();
}

void ApiStatsTimer::Stop()
{
    impl_->Stop();
}

void ApiStatsTimer::SetBackUpCallback(std::function<void()> callback)
{
    impl_->SetBackUpCallback(callback);
}

void ApiStatsTimer::SetReportCallback(std::function<void()> callback)
{
    impl_->SetReportCallback(callback);
}

void ApiStatsTimer::ExecuteBackUpCallback()
{
    impl_->ExecuteBackUpCallback();
}

void ApiStatsTimer::ExecuteReportCallback()
{
    impl_->ExecuteReportCallback();
}

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS