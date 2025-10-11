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
#include "event_config_mgr.h"

#include <application_context.h>
#include <hilog/log.h>

#include "file_util.h"
#include "hiappevent_base.h"
#include "storage_acl.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EventConfigMgr"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr const char* const APP_EVENT_DIR = "/eventConfig";

std::string GetSandBoxConfigDir()
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "Context is null.");
        return "";
    }
    if (context->GetCacheDir().empty()) {
        HILOG_ERROR(LOG_CORE, "cache dir obtained from context is empty.");
        return "";
    }
    std::string configDir = context->GetCacheDir() + APP_EVENT_DIR;
    if (!FileUtil::IsFileExists(configDir) && !FileUtil::ForceCreateDirectory(configDir)) {
        HILOG_ERROR(LOG_CORE, "failed to create dir=%{public}s", configDir.c_str());
        return "";
    }
    if (OHOS::StorageDaemon::AclSetAccess(configDir, "u:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set acl access dir=%{public}s", configDir.c_str());
        return "";
    }
    return configDir;
}
}

EventConfigMgr& EventConfigMgr::GetInstance()
{
    static EventConfigMgr instance;
    return instance;
}

int EventConfigMgr::SetEventConfig(const std::map<std::string, std::string>& configMap) const
{
    std::string configDir = GetSandBoxConfigDir();
    if (configDir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to get sandbox config dir");
        return ErrorCode::ERROR_UNKNOWN;
    }
    for (const auto& config : configMap) {
        std::string property = "user.event_config." + config.first;
        if (!FileUtil::SetDirXattr(configDir, property, config.second)) {
            HILOG_ERROR(LOG_CORE, "failed to SetDirXattr, dir: %{public}s, property: %{public}s, newValue: %{public}s,"
                " err: %{public}s, errno: %{public}d", configDir.c_str(), property.c_str(), config.second.c_str(),
                strerror(errno), errno);
            return ErrorCode::ERROR_UNKNOWN;
        } else {
            HILOG_INFO(LOG_CORE, "succeed to UpdateProperty, property: %{public}s, newValue: %{public}s",
                property.c_str(), config.second.c_str());
        }
    }
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}
}  // HiviewDFX
}  // OHOS
