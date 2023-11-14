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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_STATE_CALLBACK_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_STATE_CALLBACK_H

#include <memory>
#include <string>

#include "application_context.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class AppStateCallback : public OHOS::AbilityRuntime::AbilityLifecycleCallback {
public:
    void OnAbilityCreate(const std::shared_ptr<NativeReference>& ability) override {}

    void OnWindowStageCreate(const std::shared_ptr<NativeReference>& ability,
        const std::shared_ptr<NativeReference>& windowStage) override {}

    void OnWindowStageDestroy(const std::shared_ptr<NativeReference>& ability,
        const std::shared_ptr<NativeReference>& windowStage) override {}

    void OnWindowStageActive(const std::shared_ptr<NativeReference>& ability,
        const std::shared_ptr<NativeReference>& windowStage) override {}

    void OnWindowStageInactive(const std::shared_ptr<NativeReference>& ability,
        const std::shared_ptr<NativeReference>& windowStage) override {}

    void OnAbilityDestroy(const std::shared_ptr<NativeReference>& ability) override {}

    void OnAbilityForeground(const std::shared_ptr<NativeReference>& ability) override {}

    void OnAbilityBackground(const std::shared_ptr<NativeReference>& ability) override;

    void OnAbilityContinue(const std::shared_ptr<NativeReference>& ability) override {}
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_STATE_CALLBACK_H
