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

#include "hiappevent_param_builder.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_param_UTIL"

using namespace OHOS::HiviewDFX;

static void AppendArray(ani_env *env, ani_ref valueRef, AniArgsType type, ParamArray &paramArray)
{
    if (type == AniArgsType::ANI_STRING) {
        paramArray.stringArray.emplace_back(HiAppEventAniUtil::ParseStringValue(env, valueRef));
    } else {
        HILOG_ERROR(LOG_CORE, "Unexpected type");
    }
}

static bool AddArrayParam(AniArgsType type, std::string key, ParamArray &paramArray,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    if (type == AniArgsType::ANI_STRING) {
        appEventPack->AddParam(key, paramArray.stringArray);
    } else {
        HILOG_ERROR(LOG_CORE, "array param value type is invalid");
        return false;
    }
    return true;
}

bool HiAppEventParamBuilder::AddArrayParamToAppEventPack(ani_env *env, const std::string &key, ani_ref arrayRef,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    ani_size size = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array_ref>(arrayRef), &size)) {
        HILOG_ERROR(LOG_CORE, "get array length failed");
        return false;
    }
    if (size == 0) {
        HILOG_ERROR(LOG_CORE, "The array is empty");
        return false;
    }
    ParamArray paramArray;
    AniArgsType arrayType = HiAppEventAniUtil::GetArrayType(env, static_cast<ani_array_ref>(arrayRef));
    if (arrayType != AniArgsType::ANI_STRING) {
        return false;
    }
    for (ani_size i = 0; i < size; i++) {
        ani_ref valueRef {};
        if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(arrayRef), i, &valueRef)) {
            HILOG_ERROR(LOG_CORE, "get %{public}zu element of array failed", i);
            return false;
        }
        if (arrayType != HiAppEventAniUtil::GetArgType(env, static_cast<ani_object>(valueRef))) {
            HILOG_ERROR(LOG_CORE, "the elements in array is not same type");
            return false;
        }
        AppendArray(env, valueRef, arrayType, paramArray);
    }
    return AddArrayParam(arrayType, key, paramArray, appEventPack);
}

bool HiAppEventParamBuilder::ParseParamsInAppEventPack(ani_env *env, ani_ref params,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    if (HiAppEventAniUtil::IsRefUndefined(env, params)) {
        HILOG_ERROR(LOG_CORE, "AppEventInfo params undefined");
        return false;
    }
    std::map<std::string, ani_ref> appEventParams;
    HiAppEventAniUtil::ParseRecord(env, static_cast<ani_object>(params), appEventParams);
    for (const auto &param : appEventParams) {
        if (param.first.length() > MAX_LENGTH_OF_PARAM_NAME) {
            result_ = ERROR_INVALID_PARAM_NAME;
            HILOG_ERROR(LOG_CORE, "the length=%{public}zu of the param key is invalid", param.first.length());
            break;
        }
        if (!AddAppEventPackParam(env, param, appEventPack)) {
            return false;
        }
    }
    return true;
}
