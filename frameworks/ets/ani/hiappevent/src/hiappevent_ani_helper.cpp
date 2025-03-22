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

#include <map>

#include "hiappevent_ani_helper.h"

#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "hilog/log_cpp.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr size_t MAX_LENGTH_OF_PARAM_NAME = 32;
constexpr int32_t INVALID_OUT = -1;

typedef struct ConfigProp {
    std::string key;
    int32_t (*func)(ani_env*, ani_object, ReportConfig&);
} ConfigProp;

const std::pair<const char*, AniArgsType> OBJECT_TYPE[] = {
    {CLASS_NAME_INT, AniArgsType::ANI_INT},
    {CLASS_NAME_BOOLEAN, AniArgsType::ANI_BOOLEAN},
    {CLASS_NAME_DOUBLE, AniArgsType::ANI_NUMBER},
    {CLASS_NAME_STRING, AniArgsType::ANI_STRING},
};
}

static bool AddParamToCustomConfigs(ani_env *env, ani_ref recordRef, HiAppEvent::ReportConfig &conf)
{
    if (!HiAppEventAniUtil::IsRefUndefined(env, recordRef)) {
        std::map<std::string, ani_ref> CustomConfigsRecord;
        HiAppEventAniUtil::ParseRecord(env, static_cast<ani_object>(recordRef), CustomConfigsRecord);
        for (const auto &CustomConfigsTemp : CustomConfigsRecord) {
            conf.customConfigs[CustomConfigsTemp.first] =
                HiAppEventAniUtil::ParseStringValue(env, CustomConfigsTemp.second);
        }
    }
    return true;
}

ani_boolean HiAppEventAniHelper::AddProcessor(ani_env *env, ani_object processor, int64_t &out)
{
    ReportConfig conf;
    int32_t ret = TransConfig(env, processor, conf);
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "TransConfig Failed.");
        out = INVALID_OUT;
        return false;
    }
    std::string name = conf.name;
    if (name.empty()) {
        HILOG_ERROR(LOG_CORE, "processor name can not be empty.");
        out = INVALID_OUT;
        return false;
    }
    if (AppEventObserverMgr::GetInstance().Load(name) != 0) {
        HILOG_WARN(LOG_CORE, "failed to add processor=%{public}s, name no found", name.c_str());
        out = INVALID_OUT;
        return true;
    }
    int64_t processorId = AppEventObserverMgr::GetInstance().RegisterObserver(name, conf);
    if (processorId <= 0) {
        HILOG_WARN(LOG_CORE, "failed to add processor=%{public}s, register processor error", name.c_str());
        out = INVALID_OUT;
        return false;
    }

    out = processorId;
    return true;
}

