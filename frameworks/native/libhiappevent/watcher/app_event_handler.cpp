/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "app_event_watcher_mgr.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, 0xD002D07, "HiAppEvent_Handler" };
}
AppEventHandler::AppEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : AppExecFwk::EventHandler(runner)
{
    HiLog::Info(LABEL, "AppEventHandler instance created");
}

AppEventHandler::~AppEventHandler()
{
    HiLog::Info(LABEL, "AppEventHandler instance destroyed");
}

void AppEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event->GetInnerEventId() == AppEventType::WATCHER_TIMEOUT) {
        AppEventWatcherMgr::GetInstance()->HandleTimeOut();
    }
}
} // namespace HiviewDFX
} // namespace OHOS
