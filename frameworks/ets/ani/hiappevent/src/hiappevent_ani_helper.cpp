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

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_ANI_HELPER"

using namespace OHOS::HiviewDFX;

namespace {
constexpr size_t MAX_LENGTH_OF_PARAM_NAME = 32;
constexpr int32_t INVALID_OUT = -1;

typedef struct ConfigProp {
    std::string key;
    int32_t(*func)(ani_env*, ani_object, const std::string&, ReportConfig&);
} ConfigProp;
}

static bool AddParamToCustomConfigs(ani_env *env, ani_ref recordRef, HiAppEvent::ReportConfig &conf)
{
    if (!HiAppEventAniUtil::IsRefUndefined(env, recordRef)) {
        std::map<std::string, ani_ref> CustomConfigsRecord;
        HiAppEventAniUtil::ParseRecord(env, static_cast<ani_object>(recordRef), CustomConfigsRecord);
        if (!IsValidCustomConfigsNum(CustomConfigsRecord.size())) {
            HILOG_WARN(LOG_CORE, "invalid keys size=%{public}zu", CustomConfigsRecord.size());
            return false;
        }
        for (const auto &CustomConfigsTemp : CustomConfigsRecord) {
            conf.customConfigs[CustomConfigsTemp.first] =
                HiAppEventAniUtil::ParseStringValue(env, CustomConfigsTemp.second);
        }
    }
    return true;
}

static int32_t GetConfigIdValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.configId = static_cast<int32_t>(HiAppEventAniUtil::ParseNumberValue(env, ref));
    }
    return ERR_CODE_SUCC;
}

static int32_t GetRouteInfoRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.routeInfo = HiAppEventAniUtil::ParseStringValue(env, ref);
        if (!IsValidRouteInfo(out.routeInfo)) {
            return ERR_CODE_PARAM_INVALID;
        }
    }
    return ERR_CODE_SUCC;
}

static int32_t GetAppIdRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.appId = HiAppEventAniUtil::ParseStringValue(env, ref);
        if (!IsValidAppId(out.appId)) {
            return ERR_CODE_PARAM_INVALID;
        }
    }
    return ERR_CODE_SUCC;
}

static int32_t GetNameRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (ref == nullptr) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Invalid processor name.");
        return ERR_CODE_PARAM_FORMAT;
    }
    std::string name = HiAppEventAniUtil::ParseStringValue(env, ref);
    if (!IsValidProcessorName(name)) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Invalid processor name.");
        return ERR_CODE_PARAM_FORMAT;
    }
    out.name = name;
    return ERR_CODE_SUCC;
}

static int32_t GetPeriodReportInt(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.timeout = static_cast<int32_t>(HiAppEventAniUtil::ParseNumberValue(env, ref));
        if (!IsValidPeriodReport(out.triggerCond.timeout)) {
            return ERR_CODE_PARAM_INVALID;
        }
    }
    return ERR_CODE_SUCC;
}

static int32_t GetBatchReportInt(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.row = static_cast<int32_t>(HiAppEventAniUtil::ParseNumberValue(env, ref));
        if (!IsValidBatchReport(out.triggerCond.row)) {
            return ERR_CODE_PARAM_INVALID;
        }
    }
    return ERR_CODE_SUCC;
}

static int32_t GetUserIdsRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    std::unordered_set<std::string> userIdNames;
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        HiAppEventAniUtil::GetStringsToSet(env, ref, userIdNames);
        for (auto userId : userIdNames) {
            if (!IsValidUserIdName(userId)) {
                return ERR_CODE_PARAM_INVALID;
            }
        }
    }
    out.userIdNames = userIdNames;
    return ERR_CODE_SUCC;
}

static int32_t GetUserPropertyRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    std::unordered_set<std::string> userPropertyNames;
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        HiAppEventAniUtil::GetStringsToSet(env, ref, userPropertyNames);
        for (auto userProperty : userPropertyNames) {
            if (!IsValidUserPropName(userProperty)) {
                return ERR_CODE_PARAM_INVALID;
            }
        }
    }
    out.userPropertyNames = userPropertyNames;
    return ERR_CODE_SUCC;
}