static int32_t GetConfigIdValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "configId", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.configId = HiAppEventAniUtil::ParseIntValue(env, ref);
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetRouteInfoRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "routeInfo", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.routeInfo = HiAppEventAniUtil::ParseStringValue(env, ref);
        if (!IsValidRouteInfo(out.routeInfo)) {
            return ERR_CODE_PARAM_INVALID;
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetAppIdRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "appId", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.appId = HiAppEventAniUtil::ParseStringValue(env, ref);
        if (!IsValidAppId(out.appId)) {
            return ERR_CODE_PARAM_INVALID;
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetNameRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "name", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_FORMAT;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.name = HiAppEventAniUtil::ParseStringValue(env, ref);
        if (!IsValidProcessorName(out.name)) {
            return ERR_CODE_PARAM_FORMAT;
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetPeriodReportInt(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "periodReport", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.timeout = HiAppEventAniUtil::ParseIntValue(env, ref);
        if (!IsValidPeriodReport(out.triggerCond.timeout)) {
            return ERR_CODE_PARAM_INVALID;
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetBatchReportInt(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "batchReport", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.row = HiAppEventAniUtil::ParseIntValue(env, ref);
        if (!IsValidBatchReport(out.triggerCond.row)) {
            return ERR_CODE_PARAM_INVALID;
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetUserIdsRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "userIds", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    std::vector<std::string> userIdNamesVector;
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        HiAppEventAniUtil::ParseArrayStringValue(env, ref, userIdNamesVector);
        std::unordered_set<std::string> userIdNamesSet(userIdNamesVector.begin(), userIdNamesVector.end());
        out.userIdNames = userIdNamesSet;
        for (auto userId : userIdNamesSet) {
            if (!IsValidUserIdName(userId)) {
                return ERR_CODE_PARAM_INVALID;
            }
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetUserPropertyRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "userProperties", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    std::vector<std::string> userPropertiesVector;
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        HiAppEventAniUtil::ParseArrayStringValue(env, ref, userPropertiesVector);
        std::unordered_set<std::string> userPropertiesSet(userPropertiesVector.begin(), userPropertiesVector.end());
        out.userPropertyNames = userPropertiesSet;
        for (auto userProperty : userPropertiesSet) {
            if (!IsValidUserPropName(userProperty)) {
                return ERR_CODE_PARAM_INVALID;
            }
        }
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t ParseUserPropertiesValue(ani_env *env, ani_ref Ref, std::vector<EventConfig> &arr)
{
    ani_size length = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array_ref>(Ref), &length)) {
        HILOG_ERROR(LOG_CORE, "Array_GetLength length Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    EventConfig config;
    for (ani_size i = 0; i < length; i++) {
        ani_ref value {};
        if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(Ref), i, &value)) {
            HILOG_ERROR(LOG_CORE, "Array_GetLength length Failed");
            return ERR_CODE_PARAM_INVALID;
        }
        ani_ref domainRef {};
        if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(value), "domain", &domainRef)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref domain Failed");
            return ERR_CODE_PARAM_INVALID;
        }
        config.domain = HiAppEventAniUtil::ParseStringValue(env, domainRef);
        ani_ref nameRef {};
        if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(value), "name", &nameRef)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref name Failed");
            return ERR_CODE_PARAM_INVALID;
        }
        config.name = HiAppEventAniUtil::ParseStringValue(env, nameRef);
        ani_ref isRealTimeBol;
        if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(value), "isRealTime",
            &isRealTimeBol)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref isRealTime Failed");
            return ERR_CODE_PARAM_INVALID;
        }
        if (!HiAppEventAniUtil::IsRefUndefined(env, isRealTimeBol)) {
            config.isRealTime = HiAppEventAniUtil::ParseBoolValue(env, isRealTimeBol);
            arr.push_back(config);
        }
    }
    return ERR_CODE_SUCC;
}

