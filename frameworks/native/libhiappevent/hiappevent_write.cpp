/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_event_store.h"
#include "app_event_observer_mgr.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_ffrt.h"
#include "hiappevent_read.h"
#include "hilog/log.h"
#include "time_util.h"
#include "xcollie/watchdog.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Write"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int DB_FAILED = -1;
const std::string  MAIN_THREAD_JANK = "MAIN_THREAD_JANK";
std::mutex g_mutex;
constexpr int SUBMIT_FAILED_NUM = 50;
static int g_submitFailedCnt = 0;
static std::mutex g_submitFailedCntMutex;

std::string GetStorageDirPath()
{
    return HiAppEventConfig::GetInstance().GetStorageDir();
}

std::string GetStorageFileName()
{
    return "app_event_" + TimeUtil::GetDate() + ".log";
}

bool WriteEventToFile(const std::string& filePath, const std::string& event)
{
    LogAssistant::Instance().RealTimeAppLogUpdate(event);
    return FileUtil::SaveStringToFile(filePath, event);
}
}

void SubmitWritingTask(std::shared_ptr<AppEventPack> appEventPack, const std::string& taskName)
{
    auto ret = HiAppEvent::Submit([appEventPack]() {
        WriteEvent(appEventPack);
        }, {}, {}, ffrt::task_attr().name(taskName.c_str()));
    if (ret != ffrt_success) {
        std::lock_guard<std::mutex> lockGuard(g_submitFailedCntMutex);
        ++g_submitFailedCnt;
        if (g_submitFailedCnt >= SUBMIT_FAILED_NUM) {
            HILOG_ERROR(LOG_CORE, "failed to submit %{public}s %{public}s, ret=%{public}d",
                taskName.c_str(), appEventPack->GetParamApiStr().c_str(), ret);
            g_submitFailedCnt = 0;
        }
    }
}

void WriteEvent(std::shared_ptr<AppEventPack> appEventPack)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HILOG_WARN(LOG_CORE, "the HiAppEvent function is disabled.");
        return;
    }
    if (appEventPack == nullptr) {
        HILOG_ERROR(LOG_CORE, "appEventPack is null.");
        return;
    }
    std::string dirPath = GetStorageDirPath();
    if (dirPath.empty()) {
        HILOG_ERROR(LOG_CORE, "dirPath is null, stop writing the event.");
        return;
    }
    std::string event = appEventPack->GetEventStr();
    HILOG_DEBUG(LOG_CORE, "WriteEvent domain=%{public}s, name=%{public}s.",
        appEventPack->GetDomain().c_str(), appEventPack->GetName().c_str());
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        if (!FileUtil::IsFileExists(dirPath) && !FileUtil::ForceCreateDirectory(dirPath)) {
            HILOG_ERROR(LOG_CORE, "failed to create hiappevent dir, errno=%{public}d.", errno);
            return;
        }
        HiAppEventClean::CheckStorageSpace();
        std::string filePath = FileUtil::GetFilePathByDir(dirPath, GetStorageFileName());
        if (!WriteEventToFile(filePath, event)) {
            HILOG_ERROR(LOG_CORE, "failed to write event to log file, errno=%{public}d.", errno);
            return;
        }
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(appEventPack);
    AppEventObserverMgr::GetInstance().HandleEvents(events);
}

int SetEventParam(std::shared_ptr<AppEventPack> appEventPack)
{
    if (appEventPack == nullptr) {
        HILOG_ERROR(LOG_CORE, "appEventPack is null.");
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    int res = AppEventStore::GetInstance().InsertCustomEventParams(appEventPack);
    if (res == DB_FAILED) {
        HILOG_ERROR(LOG_CORE, "failed to insert event param, domain=%{public}s.", appEventPack->GetDomain().c_str());
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    return res;
}

int SetEventConfig(const std::string& name, const std::map<std::string, std::string> &configMap)
{
    if (name != MAIN_THREAD_JANK) {
        HILOG_ERROR(LOG_CORE, "Failed to set event config, name is invalid. name=%{public}s", name.c_str());
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return Watchdog::GetInstance().SetEventConfig(configMap);
}
} // namespace HiviewDFX
} // namespace OHOS
