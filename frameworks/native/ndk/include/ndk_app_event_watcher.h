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

#ifndef HIAPPEVENT_NDK_APPEVENT_WATCHER_H
#define HIAPPEVENT_NDK_APPEVENT_WATCHER_H

#include <memory>

#include "app_event_watcher.h"
#include "hiappevent/hiappevent.h"

namespace OHOS {
namespace HiviewDFX {
class NdkAppEventWatcher : public AppEventWatcher {
public:
    explicit NdkAppEventWatcher(const std::string& name);

    void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) override;
    bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event) override;
    void SetTriggerCondition(int row, int size, int timeOut);
    int AddAppEventFilter(const char* domain, uint8_t eventTypes, const char* const *names, int namesLen);
    void SetOnTrigger(OH_HiAppEvent_OnTrigger onTrigger);
    void SetOnOnReceive(OH_HiAppEvent_OnReceive onReceive);

protected:
    void OnTrigger(const HiAppEvent::TriggerCondition& triggerCond) override;

private:
    OH_HiAppEvent_OnTrigger onTrigger_{nullptr};
    OH_HiAppEvent_OnReceive onReceive_{nullptr};
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_NDK_APPEVENT_WATCHER_H
