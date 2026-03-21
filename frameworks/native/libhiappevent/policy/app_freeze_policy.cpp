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
#include "app_freeze_policy.h"

#include "event_policy_utils.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AppFreezePolicy"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int INVALID_PARAM = -1;
}

int AppFreezePolicy::SetEventPolicy(const std::map<std::string, std::string>& configMap)
{
    auto conf = configMap;
    return EventPolicyUtils::GetInstance().ConfigPageSwitch("appFrpageSwitchLogEnable", conf);
}

int AppFreezePolicy::SetEventPolicy(const std::map<uint8_t, uint32_t>& configMap)
{
    return INVALID_PARAM;
}
}  // HiviewDFX
}  // OHOS
