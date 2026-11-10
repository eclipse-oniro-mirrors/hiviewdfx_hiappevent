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

#include <cstdint>
#include "hiappevent_base.h"
#include "api_stats_timer.h"
#include "fuzz_cases.h"

void FuzzApiStatsTimer(const uint8_t* data, size_t size)
{
    // static timer avoids rapid create/destroy cycles: stack timers submit
    // delayed FFRT tasks on each Start() that reference destroyed impls
    static OHOS::HiviewDFX::HiAppEvent::ApiStatsTimer t;
    t.SetBackUpCallback([]() {});
    t.SetReportCallback([]() {});
    // restrict Start() frequency via fuzz data to bound delayed task count,
    // otherwise each iteration submits 2 tasks and floods the FFRT queue
    if (size > 0 && data[0] == 0) {
        t.Start();
    }
    t.Stop();
}