static int32_t GetEventConfigsRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "eventConfigs", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    std::vector<EventConfig> arr;
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        ParseUserPropertiesValue(env, ref, arr);
        out.eventConfigs = arr;
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetParamsRefValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "customConfigs", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref params failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!AddParamToCustomConfigs(env, ref, out)) {
        HILOG_ERROR(LOG_CORE, "AddParamToCustomConfigs failed");
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetDebugModeValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "debugMode", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.debugMode = HiAppEventAniUtil::ParseBoolValue(env, ref);
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetOnStartuplValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "onStartReport", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.onStartup = HiAppEventAniUtil::ParseBoolValue(env, ref);
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetOnBackgroundValue(ani_env *env, ani_object processor, ReportConfig &out)
{
    ani_ref ref {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(processor, "onBackgroundReport", &ref)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.onBackground = HiAppEventAniUtil::ParseBoolValue(env, ref);
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_SUCC;
}

const ConfigProp CONFIG_PROPS[] = {
    {
        .key = PROCESSOR_NAME,
        .func = GetNameRefValue
    },
    {
        .key = ROUTE_INFO,
        .func = GetRouteInfoRefValue
    },
    {
        .key = APP_ID,
        .func = GetAppIdRefValue
    },
    {
        .key = USER_IDS,
        .func = GetUserIdsRefValue
    },
    {
        .key = USER_PROPERTIES,
        .func = GetUserPropertyRefValue
    },
    {
        .key = DEBUG_MODE,
        .func = GetDebugModeValue
    },
    {
        .key = START_REPORT,
        .func = GetOnStartuplValue
    },
    {
        .key = BACKGROUND_REPORT,
        .func = GetOnBackgroundValue
    },
    {
        .key = PERIOD_REPORT,
        .func = GetPeriodReportInt
    },
    {
        .key = BATCH_REPORT,
        .func = GetBatchReportInt
    },
    {
        .key = EVENT_CONFIGS,
        .func = GetEventConfigsRefValue
    },
    {
        .key = CONFIG_ID,
        .func = GetConfigIdValue
    },
    {
        .key = CUSTOM_CONFIG,
        .func = GetParamsRefValue
    }
};

int32_t HiAppEventAniHelper::TransConfig(ani_env *env, ani_object processor, ReportConfig& out)
{
    for (auto prop : CONFIG_PROPS) {
        int32_t ret = (prop.func)(env, processor, out);
        if (ret == ERR_CODE_PARAM_FORMAT) {
            HILOG_ERROR(LOG_CORE, "failed to add processor, params format error");
            return ERR_CODE_PARAM_FORMAT;
        } else if (ret == ERR_CODE_PARAM_INVALID) {
            HILOG_WARN(LOG_CORE, "Parameter error. The %{public}s parameter is invalid.", prop.key.c_str());
        }
    }
    return ERR_CODE_SUCC;
}

bool HiAppEventAniHelper::GetPropertyDomain(ani_object info, ani_env *env, std::string &domain)
{
    ani_ref domainResultTemp {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "domain", &domainResultTemp)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return false;
    }

    domain = HiAppEventAniUtil::ParseStringValue(env, domainResultTemp);
    return true;
}

bool HiAppEventAniHelper::GetPropertyName(ani_object info, ani_env *env, std::string &name)
{
    ani_ref nameResultTemp {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "name", &nameResultTemp)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return false;
    }

    name = HiAppEventAniUtil::ParseStringValue(env, nameResultTemp);
    return true;
}

bool HiAppEventAniHelper::GeteventTypeValue(ani_object info, ani_env *env, int32_t &enumValue)
{
    ani_ref eventTypeRef {};
    if (ANI_OK != env->Object_GetPropertyByName_Ref(info, "eventType", &eventTypeRef)) {
        HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref Failed");
        return false;
    }

    ani_enum_item eventTypeItem = static_cast<ani_enum_item>(eventTypeRef);
    if (ANI_OK != HiAppEventAniHelper::ParseEnumGetValueInt32(env, eventTypeItem, enumValue)) {
        HILOG_ERROR(LOG_CORE, "Failed to get 'enumValue'");
        return false;
    }
    return true;
}

ani_status HiAppEventAniHelper::ParseEnumGetValueInt32(ani_env *env, ani_enum_item enumItem, int32_t &value)
{
    ani_int aniInt = 0;
    if (ANI_OK != env->EnumItem_GetValue_Int(enumItem, &aniInt)) {
        HILOG_ERROR(LOG_CORE, "EnumItem_GetValue_Int Failed");
        return ANI_ERROR;
    }
    value = static_cast<int32_t>(aniInt);
    return ANI_OK;
}

static AniArgsType GetArgType(ani_env *env, ani_object elementObj)
{
    if (HiAppEventAniUtil::IsRefUndefined(env, static_cast<ani_ref>(elementObj))) {
        return AniArgsType::ANI_UNDEFINED;
    }
    for (const auto &objType : OBJECT_TYPE) {
        ani_class cls {};
        if (ANI_OK != env->FindClass(objType.first, &cls)) {
            continue;
        }
        ani_boolean isInstance = false;
        if (ANI_OK != env->Object_InstanceOf(elementObj, cls, &isInstance)) {
            continue;
        }
        if (static_cast<bool>(isInstance)) {
            return objType.second;
        }
    }
    return AniArgsType::ANI_UNKNOWN;
}

