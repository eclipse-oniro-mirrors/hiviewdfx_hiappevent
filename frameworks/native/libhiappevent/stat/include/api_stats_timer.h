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

#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_TIMER_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_TIMER_H

#include <functional>
#include <memory>

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

class ApiStatsTimer {
public:
    ApiStatsTimer();
    ~ApiStatsTimer();

    void Start();
    void Stop();
    void SetBackUpCallback(std::function<void()> callback);
    void SetReportCallback(std::function<void()> callback);

    static constexpr int BACKUP_TIME_MS = 10 * 1000;
    static constexpr int REPORT_TIME_MS = 60 * 1000;
    static constexpr int MILLI_TO_MICRO = 1000;

private:
    class ApiStatsTimerImpl;
    std::shared_ptr<ApiStatsTimerImpl> impl_;
};

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_TIMER_H