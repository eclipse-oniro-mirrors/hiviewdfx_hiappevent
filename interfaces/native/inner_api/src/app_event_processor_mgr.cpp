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
#include "app_event_processor_mgr.h"

#include "app_event_store.h"
#include "app_event_observer_mgr.h"
#include "hiappevent_verify.h"
#include "module_loader.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
int64_t AppEventProcessorMgr::AddProcessor(const ReportConfig& config)
{
    ReportConfig realConfig = config;
    if (int ret = VerifyReportConfig(realConfig); ret != 0) {
        return ret;
    }
    if (int ret = ModuleLoader::GetInstance().Load(realConfig.name); ret != 0) {
        return ret;
    }
    return AppEventObserverMgr::GetInstance().RegisterObserver(realConfig.name, realConfig);
}

int AppEventProcessorMgr::RemoveProcessor(int64_t processorId)
{
    return AppEventObserverMgr::GetInstance().UnregisterObserver(processorId);
}

int AppEventProcessorMgr::RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor)
{
    return ModuleLoader::GetInstance().RegisterProcessor(name, processor);
}

int AppEventProcessorMgr::UnregisterProcessor(const std::string& name)
{
    if (int ret = AppEventObserverMgr::GetInstance().UnregisterObserver(name); ret < 0) {
        return ret;
    }
    return ModuleLoader::GetInstance().UnregisterProcessor(name);
}

int AppEventProcessorMgr::SetProcessorConfig(int64_t processorSeq, const ReportConfig& config)
{
    return AppEventObserverMgr::GetInstance().SetReportConfig(processorSeq, config);
}

int AppEventProcessorMgr::GetProcessorConfig(int64_t processorSeq, ReportConfig& config)
{
    return AppEventObserverMgr::GetInstance().GetReportConfig(processorSeq, config);
}

int AppEventProcessorMgr::GetProcessorSeqs(const std::string& name, std::vector<int64_t>& processorSeqs)
{
    processorSeqs.clear(); // prevent repeated invoking scenarios
    return AppEventStore::GetInstance().QueryObserverSeqs(name, processorSeqs);
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
