/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ndk_app_event_watcher_service.h"

#include "ndk_app_event_watcher_proxy.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"

using namespace OHOS::HiviewDFX;

#ifndef CHECK_WATCHER_PTR
#define CHECK_WATCHER_PTR(ptr)                  \
    if ((ptr) == nullptr) {                     \
        return;                                 \
    }
#endif

#ifndef CHECK_WATCHER_PTR_AND_RETURN
#define CHECK_WATCHER_PTR_AND_RETURN(ptr, ret)  \
    if ((ptr) == nullptr) {                     \
        return ret;                             \
    }
#endif

struct HiAppEvent_Watcher* CreateWatcher(const char *name)
{
    CHECK_WATCHER_PTR_AND_RETURN(name, nullptr)
    auto ndkWatcherPtr = new (std::nothrow) NdkAppEventWatcherProxy(name);
    return reinterpret_cast<HiAppEvent_Watcher *>(ndkWatcherPtr);
}

int SetAppEventFilter(struct HiAppEvent_Watcher* watcher, const char* domain, uint8_t eventTypes,
                      const char* const* names, int namesLen)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER)
    auto ndkWatcher = reinterpret_cast<NdkAppEventWatcherProxy *>(watcher);
    return ndkWatcher->SetAppEventFilter(domain, eventTypes, names, namesLen);
}

int SetTriggerCondition(struct HiAppEvent_Watcher* watcher, int row, int size, int timeOut)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER);
    auto ndkWatcher = reinterpret_cast<NdkAppEventWatcherProxy *>(watcher);
    return ndkWatcher->SetTriggerCondition(row, size, timeOut);
}

int SetWatcherOnTrigger(struct HiAppEvent_Watcher* watcher, OH_HiAppEvent_OnTrigger onTrigger)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER)
    auto ndkWatcher = reinterpret_cast<NdkAppEventWatcherProxy *>(watcher);
    return ndkWatcher->SetWatcherOnTrigger(onTrigger);
}

int SetWatcherOnReceiver(struct HiAppEvent_Watcher* watcher, OH_HiAppEvent_OnReceive onReceiver)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER)
    auto ndkWatcher = reinterpret_cast<NdkAppEventWatcherProxy *>(watcher);
    return ndkWatcher->SetWatcherOnReceiver(onReceiver);
}

int AddWatcher(struct HiAppEvent_Watcher* watcher)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER)
    auto ndkWatcher = reinterpret_cast<NdkAppEventWatcherProxy *>(watcher);
    return ndkWatcher->AddWatcher();
}

int TakeWatcherData(struct HiAppEvent_Watcher* watcher, uint32_t size, OH_HiAppEvent_OnTake onTake)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER)
    return reinterpret_cast<NdkAppEventWatcherProxy *>(watcher)->TakeWatcherData(size, onTake);
}

int RemoveWatcher(struct HiAppEvent_Watcher *watcher)
{
    CHECK_WATCHER_PTR_AND_RETURN(watcher, ErrorCode::ERROR_INVALID_WATCHER)
    return reinterpret_cast<NdkAppEventWatcherProxy *>(watcher)->RemoveWatcher();
}

void DestroyWatcher(struct HiAppEvent_Watcher* watcher)
{
    CHECK_WATCHER_PTR(watcher)
    auto *watcherProxy = reinterpret_cast<NdkAppEventWatcherProxy *>(watcher);
    watcherProxy->RemoveWatcher();
    delete watcherProxy;
}