static int32_t ParseEventConfigsValue(ani_env *env, ani_ref Ref, std::vector<EventConfig> &arr)
{
    ani_size length = 0;
    if (env->Array_GetLength(static_cast<ani_array_ref>(Ref), &length) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get length.");
        return ERR_CODE_PARAM_INVALID;
    }
    EventConfig config;
    for (ani_size i = 0; i < length; i++) {
        ani_ref value {};
        if (env->Array_Get_Ref(static_cast<ani_array_ref>(Ref), i, &value) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get length");
            return ERR_CODE_PARAM_INVALID;
        }
        ani_ref domainRef =
            HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(value), EVENT_CONFIG_DOMAIN.c_str());
        config.domain = HiAppEventAniUtil::ParseStringValue(env, domainRef);
        ani_ref nameRef =
            HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(value), EVENT_CONFIG_NAME.c_str());
        config.name = HiAppEventAniUtil::ParseStringValue(env, nameRef);
        ani_ref isRealTimeBol =
            HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(value), EVENT_CONFIG_REALTIME.c_str());
        if (!HiAppEventAniUtil::IsRefUndefined(env, isRealTimeBol)) {
            config.isRealTime = HiAppEventAniUtil::ParseBoolValue(env, isRealTimeBol);
            arr.push_back(config);
        }
    }
    return ERR_CODE_SUCC;
}

static int32_t GetEventConfigsRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    std::vector<EventConfig> arr;
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        ParseEventConfigsValue(env, ref, arr);
        out.eventConfigs = arr;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetCustomConfigRefValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        return ERR_CODE_SUCC;
    }
    if (!AddParamToCustomConfigs(env, ref, out)) {
        HILOG_WARN(LOG_CORE, "AddParamToCustomConfigs failed");
        return ERR_CODE_PARAM_INVALID;
    }
    return ERR_CODE_SUCC;
}

static int32_t GetDebugModeValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.debugMode = HiAppEventAniUtil::ParseBoolValue(env, ref);
    }
    return ERR_CODE_SUCC;
}

static int32_t GetOnStartuplValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.onStartup = HiAppEventAniUtil::ParseBoolValue(env, ref);
    }
    return ERR_CODE_SUCC;
}

static int32_t GetOnBackgroundValue(ani_env *env, ani_object processor, const std::string &key, ReportConfig &out)
{
    ani_ref ref = HiAppEventAniUtil::GetProperty(env, processor, key);
    if (!HiAppEventAniUtil::IsRefUndefined(env, ref)) {
        out.triggerCond.onBackground = HiAppEventAniUtil::ParseBoolValue(env, ref);
    }
    return ERR_CODE_SUCC;
}

static const ConfigProp CONFIG_PROPS[] = {
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
        .func = GetCustomConfigRefValue
    }
};

static int32_t TransConfig(ani_env *env, ani_object processor, ReportConfig& out)
{
    for (const auto &prop : CONFIG_PROPS) {
        int32_t ret = (prop.func)(env, processor, prop.key, out);
        if (ret == ERR_CODE_PARAM_FORMAT) {
            HILOG_ERROR(LOG_CORE, "failed to add processor, params format error");
            return ERR_CODE_PARAM_FORMAT;
        } else if (ret == ERR_CODE_PARAM_INVALID) {
            HILOG_WARN(LOG_CORE, "Parameter error. The %{public}s parameter is invalid.", prop.key.c_str());
        }
    }
    return ERR_CODE_SUCC;
}

bool HiAppEventAniHelper::AddProcessor(ani_env *env, ani_object processor, int64_t &out)
{
    ReportConfig conf;
    int32_t ret = TransConfig(env, processor, conf);
    if (ret != 0) {
        HILOG_ERROR(LOG_CORE, "TransConfig failed.");
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
    int64_t processorId = AppEventObserverMgr::GetInstance().AddProcessor(name, conf);
    if (processorId <= 0) {
        HILOG_WARN(LOG_CORE, "failed to add processor=%{public}s, register processor error", name.c_str());
        out = INVALID_OUT;
        return false;
    }
    out = processorId;
    return true;
}

bool HiAppEventAniHelper::GetPropertyDomain(ani_object info, ani_env *env, std::string &domain)
{
    ani_ref domainResultTemp {};
    if (env->Object_GetPropertyByName_Ref(info, "domain", &domainResultTemp) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get property domain failed");
        return false;
    }

    domain = HiAppEventAniUtil::ParseStringValue(env, domainResultTemp);
    return true;
}

bool HiAppEventAniHelper::GetPropertyName(ani_object info, ani_env *env, std::string &name)
{
    ani_ref nameResultTemp {};
    if (env->Object_GetPropertyByName_Ref(info, "name", &nameResultTemp) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get property name failed");
        return false;
    }

    name = HiAppEventAniUtil::ParseStringValue(env, nameResultTemp);
    return true;
}

bool HiAppEventAniHelper::GeteventTypeValue(ani_object info, ani_env *env, int32_t &enumValue)
{
    ani_ref eventTypeRef {};
    if (env->Object_GetPropertyByName_Ref(info, "eventType", &eventTypeRef) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get property eventType failed");
        return false;
    }

    ani_enum_item eventTypeItem = static_cast<ani_enum_item>(eventTypeRef);
    if (HiAppEventAniHelper::ParseEnumGetValueInt32(env, eventTypeItem, enumValue) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "fail to get 'enumValue'");
        return false;
    }
    return true;
}

