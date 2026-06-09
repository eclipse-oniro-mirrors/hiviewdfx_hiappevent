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
#include "app_api_metric.h"

#include "hiappevent_api_metric.h"
#include "hiappevent_base.h"
#include "hiappevent_facade.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
int ReportApiMetric(const ApiInfo& apiInfo, const ApiMetric& metric)
{
    if (!AppEventVerifyFacade::VerifyIsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }

    return ApiMetricProcessor::GetInstance().ProcessApiMetric(apiInfo, metric);
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
