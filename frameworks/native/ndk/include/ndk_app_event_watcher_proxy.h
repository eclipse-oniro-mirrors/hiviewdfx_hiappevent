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

#ifndef HIAPPEVENT_NDK_APPEVENT_WATCHER_PROXY_H
#define HIAPPEVENT_NDK_APPEVENT_WATCHER_PROXY_H

#include <memory>

#include "ndk_app_event_watcher.h"

namespace OHOS {
namespace HiviewDFX {
class NdkAppEventWatcherProxy {
public:
    explicit NdkAppEventWatcherProxy(const std::string& name);
    NdkAppEventWatcherProxy(const NdkAppEventWatcherProxy&) = delete;
    NdkAppEventWatcherProxy(const NdkAppEventWatcherProxy&&) = delete;
    NdkAppEventWatcherProxy& operator= (const NdkAppEventWatcherProxy&) = delete;
    NdkAppEventWatcherProxy& operator= (const NdkAppEventWatcherProxy&&) = delete;
    int SetAppEventFilter(const char* domain, uint8_t eventTypes, const char* const *names, int namesLen);
    int SetTriggerCondition(int row, int size, int timeOut);
    int SetWatcherOnTrigger(OH_HiAppEvent_OnTrigger onTrigger);
    int SetWatcherOnReceiver(OH_HiAppEvent_OnReceive onReceiver);

    int AddWatcher();
    int TakeWatcherData(uint32_t size, OH_HiAppEvent_OnTake onTake);
    int RemoveWatcher();
private:
    std::shared_ptr<NdkAppEventWatcher> watcher_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_NDK_APPEVENT_WATCHER_PROXY_H
