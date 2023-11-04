/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "app_event_handler.h"

#include "application_context.h"
#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_AppEventHandler" };
}
AppEventHandler::AppEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : AppExecFwk::EventHandler(runner)
{
    HiLog::Info(LABEL, "AppEventHandler instance created");
    RegisterAppStateCallback();
}

void AppEventHandler::RegisterAppStateCallback()
{
    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (context == nullptr) {
        HiLog::Warn(LABEL, "app context is null");
        return;
    }
    appStateCallback_ = std::make_shared<AppStateCallback>(std::shared_ptr<AppEventHandler>(this));
    context->RegisterAbilityLifecycleCallback(appStateCallback_);
    HiLog::Info(LABEL, "succ to register application state callback");
}

AppEventHandler::~AppEventHandler()
{
    HiLog::Info(LABEL, "AppEventHandler instance destroyed");
    UnregisterAppStateCallback();
}

void AppEventHandler::UnregisterAppStateCallback()
{
    if (appStateCallback_ == nullptr) {
        return;
    }

    auto context = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    if (context == nullptr) {
        HiLog::Warn(LABEL, "app context is null");
        return;
    }
    context->UnregisterAbilityLifecycleCallback(appStateCallback_);
    HiLog::Info(LABEL, "succ to unregister application state callback");
}

void AppEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event->GetInnerEventId() == AppEventType::WATCHER_TIMEOUT) {
        AppEventObserverMgr::GetInstance().HandleTimeout();
    } else if (event->GetInnerEventId() == AppEventType::APP_STATE_BACKGROUND) {
        AppEventObserverMgr::GetInstance().HandleBackground();
    } else {
        HiLog::Warn(LABEL, "invalid event id=%{public}u", event->GetInnerEventId());
    }
}
} // namespace HiviewDFX
} // namespace OHOS
