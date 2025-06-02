/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_LOAD_PROCESSOR_CONFIG_LOADER_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_LOAD_PROCESSOR_CONFIG_LOADER_H

#include <string>

#include "base_type.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class ProcessorConfigLoader {
public:
    bool LoadProcessorConfig(const std::string& processorName, const std::string& configName);
    ReportConfig GetReportConfig();
    ProcessorConfigLoader();
    ~ProcessorConfigLoader();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_LOAD_PROCESSOR_CONFIG_LOADER_H