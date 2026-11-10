/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <memory>

#include "app_state_callback.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr int NUM_OF_CALLBACK_TYPES = 2;
}

void FuzzAppStateCallback(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    OHOS::AbilityRuntime::AbilityLifecycleCallbackArgs args;
    args.type_ = static_cast<OHOS::AbilityRuntime::AbilityLifecycleCallbackType>(
        data[0] % NUM_OF_CALLBACK_TYPES);
    auto callback = std::make_shared<OHOS::HiviewDFX::HiAppEvent::AppStateCallback>();
    callback->OnAbilityCreate(args);
    callback->OnAbilityForeground(args);
    callback->OnAbilityBackground(args);
    callback->OnAbilityDestroy(args);
    callback->OnAbilityContinue(args);
}
