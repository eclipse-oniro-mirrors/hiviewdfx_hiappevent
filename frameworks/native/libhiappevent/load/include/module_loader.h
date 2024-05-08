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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_LOAD_MODULE_LOADER_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_LOAD_MODULE_LOADER_H

#include <mutex>
#include <unordered_map>

#include "app_event_observer.h"
#include "app_event_processor.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class ModuleLoader {
public:
    ModuleLoader() = default;
    ~ModuleLoader();
    static ModuleLoader& GetInstance();
    int Load(const std::string& moduleName);
    int Unload(const std::string& moduleName);
    int RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor);
    int UnregisterProcessor(const std::string& name);
    std::shared_ptr<AppEventObserver> CreateProcessorProxy(const std::string& name);

private:
    /* <module name, module handler> */
    std::unordered_map<std::string, void*> modules_;

    /* <processor name, processor object> */
    std::unordered_map<std::string, std::shared_ptr<AppEventProcessor>> processors_;

private:
    static std::mutex instanceMutex_;
    static std::mutex moduleMutex_;
    static std::mutex processorMutex_;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_LOAD_MODULE_LOADER_H