ani_status HiAppEventAniHelper::ParseEnumGetValueInt32(ani_env *env, ani_enum_item enumItem, int32_t &value)
{
    ani_int aniInt = 0;
    if (env->EnumItem_GetValue_Int(enumItem, &aniInt) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "enumItem get int value failed");
        return ANI_ERROR;
    }
    value = static_cast<int32_t>(aniInt);
    return ANI_OK;
}

bool HiAppEventAniHelper::AddArrayParamToAppEventPack(ani_env *env, const std::string &key, ani_ref arrayRef,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    AniArgsType arrayType = HiAppEventAniUtil::GetArrayType(env, static_cast<ani_array_ref>(arrayRef));
    switch (arrayType) {
        case AniArgsType::ANI_INT: {
            std::vector<int> ints = HiAppEventAniUtil::GetInts(env, arrayRef);
            appEventPack->AddParam(key, ints);
            break;
        }
        case AniArgsType::ANI_BOOLEAN: {
            std::vector<bool> bools = HiAppEventAniUtil::GetBooleans(env, arrayRef);
            appEventPack->AddParam(key, bools);
            break;
        }
        case AniArgsType::ANI_NUMBER: {
            std::vector<double> doubles = HiAppEventAniUtil::GetDoubles(env, arrayRef);
            appEventPack->AddParam(key, doubles);
            break;
        }
        case AniArgsType::ANI_STRING: {
            std::vector<std::string> strs = HiAppEventAniUtil::GetStrings(env, arrayRef);
            appEventPack->AddParam(key, strs);
            break;
        }
        case AniArgsType::ANI_NULL: {
            appEventPack->AddParam(key);
            break;
        }
        default:
            HILOG_ERROR(LOG_CORE, "array param value type is invalid");
            return false;
    }
    return true;
}

bool HiAppEventAniHelper::AddParamToAppEventPack(ani_env *env, const std::string &key, ani_ref element,
    std::shared_ptr<AppEventPack> &appEventPack)
{
    ani_object elementObj = static_cast<ani_object>(element);
    AniArgsType type = HiAppEventAniUtil::GetArgType(env, elementObj);
    if (type <= AniArgsType::ANI_UNKNOWN || type >= AniArgsType::ANI_UNDEFINED) {
        return false;
    }
    switch (type) {
        case AniArgsType::ANI_INT:
            appEventPack->AddParam(key, HiAppEventAniUtil::ParseIntValue(env, element));
            break;
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
            HILOG_ERROR(LOG_CORE, "param value type is invalid");
            return false;
    }
    return true;
}

ani_object HiAppEventAniHelper::WriteResult(ani_env *env, std::pair<int32_t, std::string> result)
{
    ani_object results_obj {};
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_RESULTS, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to find class %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get method %{public}s <ctor> failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    if (env->Object_New(cls, ctor, &results_obj) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method codeSetter {};
    if (env->Class_FindMethod(cls, "<set>code", nullptr, &codeSetter) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get method %{public}s failed", CLASS_NAME_RESULTS);
    }

    if (env->Object_CallMethod_Void(results_obj, codeSetter, result.first) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method messageSetter {};
    if (env->Class_FindMethod(cls, "<set>message", nullptr, &messageSetter) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", CLASS_NAME_RESULTS);
    }

    std::string message = result.second;
    ani_string message_string {};
    env->String_NewUTF8(message.c_str(), message.size(), &message_string);

    if (env->Object_CallMethod_Void(results_obj, messageSetter, message_string) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method Fail %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    return results_obj;
}

std::pair<int32_t, std::string> HiAppEventAniHelper::BuildErrorByResult(int32_t result)
{
    const std::map<int32_t, std::pair<int32_t, std::string>> codeMap = {
        { ERR_CODE_SUCC,
            { ERR_CODE_SUCC, "Success." } },
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