static void AppendArray(ani_env *env, ani_ref valueRef, AniArgsType type, ParamArray &paramArray)
{
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            paramArray.boolArray.emplace_back(HiAppEventAniUtil::ParseBoolValue(env, valueRef));
            break;
        case AniArgsType::ANI_NUMBER:
            paramArray.numberArray.emplace_back(HiAppEventAniUtil::ParseNumberValue(env, valueRef));
            break;
        case AniArgsType::ANI_STRING:
            paramArray.stringArray.emplace_back(HiAppEventAniUtil::ParseStringValue(env, valueRef));
            break;
        default:
            HILOG_ERROR(LOG_CORE, "Unexpected type");
            break;
    }
}

static bool AddArrayParam(AniArgsType type, std::string key, ParamArray &paramArray,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            appEventPack->AddParam(key, paramArray.boolArray);
            break;
        case AniArgsType::ANI_NUMBER:
            appEventPack->AddParam(key, paramArray.numberArray);
            break;
        case AniArgsType::ANI_STRING:
            appEventPack->AddParam(key, paramArray.stringArray);
            break;
        default:
            HILOG_ERROR(LOG_CORE, "array param value type is invalid");
            return false;
    }
    return true;
}

bool HiAppEventAniHelper::AddArrayParamToAppEventPack(ani_env *env, const std::string &key, ani_ref arrayRef,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    ani_size size = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array_ref>(arrayRef), &size)) {
        HILOG_ERROR(LOG_CORE, "Array_GetLength Failed");
        result_ = ERR_PARAM;
        return false;
    }
    if (size == 0) {
        appEventPack->AddParam(key);
        return true;
    }
    ParamArray paramArray;
    AniArgsType arrayType = AniArgsType::ANI_UNKNOWN;
    for (ani_size i = 0; i < size; i++) {
        ani_ref valueRef {};
        if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(arrayRef), i, &valueRef)) {
            HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}zu Failed", i);
            return false;
        }
        AniArgsType objType = GetArgType(env, static_cast<ani_object>(valueRef));
        if (objType <= AniArgsType::ANI_UNKNOWN || objType >= AniArgsType::ANI_UNDEFINED) {
            continue;
        } else if (arrayType == AniArgsType::ANI_UNKNOWN) {
            arrayType = objType;
        }
        AppendArray(env, valueRef, objType, paramArray);
    }
    if (!AddArrayParam(arrayType, key, paramArray, appEventPack)) {
        result_ = ERR_PARAM;
        return false;
    }
    return true;
}

bool HiAppEventAniHelper::AddParamToAppEventPack(ani_env *env, const std::string &key, ani_ref element,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    ani_object elementObj = static_cast<ani_object>(element);
    AniArgsType type = GetArgType(env, elementObj);
    if (type <= AniArgsType::ANI_UNKNOWN || type >= AniArgsType::ANI_UNDEFINED) {
        result_ = ERR_PARAM;
        return false;
    }
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            appEventPack->AddParam(key, HiAppEventAniUtil::ParseBoolValue(env, element));
            break;
        case AniArgsType::ANI_NUMBER:
            appEventPack->AddParam(key, HiAppEventAniUtil::ParseNumberValue(env, element));
            break;
        case AniArgsType::ANI_STRING:
            appEventPack->AddParam(key, HiAppEventAniUtil::ParseStringValue(env, element));
            break;
        default:
            result_ = ERR_PARAM;
            HILOG_ERROR(LOG_CORE, "param value type is invalid");
            return false;
    }
    return true;
}

