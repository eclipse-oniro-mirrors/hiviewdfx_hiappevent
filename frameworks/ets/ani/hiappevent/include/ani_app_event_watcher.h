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

#ifndef ANI_APP_EVENT_WATCHER_H
#define ANI_APP_EVENT_WATCHER_H

#include <ani.h>

#include "app_event_watcher.h"
#include "event_handler.h"
#include "hiappevent_ani_error_code.h"

namespace OHOS {
namespace HiviewDFX {
struct OnTriggerContext {
    ~OnTriggerContext();
    ani_vm* vm = nullptr;
    ani_ref onTrigger {};
    ani_ref holder {};
};

struct OnReceiveContext {
    ~OnReceiveContext();
    ani_vm* vm = nullptr;
    ani_ref onReceive {};
};

class AniAppEventWatcher : public AppEventWatcher {
public:
    AniAppEventWatcher(
        const std::string& name,
        const std::vector<AppEventFilter>& filters,
        TriggerCondition cond);
    ani_status AniSendEvent(const std::function<void()> cb, const std::string& name);
    void InitTrigger(ani_env *env, ani_ref trigger);
    void InitHolder(ani_env *env, ani_ref holder);
    void InitReceiver(ani_env *env, ani_ref receiver);
    bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event) override;
    void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) override;

protected:
    void OnTrigger(const TriggerCondition& triggerCond) override;

private:
    std::shared_ptr<OnTriggerContext> triggerContext_ = nullptr;
    std::shared_ptr<OnReceiveContext> receiveContext_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_APP_EVENT_WATCHER_H
