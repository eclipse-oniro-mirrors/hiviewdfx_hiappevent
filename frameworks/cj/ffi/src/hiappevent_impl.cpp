/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <mutex>
#include <string>

#include "appevent_watcher_impl.h"
#include "app_event_observer_mgr.h"
#include "cj_ffi/cj_common_ffi.h"
#include "error.h"
#include "file_util.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_impl.h"
#include "hiappevent_read.h"
#include "hiappevent_userinfo.h"
#include "hiappevent_verify.h"
#include "hiappevent_write.h"
#include "log.h"
#include "module_loader.h"
#include "time_util.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiAppEvent;

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
std::mutex g_mutex;
int HiAppEventImpl::Configure(bool disable, const std::string& maxStorage)
{
    std::string disableStr = disable == true ? "true" : "false";
    bool disableRes = HiAppEventConfig::GetInstance().SetConfigurationItem("disable", disableStr);
    if (!disableRes) {
        LOGE("HiAppEvent failed to configure disable HiAppEvent");
        return ERR_INVALID_MAX_STORAGE;
    }
    bool maxStorageRes = HiAppEventConfig::GetInstance().SetConfigurationItem("max_storage", maxStorage);
    if (!maxStorageRes) {
        LOGE("HiAppEvent failed to configure maxStorage HiAppEvent");
        return ERR_INVALID_MAX_STORAGE;
    }
    return SUCCESS_CODE;
}

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
    LOGI("hiappevent dir space is full, start to clean");
    HiAppEventClean::ReleaseSomeStorageSpace(dir, maxSize);
}

bool WriteEventToFile(const std::string& filePath, const std::string& event)
{
    LogAssistant::Instance().RealTimeAppLogUpdate(event);
    return FileUtil::SaveStringToFile(filePath, event);
}

void HiWriteEvent(std::shared_ptr<AppEventPack> appEventPack)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        LOGE("the HiAppEvent function is disabled.");
        return;
    }
    if (appEventPack == nullptr) {
        LOGE("appEventPack is null.");
        return;
    }
    std::string dirPath = GetStorageDirPath();
    if (dirPath.empty()) {
        LOGE("dirPath is null, stop writing the event.");
        return;
    }
    std::string event = appEventPack->GetEventStr();
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        if (!FileUtil::IsFileExists(dirPath) && !FileUtil::ForceCreateDirectory(dirPath)) {
            LOGE("failed to create hiappevent dir, errno=%{public}d.", errno);
            return;
        }
        CheckStorageSpace(dirPath);
        std::string filePath = FileUtil::GetFilePathByDir(dirPath, GetStorageFileName());
        if (WriteEventToFile(filePath, event)) {
            std::vector<std::shared_ptr<AppEventPack>> events;
            events.emplace_back(appEventPack);
            AppEventObserverMgr::GetInstance().HandleEvents(events);
            return;
        }
        LOGE("failed to write event to log file, errno=%{public}d.", errno);
    }
}

int HiAppEventImpl::Write(std::shared_ptr<HiviewDFX::AppEventPack> appEventPack)
{
    if (auto ret = VerifyAppEvent(appEventPack); ret != 0) {
        LOGE("HiAppEvent failed to write HiAppEvent %{public}d", ret);
        return ret;
    }
    HiWriteEvent(appEventPack);
    return SUCCESS_CODE;
}

int64_t HiAppEventImpl::AddProcessor(const ReportConfig& conf)
{
    int64_t processorId = AppEventObserverMgr::GetInstance().RegisterObserver(conf.name, conf);
    if (processorId <= 0) {
        LOGE("failed to add processor=%{public}s, register processor error", conf.name.c_str());
        return processorId;
    }
    return processorId;
}

int HiAppEventImpl::RemoveProcessor(int64_t processorId)
{
    if (processorId <= 0) {
        LOGE("failed to remove processor id=%{public}" PRIi64 "", processorId);
        return SUCCESS_CODE;
    }
    if (AppEventObserverMgr::GetInstance().UnregisterObserver(processorId) != 0) {
        LOGE("failed to remove processor id=%{public}" PRIi64"", processorId);
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

int HiAppEventImpl::SetUserId(const std::string& name, const std::string& value)
{
    if (value.empty()) {
        if (UserInfo::GetInstance().RemoveUserId(name) != 0) {
            LOGE("failed to remove userId");
            return ERR_CODE_PARAM_INVALID;
        }
        return SUCCESS_CODE;
    }
    if (UserInfo::GetInstance().SetUserId(name, value) != 0) {
        LOGE("failed to set userId");
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

std::tuple<int, std::string> HiAppEventImpl::GetUserId(const std::string& name)
{
    std::string strUserId;
    if (UserInfo::GetInstance().GetUserId(name, strUserId) != 0) {
        LOGE("failed to get userId");
        return {ERR_CODE_PARAM_INVALID, nullptr};
    }
    return {SUCCESS_CODE, strUserId};
}

int HiAppEventImpl::SetUserProperty(const std::string& name, const std::string& value)
{
    if (value.empty()) {
        if (UserInfo::GetInstance().RemoveUserProperty(name) != 0) {
            LOGE("failed to set user propertyd");
            return ERR_CODE_PARAM_INVALID;
        }
        return SUCCESS_CODE;
    }
    if (UserInfo::GetInstance().SetUserProperty(name, value) != 0) {
        LOGE("failed to set user property");
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

std::tuple<int, std::string> HiAppEventImpl::GetUserProperty(const std::string& name)
{
    std::string strUserProperty;
    if (UserInfo::GetInstance().GetUserProperty(name, strUserProperty) != 0) {
        LOGE("failed to get user property");
        return {ERR_CODE_PARAM_INVALID, nullptr};
    }
    return {SUCCESS_CODE, strUserProperty};
}

void HiAppEventImpl::ClearData()
{
    std::string dir = HiAppEventConfig::GetInstance().GetStorageDir();
    HiAppEventClean::ClearData(dir);
}

std::tuple<int, int64_t> HiAppEventImpl::addWatcher(const std::string& name,
                                                    const std::vector<AppEventFilter>& filters,
                                                    const TriggerCondition& cond,
                                                    void (*callbackOnTriggerRef)(int, int, int64_t),
                                                    void (*callbackOnReceiveRef)(char*, CArrRetAppEventGroup))
{
    auto watcherPtr = std::make_shared<AppEventWatcherImpl>(name, filters, cond);
    if (callbackOnTriggerRef != (void*)-1) {
        watcherPtr->InitTrigger(callbackOnTriggerRef);
    }
    if (callbackOnReceiveRef != (void*)-1) {
        watcherPtr->InitReceiver(callbackOnReceiveRef);
    }
    int64_t observerSeq = AppEventObserverMgr::GetInstance().RegisterObserver(watcherPtr);
    if (observerSeq <= 0) {
        LOGE("invalid observer sequence");
        return {ERR_CODE_PARAM_INVALID, -1};
    }
    auto holder = OHOS::FFI::FFIData::Create<AppEventPackageHolderImpl>(name, -1);
    if (holder == nullptr) {
        return {ERR_PARAM, -1};
    }
    watcherPtr->InitHolder(holder);
    return {SUCCESS_CODE, holder->GetID()};
}

void HiAppEventImpl::removeWatcher(const std::string& name)
{
    AppEventObserverMgr::GetInstance().UnregisterObserver(name);
}
} // HiAppEvent
} // CJSystemapi
} // OHOS