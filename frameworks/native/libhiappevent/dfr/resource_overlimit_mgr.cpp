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
#include "resource_overlimit_mgr.h"

#include <unordered_set>
#include <application_context.h>
#include <hilog/log.h>
#include "file_util.h"
#include "hiappevent_base.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002DA0

#undef LOG_TAG
#define LOG_TAG "ResourceOverlimitMgr"

namespace {
inline constexpr const char* const RAWHEAP = "/rawheap";
}

namespace OHOS {
namespace HiviewDFX {
ResourceOverlimitMgr& ResourceOverlimitMgr::GetInstance()
{
    static ResourceOverlimitMgr instance;
    return instance;
}

std::string ResourceOverlimitMgr::GetRawheapDir() const
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
    std::string path = context->GetCacheDir() + RAWHEAP;
    if (!FileUtil::IsFileExists(path)) {
        HILOG_ERROR(LOG_CORE, "rawheap file dosen't exist");
        return "";
    }

    return path;
}

int ResourceOverlimitMgr::UpdateProperty(const std::string& property, const std::string& value) const
{
    if (runningId_.empty()) {
        HILOG_ERROR(LOG_CORE, "empty runningId");
        return ErrorCode::ERROR_UNKNOWN;
    }
    std::string newValue = runningId_ + "," + value;
    std::string dir = GetRawheapDir();
    if (dir.empty()) {
        HILOG_ERROR(LOG_CORE, "failed to GetRawheapDir");
        return ErrorCode::ERROR_UNKNOWN;
    }
    bool rtn = FileUtil::SetDirXattr(dir, property, newValue);
    if (!rtn) {
        HILOG_ERROR(LOG_CORE, "failed to SetDirXattr, dir: %{public}s, property: %{public}s, newValue: %{public}s,"
                    " err: %{public}s, errno: %{public}d",
                    dir.c_str(), property.c_str(), newValue.c_str(), strerror(errno), errno);
        return ErrorCode::ERROR_UNKNOWN;
    }
    HILOG_INFO(LOG_CORE, "succeed to UpdateProperty, property: %{public}s, newValue: %{public}s",
               property.c_str(), newValue.c_str());
    return 0;
}

void ResourceOverlimitMgr::SetRunningId(const std::string& id)
{
    runningId_ = id;
}

bool ResourceOverlimitMgr::IsValid(const std::string &key, const std::string &value) const
{
    const std::map<std::string, std::set<std::string>> whiteList = {
        {"js_heap_logtype", {"event", "event_rawheap"}}
    };

    auto it = whiteList.find(key);
    if (it == whiteList.end()) {
        HILOG_ERROR(LOG_CORE, "invalid config key: %{public}s", key.c_str());
        return false;
    }
    if (it->second.find(value) == it->second.end()) {
        HILOG_ERROR(LOG_CORE, "invalid value for key %{public}s: %{public}s", key.c_str(), value.c_str());
        return false;
    }

    return true;
}

int ResourceOverlimitMgr::SetEventConfig(const std::map<std::string, std::string>& configMap) const
{
    int rtn = ErrorCode::ERROR_UNKNOWN;
    for (const auto& [key, value] : configMap) {
        if (!IsValid(key, value)) {
            HILOG_WARN(LOG_CORE, "failed SetEventConfig, invalid key %{public}s, value: %{public}s",
                       key.c_str(), value.c_str());
            continue;
        }
        if (key == "js_heap_logtype") {
            rtn = UpdateProperty("user.event_config." + key, value); // key must start with 'user.'
        }
    }

    return rtn;
}
}  // HiviewDFX
}  // OHOS
