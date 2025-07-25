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

#ifndef HIAPPEVENT_ANI_HELPER_H
#define HIAPPEVENT_ANI_HELPER_H

#include <unordered_map>
#include <ani.h>
#include "base_type.h"
#include "hiappevent_ani_util.h"
#include "hiappevent_ani_error_code.h"
#include "hiappevent_ani_parameter_name.h"
#include "hiappevent_write.h"

namespace OHOS {
namespace HiviewDFX {
constexpr size_t MAX_LENGTH_OF_PARAM_NAME = 32;

class HiAppEventAniHelper {
public:
    bool AddProcessor(ani_env *env, ani_object processor, int64_t &out);
    bool GetPropertyDomain(ani_object info, ani_env *env, std::string &domain);
    bool GetPropertyName(ani_object info, ani_env *env, std::string &name);
    bool GeteventTypeValue(ani_object info, ani_env *env, int32_t &enumValue);
    ani_status ParseEnumGetValueInt32(ani_env *env, ani_enum_item enumItem, int32_t &value);
    virtual bool AddArrayParamToAppEventPack(ani_env *env, const std::string &key, ani_ref arrayRef,
        std::shared_ptr<AppEventPack> &appEventPack);
    bool AddParamToAppEventPack(ani_env *env, const std::string &key, ani_ref element,
        std::shared_ptr<AppEventPack> &appEventPack);
    virtual bool ParseParamsInAppEventPack(ani_env *env, ani_ref params, std::shared_ptr<AppEventPack> &appEventPack);
    int32_t GetResult();
    bool AddAppEventPackParam(ani_env *env,
        std::pair<std::string, ani_ref> recordTemp, std::shared_ptr<AppEventPack> &appEventPack);
    bool Configure(ani_env *env, ani_object configObj);
    bool SetUserId(ani_env *env, ani_string name, ani_string value);
    bool GetUserId(ani_env *env, ani_string name, ani_string &userId);
    bool SetUserProperty(ani_env *env, ani_string name, ani_string value);
    bool GetUserProperty(ani_env *env, ani_string name, ani_string &userProperty);
    bool RemoveProcessor(ani_env *env, ani_long id);
    ani_object AddWatcher(ani_env *env, ani_object watcher, uint64_t beginTime);
    void RemoveWatcher(ani_env *env, ani_object watcher, uint64_t beginTime);

protected:
    int32_t result_ = 0;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_ANI_HELPER_H
