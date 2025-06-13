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

#ifndef HIAPP_EVENT_ANI_H
#define HIAPP_EVENT_ANI_H

#include <unordered_map>
#include <ani.h>
#include "base_type.h"

namespace OHOS {
namespace HiviewDFX {

class HiAppEventAni {
public:
    static ani_object Write(ani_env *env, ani_object info);
    static ani_double AddProcessor(ani_env *env, ani_object processor);
};

} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPP_EVENT_ANI_H
