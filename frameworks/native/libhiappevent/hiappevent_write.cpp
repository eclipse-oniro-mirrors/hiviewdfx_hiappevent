/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "hiappevent_write.h"

#include <mutex>
#include <string>

#include "app_event_observer_mgr.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_read.h"
#include "hilog/log.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_Write" };
std::mutex g_mutex;

std::string GetStorageDirPath()
{
    return HiAppEventConfig::GetInstance().GetStorageDir();
}

uint64_t GetMaxStorageSize()
{
    return HiAppEventConfig::GetInstance().GetMaxStorageSize();
}

std::string GetStorageFileName()
{
    return "app_event_" + TimeUtil::GetDate() + ".log";
}

void CheckStorageSpace(const std::string& dir)
{
    auto maxSize = GetMaxStorageSize();
    if (!HiAppEventClean::IsStorageSpaceFull(dir, maxSize)) {
        return;
    }
    HiLog::Info(LABEL, "hiappevent dir space is full, start to clean");
    HiAppEventClean::ReleaseSomeStorageSpace(dir, maxSize);
}

bool WriteEventToFile(const std::string& filePath, const std::string& event)
{
    LogAssistant::Instance().RealTimeAppLogUpdate(event);
    return FileUtil::SaveStringToFile(filePath, event);
}
}

void WriteEvent(std::shared_ptr<AppEventPack> appEventPack)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HiLog::Warn(LABEL, "the HiAppEvent function is disabled.");
        return;
    }
    if (appEventPack == nullptr) {
        HiLog::Error(LABEL, "appEventPack is null.");
        return;
    }
    std::string dirPath = GetStorageDirPath();
    if (dirPath.empty()) {
        HiLog::Error(LABEL, "dirPath is null, stop writing the event.");
        return;
    }
    std::string event = appEventPack->GetEventStr();
    HiLog::Debug(LABEL, "WriteEvent eventInfo=%{public}s.", event.c_str());
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        if (!FileUtil::IsFileExists(dirPath) && !FileUtil::ForceCreateDirectory(dirPath)) {
            HiLog::Error(LABEL, "failed to create hiappevent dir, errno=%{public}d.", errno);
            return;
        }
        CheckStorageSpace(dirPath);
        std::string filePath = FileUtil::GetFilePathByDir(dirPath, GetStorageFileName());
        if (WriteEventToFile(filePath, event)) {
            AppEventObserverMgr::GetInstance().HandleEvent(appEventPack);
            return;
        }
        HiLog::Error(LABEL, "failed to write event to log file, errno=%{public}d.", errno);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
