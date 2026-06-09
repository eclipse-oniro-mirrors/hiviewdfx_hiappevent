/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "hiappevent_impl.h"

#include <cinttypes>
#include <mutex>
#include <string>

#include "appevent_watcher_impl.h"
#include "cj_ffi/cj_common_ffi.h"
#include "error.h"
#include "hiappevent_facade.h"
#include "log.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiAppEvent;

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
std::mutex g_mutex;
int HiAppEventImpl::Configure(bool disable, const std::string& maxStorage)
{
    std::string disableStr = disable == true ? "true" : "false";
    bool disableRes = AppEventConfigFacade::SetConfigurationItem("disable", disableStr);
    if (!disableRes) {
        LOGE("HiAppEvent failed to configure disable HiAppEvent");
        return ERR_INVALID_MAX_STORAGE;
    }
    bool maxStorageRes = AppEventConfigFacade::SetConfigurationItem("max_storage", maxStorage);
    if (!maxStorageRes) {
        LOGE("HiAppEvent failed to configure maxStorage HiAppEvent");
        return ERR_INVALID_MAX_STORAGE;
    }
    return SUCCESS_CODE;
}

std::string GetStorageDirPath()
{
    return AppEventConfigFacade::GetStorageDir();
}

uint64_t GetMaxStorageSize()
{
    return AppEventConfigFacade::GetMaxStorageSize();
}

std::string GetStorageFileName()
{
    return "app_event_" + AppEventUtilityFacade::GetDate() + ".log";
}

void CheckStorageSpace(const std::string& dir)
{
    auto maxSize = GetMaxStorageSize();
    if (!AppEventStoreFacade::IsStorageSpaceFull(dir, maxSize)) {
        return;
    }
    LOGI("hiappevent dir space is full, start to clean");
    AppEventStoreFacade::ReleaseSomeStorageSpace(dir, maxSize);
}

bool WriteEventToFile(const std::string& filePath, const std::string& event)
{
    return AppEventUtilityFacade::SaveStringToFile(filePath, event);
}

void HiWriteEvent(std::shared_ptr<AppEventPack> appEventPack)
{
    if (AppEventConfigFacade::GetDisable()) {
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
        if (!AppEventUtilityFacade::IsFileExists(dirPath) && !AppEventUtilityFacade::ForceCreateDirectory(dirPath)) {
            LOGE("failed to create hiappevent dir, errno=%{public}d.", errno);
            return;
        }
        CheckStorageSpace(dirPath);
        std::string filePath = AppEventUtilityFacade::GetFilePathByDir(dirPath, GetStorageFileName());
        if (WriteEventToFile(filePath, event)) {
            std::vector<std::shared_ptr<AppEventPack>> events;
            events.emplace_back(appEventPack);
            AppEventObserverFacade::HandleEvents(events);
            return;
        }
        LOGE("failed to write event to log file, errno=%{public}d.", errno);
    }
}

int HiAppEventImpl::Write(std::shared_ptr<HiviewDFX::AppEventPack> appEventPack)
{
    if (auto ret = AppEventVerifyFacade::VerifyTheAppEvent(appEventPack); ret != 0) {
        LOGE("HiAppEvent failed to write HiAppEvent %{public}d", ret);
        return ret;
    }
    HiWriteEvent(appEventPack);
    return SUCCESS_CODE;
}

int64_t HiAppEventImpl::AddProcessor(const ReportConfig& conf)
{
    int64_t processorId = AppEventObserverFacade::AddProcessor(conf.name, conf);
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
    if (AppEventObserverFacade::RemoveObserver(processorId) != 0) {
        LOGE("failed to remove processor id=%{public}" PRIi64"", processorId);
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

int HiAppEventImpl::SetUserId(const std::string& name, const std::string& value)
{
    if (value.empty()) {
        if (AppEventUserInfoFacade::RemoveUserId(name) != 0) {
            LOGE("failed to remove userId");
            return ERR_CODE_PARAM_INVALID;
        }
        return SUCCESS_CODE;
    }
    if (!AppEventVerifyFacade::VerifyIsValidUserIdValue(std::string(value))) {
        return ERR_CODE_PARAM_INVALID;
    }
    if (AppEventUserInfoFacade::SetUserId(name, value) != 0) {
        LOGE("failed to set userId");
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

std::tuple<int, std::string> HiAppEventImpl::GetUserId(const std::string& name)
{
    std::string strUserId;
    if (AppEventUserInfoFacade::GetUserId(name, strUserId) != 0) {
        LOGE("failed to get userId");
        return {ERR_CODE_PARAM_INVALID, nullptr};
    }
    return {SUCCESS_CODE, strUserId};
}

int HiAppEventImpl::SetUserProperty(const std::string& name, const std::string& value)
{
    if (value.empty()) {
        if (AppEventUserInfoFacade::RemoveUserProperty(name) != 0) {
            LOGE("failed to set user propertyd");
            return ERR_CODE_PARAM_INVALID;
        }
        return SUCCESS_CODE;
    }
    if (AppEventUserInfoFacade::SetUserProperty(name, value) != 0) {
        LOGE("failed to set user property");
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

std::tuple<int, std::string> HiAppEventImpl::GetUserProperty(const std::string& name)
{
    std::string strUserProperty;
    if (AppEventUserInfoFacade::GetUserProperty(name, strUserProperty) != 0) {
        LOGE("failed to get user property");
        return {ERR_CODE_PARAM_INVALID, nullptr};
    }
    return {SUCCESS_CODE, strUserProperty};
}

void HiAppEventImpl::ClearData()
{
    std::string dir = AppEventConfigFacade::GetStorageDir();
    AppEventStoreFacade::ClearData(dir);
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
    int64_t observerSeq = AppEventObserverFacade::AddWatcher(watcherPtr);
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
    AppEventObserverFacade::RemoveObserver(name);
}

int HiAppEventImpl::Load(const std::string& moduleName)
{
    return AppEventObserverFacade::Load(moduleName);
}
} // HiAppEvent
} // CJSystemapi
} // OHOS