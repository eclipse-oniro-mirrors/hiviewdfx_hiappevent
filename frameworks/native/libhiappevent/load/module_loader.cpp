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
#include "module_loader.h"

#include <dlfcn.h>

#include "app_event_processor_proxy.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_ModuleLoader" };

std::string GetModulePath(const std::string& moduleName)
{
    const std::string searchDirs[] = {
        "/system/lib/", "/system/lib64/"
    };
    std::string modulePath;
    std::string libName = "lib" + moduleName + ".z.so";
    for (auto& searchDir : searchDirs) {
        std::string tempModulePath = searchDir + libName;
        if (FileUtil::IsFileExists(tempModulePath)) {
            modulePath = tempModulePath;
            break;
        }
    }
    return modulePath;
}
}
ModuleLoader::~ModuleLoader()
{
    processors_.clear();

    for (auto it = modules_.begin(); it != modules_.end(); ++it) {
        dlclose(it->second);
        HiLog::Info(LABEL, "succ to unload module=%{public}s", it->first.c_str());
    }
    modules_.clear();
}

int ModuleLoader::Load(const std::string& moduleName)
{
    if (modules_.find(moduleName) != modules_.end()) {
        HiLog::Info(LABEL, "the module=%{public}s already exists", moduleName.c_str());
        return 0;
    }

    std::string modulePath = GetModulePath(moduleName);
    if (modulePath.empty()) {
        HiLog::Warn(LABEL, "the module=%{public}s does not exist.", moduleName.c_str());
        return -1;
    }
    void* handler = nullptr;
    if (handler = dlopen(modulePath.c_str(), RTLD_GLOBAL); handler == nullptr) {
        HiLog::Error(LABEL, "failed to load module=%{public}s, error=%{public}s.", modulePath.c_str(), dlerror());
        return -1;
    }
    HiLog::Info(LABEL, "succ to load module=%{public}s.", modulePath.c_str());
    modules_[moduleName] = handler;
    return 0;
}

int ModuleLoader::Unload(const std::string& moduleName)
{
    if (modules_.find(moduleName) == modules_.end()) {
        HiLog::Warn(LABEL, "the module=%{public}s does not exists", moduleName.c_str());
        return -1;
    }
    dlclose(modules_[moduleName]);
    modules_.erase(moduleName);
    HiLog::Info(LABEL, "succ to unload module=%{public}s", moduleName.c_str());
    return 0;
}

int ModuleLoader::RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor)
{
    if (name.empty() || processor == nullptr) {
        HiLog::Warn(LABEL, "the name or processor is invalid");
        return -1;
    }
    if (processors_.find(name) != processors_.end()) {
        HiLog::Warn(LABEL, "the processor already exists");
        return -1;
    }
    processors_[name] = processor;
    return 0;
}

int ModuleLoader::UnregisterProcessor(const std::string& name)
{
    if (processors_.find(name) == processors_.end()) {
        HiLog::Warn(LABEL, "the name is invalid");
        return -1;
    }
    processors_.erase(name);
    return 0;
}

std::shared_ptr<AppEventObserver> ModuleLoader::CreateProcessorProxy(const std::string& name)
{
    if (processors_.find(name) == processors_.end()) {
        HiLog::Warn(LABEL, "the name is invalid");
        return nullptr;
    }
    return std::make_shared<AppEventProcessorProxy>(name, processors_[name]);
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