ani_object HiAppEventAniHelper::WriteResult(ani_env *env, std::pair<int32_t, std::string> result)
{
    ani_object results_obj {};
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_RESULTS, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        HILOG_ERROR(LOG_CORE, "get %{public}s ctor Failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    if (ANI_OK != env->Object_New(cls, ctor, &results_obj)) {
        HILOG_ERROR(LOG_CORE, "Create Object Failed %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method codeSetter {};
    if (ANI_OK != env->Class_FindMethod(cls, "<set>code", nullptr, &codeSetter)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod Fail %{public}s", CLASS_NAME_RESULTS);
    }

    if (ANI_OK != env->Object_CallMethod_Void(results_obj, codeSetter, result.first)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Void Fail %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method messageSetter {};
    if (ANI_OK != env->Class_FindMethod(cls, "<set>message", nullptr, &messageSetter)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod Fail %{public}s", CLASS_NAME_RESULTS);
    }

    std::string message = result.second;
    ani_string message_string {};
    env->String_NewUTF8(message.c_str(), message.size(), &message_string);

    if (ANI_OK != env->Object_CallMethod_Void(results_obj, messageSetter, message_string)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Void Fail %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    return results_obj;
}

std::pair<int32_t, std::string> HiAppEventAniHelper::BuildErrorByResult(int32_t result)
{
    const std::map<int32_t, std::pair<int32_t, std::string>> codeMap = {
        { ERR_CODE_SUCC,
            { ERR_CODE_SUCC, "Success." } },
        { ERR_PARAM,
            { ERR_CODE_PARAMETER_ERROR, "Parameter error." } },
        { ErrorCode::ERROR_INVALID_EVENT_NAME,
            { ERR_INVALID_NAME, "Invalid event name." } },
        { ErrorCode::ERROR_INVALID_EVENT_DOMAIN,
            { ERR_INVALID_DOMAIN, "Invalid event domain." } },
        { ErrorCode::ERROR_HIAPPEVENT_DISABLE,
            { ERR_DISABLE, "Function disabled." } },
        { ErrorCode::ERROR_INVALID_PARAM_NAME,
            { ERR_INVALID_KEY, "Invalid event parameter name." } },
        { ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH,
            { ERR_INVALID_STR_LEN, "Invalid string length of the event parameter." } },
        { ErrorCode::ERROR_INVALID_PARAM_NUM,
            { ERR_INVALID_PARAM_NUM, "Invalid number of event parameters." } },
        { ErrorCode::ERROR_INVALID_LIST_PARAM_SIZE,
            { ERR_INVALID_ARR_LEN, "Invalid array length of the event parameter." } },
        { ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM,
            { ERR_INVALID_CUSTOM_PARAM_NUM, "The number of parameter keys exceeds the limit." }},
    };
    return codeMap.at(result);
}

bool HiAppEventAniHelper::ParseParamsInAppEventPack(ani_env *env, ani_ref params,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    if (HiAppEventAniUtil::IsRefUndefined(env, params)) {
        HILOG_ERROR(LOG_CORE, "AppEventInfo params undefined");
        result_ = ERR_PARAM;
        return false;
    }
    std::map<std::string, ani_ref> appEventParams;
    HiAppEventAniUtil::ParseRecord(env, static_cast<ani_object>(params), appEventParams);
    for (const auto &param : appEventParams) {
        if (param.first.length() > MAX_LENGTH_OF_PARAM_NAME) {
            result_ = ERROR_INVALID_PARAM_NAME;
            HILOG_INFO(LOG_CORE, "the length=%{public}zu of the param key is invalid", param.first.length());
            continue;
        }
        if (!HiAppEventAniHelper::AddAppEventPackParam(env, param, appEventPack)) {
            return false;
        }
    }
    return true;
}

int32_t HiAppEventAniHelper::GetResult()
{
    return result_;
}

bool HiAppEventAniHelper::AddAppEventPackParam(ani_env *env,
    std::pair<std::string, ani_ref> recordTemp, std::shared_ptr<AppEventPack> &appEventPack)
{
    if (HiAppEventAniUtil::IsArray(env, static_cast<ani_object>(recordTemp.second))) {
        if (!HiAppEventAniHelper::AddArrayParamToAppEventPack(env, recordTemp.first, recordTemp.second, appEventPack)) {
            return false;
        }
    } else {
        if (!HiAppEventAniHelper::AddParamToAppEventPack(env, recordTemp.first, recordTemp.second, appEventPack)) {
            return false;
        }
    }
    return true;
}
