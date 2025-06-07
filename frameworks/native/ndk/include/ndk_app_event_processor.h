/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef HIAPPEVENT_NDK_APPEVENT_PROCESSOR_H
#define HIAPPEVENT_NDK_APPEVENT_PROCESSOR_H

#include "base_type.h"

namespace OHOS {
namespace HiviewDFX {
class NdkAppEventProcessor {
public:
    explicit NdkAppEventProcessor(const std::string& name);
    NdkAppEventProcessor(const NdkAppEventProcessor&) = delete;
    NdkAppEventProcessor(const NdkAppEventProcessor&&) = delete;
    NdkAppEventProcessor& operator= (const NdkAppEventProcessor&) = delete;
    NdkAppEventProcessor& operator= (const NdkAppEventProcessor&&) = delete;

    int SetReportRoute(const char* appId, const char* routeInfo);
    int SetReportPolicy(int periodReport, int batchReport, bool onStartReport, bool onBackgroundReport);
    int SetReportEvent(const char* domain, const char* name, bool isRealTime);
    int SetCustomConfig(const char* key, const char* value);
    int SetConfigId(int configId);
    int SetConfigName(const std::string& configName);
    int SetReportUserId(const char* const * userIdNames, int size);
    int SetReportUserProperty(const char* const * userPropertyNames, int size);

    HiAppEvent::ReportConfig GetConfig();
private:
    HiAppEvent::ReportConfig config_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_NDK_APPEVENT_PROCESSOR_H
