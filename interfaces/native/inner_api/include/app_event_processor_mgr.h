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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_MGR_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_MGR_H

#include <memory>

#include "app_event_processor.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class AppEventProcessorMgr {
public:
    static int64_t AddProcessor(const ReportConfig& config);
    static int RemoveProcessor(int64_t processorId);

    static int RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor);
    static int UnregisterProcessor(const std::string& name);

    static int SetProcessorConfig(int64_t processorSeq, const ReportConfig& config);
    static int GetProcessorConfig(int64_t processorSeq, ReportConfig& config);

    static int GetProcessorSeqs(const std::string& name, std::vector<int64_t>& processorSeqs);
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_MGR_H
