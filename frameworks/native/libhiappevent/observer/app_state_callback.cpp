/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "app_state_callback.h"

#include "app_event_observer_mgr.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
void AppStateCallback::OnAbilityBackground(const std::shared_ptr<NativeReference>& ability)
{
    AppEventObserverMgr::GetInstance().HandleBackground();
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
