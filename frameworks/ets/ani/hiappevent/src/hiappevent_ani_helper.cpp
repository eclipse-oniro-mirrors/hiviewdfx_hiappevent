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
#include <cinttypes>

#include "hiappevent_ani_helper.h"

#include "app_event_observer_mgr.h"
#include "app_event_stat.h"
#include "ani_app_event_holder.h"
#include "ani_app_event_watcher.h"
#include "hiappevent_verify.h"
#include "hiappevent_config.h"
#include "hiappevent_userinfo.h"
#include "hilog/log.h"
#include "hilog/log_cpp.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_ANI_HELPER"

using namespace OHOS::HiviewDFX;
using namespace HiAppEvent;

namespace {
constexpr int32_t INVALID_OUT = -1;

typedef struct ConfigProp {
    std::string key;
    int32_t(*func)(ani_env*, ani_object, const std::string&, ReportConfig&);
} ConfigProp;

const std::vector<std::string> ConfigOptionKeys = {"disable", "maxStorage"};
const std::string COND_PROPS[] = {"row", "size", "timeOut"};
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
            arr.emplace_back(config);
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
    for (auto prop : CONFIG_PROPS) {
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
    int64_t processorId = AppEventObserverMgr::GetInstance().RegisterObserver(name, conf);
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
        case AniArgsType::ANI_DOUBLE: {
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
        case AniArgsType::ANI_DOUBLE:
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
        if (!AddArrayParamToAppEventPack(env, recordTemp.first, recordTemp.second, appEventPack)) {
            return false;
        }
    } else {
        if (!HiAppEventAniHelper::AddParamToAppEventPack(env, recordTemp.first, recordTemp.second, appEventPack)) {
            return false;
        }
    }
    return true;
}

bool HiAppEventAniHelper::Configure(ani_env *env, ani_object configObj)
{
    for (const auto &key: ConfigOptionKeys) {
        ani_ref valueRef = HiAppEventAniUtil::GetProperty(env, configObj, key);
        if (HiAppEventAniUtil::IsRefUndefined(env, valueRef)) {
            continue;
        }
        if (!HiAppEventConfig::GetInstance().SetConfigurationItem(
            key, HiAppEventAniUtil::ConvertToString(env, valueRef))) {
            std::string errMsg = "Invalid max storage quota value.";
            HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_MAX_STORAGE, errMsg);
            return false;
        }
    }
    return true;
}

bool HiAppEventAniHelper::SetUserId(ani_env *env, ani_string name, ani_string value)
{
    std::string strName = HiAppEventAniUtil::ParseStringValue(env, name);
    if (!IsValidUserIdName(strName)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The name parameter is invalid.");
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    std::string strUserId = HiAppEventAniUtil::ParseStringValue(env, value);
    if (strUserId.empty()) {
        if (HiAppEvent::UserInfo::GetInstance().RemoveUserId(strName) != 0) {
            HILOG_ERROR(LOG_CORE, "failed to remove userId");
            return false;
        }
        return true;
    }
    if (!IsValidUserIdValue(strUserId)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The value parameter is invalid.");
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Parameter error. The value parameter is invalid.");
        return false;
    }
    if (HiAppEvent::UserInfo::GetInstance().SetUserId(strName, strUserId) != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set userId");
        return false;
    }
    return true;
}

bool HiAppEventAniHelper::GetUserId(ani_env *env, ani_string name, ani_string &userId)
{
    std::string strName = HiAppEventAniUtil::ParseStringValue(env, name);
    if (!IsValidUserIdName(strName)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The name parameter is invalid.");
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    std::string strUserId = "";
    if (HiAppEvent::UserInfo::GetInstance().GetUserId(strName, strUserId) != 0) {
        HILOG_ERROR(LOG_CORE, "failed to get userId");
        return false;
    }
    env->String_NewUTF8(strUserId.c_str(), strUserId.size(), &userId);
    return true;
}

bool HiAppEventAniHelper::SetUserProperty(ani_env *env, ani_string name, ani_string value)
{
    std::string strName = HiAppEventAniUtil::ParseStringValue(env, name);
    if (!IsValidUserPropName(strName)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The name parameter is invalid.");
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    std::string strUserProperty = HiAppEventAniUtil::ParseStringValue(env, value);
    if (strUserProperty.empty()) {
        if (HiAppEvent::UserInfo::GetInstance().RemoveUserProperty(strName) != 0) {
            HILOG_ERROR(LOG_CORE, "failed to remove user property");
            return false;
        }
        return true;
    }
    if (!IsValidUserPropValue(strUserProperty)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The value parameter is invalid.");
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Parameter error. The value parameter is invalid.");
        return false;
    }
    if (HiAppEvent::UserInfo::GetInstance().SetUserProperty(strName, strUserProperty) != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set user property");
        return false;
    }
    return true;
}

bool HiAppEventAniHelper::GetUserProperty(ani_env *env, ani_string name, ani_string &userProperty)
{
    std::string strName = HiAppEventAniUtil::ParseStringValue(env, name);
    if (!IsValidUserPropName(strName)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The name parameter is invalid.");
        HiAppEventAniUtil::ThrowAniError(env, ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    std::string strUserProperty = "";
    if (HiAppEvent::UserInfo::GetInstance().GetUserProperty(strName, strUserProperty) != 0) {
        HILOG_ERROR(LOG_CORE, "failed to get user property");
        return false;
    }
    env->String_NewUTF8(strUserProperty.c_str(), strUserProperty.size(), &userProperty);
    return true;
}

bool HiAppEventAniHelper::RemoveProcessor(ani_env *env, ani_double id)
{
    int64_t processorId = static_cast<int64_t>(id);
    if (processorId <= 0) {
        HILOG_ERROR(LOG_CORE, "failed to remove processor id=%{public}" PRId64, processorId);
        return true;
    }
    if (AppEventObserverMgr::GetInstance().UnregisterObserver(processorId) != 0) {
        HILOG_WARN(LOG_CORE, "failed to remove processor id=%{public}" PRId64, processorId);
        return false;
    }
    return true;
}

static bool IsValidName(ani_env *env, ani_ref nameRef, int32_t& errCode)
{
    if (!IsValidWatcherName(HiAppEventAniUtil::ParseStringValue(env, nameRef))) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_WATCHER_NAME, "Invalid watcher name.");
        errCode = ERR_INVALID_WATCHER_NAME;
        return false;
    }
    return true;
}

static bool IsValidFilter(ani_env *env, ani_ref filterValue, int32_t& errCode)
{
    ani_ref domainRef =
        HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(filterValue), FILTER_DOMAIN.c_str());
    if (!IsValidDomain(HiAppEventAniUtil::ParseStringValue(env, domainRef))) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_FILTER_DOMAIN, "Invalid filtering event domain.");
        errCode = ERR_INVALID_FILTER_DOMAIN;
        return false;
    }
    return true;
}

static bool IsValidFilters(ani_env *env, ani_ref filtersRef, int32_t& errCode)
{
    if (HiAppEventAniUtil::IsRefUndefined(env, filtersRef)) {
        return true;
    }
    ani_size length = 0;
    if (env->Array_GetLength(static_cast<ani_array_ref>(filtersRef), &length) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get array length failed");
        return false;
    }
    for (ani_size i = 0; i < length; i++) {
        ani_ref filterValue {};
        if (env->Array_Get_Ref(static_cast<ani_array_ref>(filtersRef), i, &filterValue) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get array element failed");
            return false;
        }
        if (!IsValidFilter(env, filterValue, errCode)) {
            return false;
        }
    }
    return true;
}

static bool IsValidWatcher(ani_env *env, ani_object watcher, int32_t& errCode)
{
    return IsValidName(env, HiAppEventAniUtil::GetProperty(env, watcher, WATCHER_NAME.c_str()), errCode)
        && IsValidFilters(env, HiAppEventAniUtil::GetProperty(env, watcher, APPEVENT_FILTERS.c_str()), errCode);
}

static void GetFilters(ani_env *env, ani_object watcher, std::vector<AppEventFilter>& filters)
{
    ani_ref filtersRef = HiAppEventAniUtil::GetProperty(env, watcher, APPEVENT_FILTERS.c_str());
    if (HiAppEventAniUtil::IsRefUndefined(env, filtersRef)) {
        return;
    }
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array_ref>(filtersRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get array length failed");
        return;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref value {};
        if (env->Array_Get_Ref(static_cast<ani_array_ref>(filtersRef), i, &value) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get length");
            return;
        }
        ani_ref domainValue =
            HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(value), FILTER_DOMAIN.c_str());
        std::string domain = HiAppEventAniUtil::ParseStringValue(env, domainValue);
        ani_ref namesValue =
            HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(value), FILTER_NAMES.c_str());
        std::unordered_set<std::string> names;
        if (!HiAppEventAniUtil::IsRefUndefined(env, namesValue)) {
            HiAppEventAniUtil::GetStringsToSet(env, namesValue, names);
        }
        ani_ref typesValue =
            HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(value), FILTER_TYPES.c_str());
        if (HiAppEventAniUtil::IsRefUndefined(env, typesValue)) {
            filters.emplace_back(AppEventFilter(domain, names, BIT_ALL_TYPES));
            continue;
        }
        std::vector<int> types;
        HiAppEventAniUtil::GetIntValueToVector(env, typesValue, types);
        unsigned int filterType = 0;
        for (auto type : types) {
            filterType |= (BIT_MASK << type);
        }
        filterType = filterType > 0 ? filterType : BIT_ALL_TYPES;
        filters.emplace_back(AppEventFilter(domain, names, filterType));
    }
}

static int GetConditionValue(ani_env *env, ani_ref cond, const std::string& name)
{
    auto value = HiAppEventAniUtil::GetProperty(env, static_cast<ani_object>(cond), name);
    if (!HiAppEventAniUtil::IsRefUndefined(env, value)) {
        return HiAppEventAniUtil::ParseNumberValue(env, value);
    }
    return 0;
}

static bool GetCondition(ani_env *env, ani_object watcher, TriggerCondition& resCond)
{
    ani_ref cond = HiAppEventAniUtil::GetProperty(env, watcher, TRIGGER_CONDITION.c_str());
    if (HiAppEventAniUtil::IsRefUndefined(env, cond)) {
        return true;
    }

    size_t index = 0;
    int row = GetConditionValue(env, cond, COND_PROPS[index++]);
    if (row < 0) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_COND_ROW, "Invalid row value.");
        return false;
    }
    resCond.row = row;

    int size = GetConditionValue(env, cond, COND_PROPS[index++]);
    if (size < 0) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_COND_SIZE, "Invalid size value.");
        return false;
    }
    resCond.size = size;

    int timeout = GetConditionValue(env, cond, COND_PROPS[index++]);
    if (timeout < 0) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_COND_TIMEOUT, "Invalid timeout value.");
        return false;
    }
    resCond.timeout = timeout * HiAppEvent::TIMEOUT_STEP;
    return true;
}

static ani_object CreateHolderObject(ani_env *env, ani_string watcherName)
{
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_EVENT_PACKAGE_HOLDER, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    }

    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get %{public}s ctor Failed", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    }

    ani_object obj {};
    if (env->Object_New(cls, ctor, &obj, watcherName) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Create Object Failed: %{public}s", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    }
    return obj;
}

ani_object HiAppEventAniHelper::AddWatcher(ani_env *env, ani_object watcher, uint64_t beginTime)
{
    int32_t errCode = ERR_CODE_SUCC;
    if (!IsValidWatcher(env, watcher, errCode)) {
        HILOG_ERROR(LOG_CORE, "invalid watcher");
        AppEventStat::WriteApiEndEventAsync("addWatcher", beginTime, AppEventStat::FAILED, errCode);
        return {};
    }
    std::vector<AppEventFilter> filters;
    GetFilters(env, watcher, filters);
    std::string name = HiAppEventAniUtil::ParseStringValue(env,
        HiAppEventAniUtil::GetProperty(env, watcher, WATCHER_NAME.c_str()));
    TriggerCondition cond {
        .row = 0,
        .size = 0,
        .timeout = 0
    };
    if (!GetCondition(env, watcher, cond)) {
        return {};
    }
    auto watcherPtr = std::make_shared<AniAppEventWatcher>(name, filters, cond);

    ani_ref trigger = HiAppEventAniUtil::GetProperty(env, watcher, FUNCTION_ONTRIGGER.c_str());
    if (!HiAppEventAniUtil::IsRefUndefined(env, trigger)) {
        watcherPtr->InitTrigger(env, trigger);
    }

    ani_ref receiver = HiAppEventAniUtil::GetProperty(env, watcher, FUNCTION_ONRECEIVE.c_str());
    if (!HiAppEventAniUtil::IsRefUndefined(env, receiver)) {
        watcherPtr->InitReceiver(env, receiver);
    }

    int64_t observerSeq = AppEventObserverMgr::GetInstance().RegisterObserver(watcherPtr);
    if (observerSeq <= 0) {
        HILOG_ERROR(LOG_CORE, "invalid observer sequence");
        AppEventStat::WriteApiEndEventAsync("addWatcher", beginTime, AppEventStat::FAILED, ERR_CODE_SUCC);
        return {};
    }

    ani_object holder = CreateHolderObject(env, HiAppEventAniUtil::CreateAniString(env, name));
    watcherPtr->InitHolder(env, holder);
    AppEventStat::WriteApiEndEventAsync("addWatcher", beginTime, AppEventStat::SUCCESS, ERR_CODE_SUCC);
    return static_cast<ani_object>(holder);
}

void HiAppEventAniHelper::RemoveWatcher(ani_env *env, ani_object watcher, uint64_t beginTime)
{
    ani_ref nameRef = HiAppEventAniUtil::GetProperty(env, watcher, WATCHER_NAME.c_str());
    int32_t errCode = ERR_CODE_SUCC;
    if (!IsValidName(env, nameRef, errCode)) {
        AppEventStat::WriteApiEndEventAsync("removeWatcher", beginTime, AppEventStat::FAILED, errCode);
        return;
    }
    (void)AppEventObserverMgr::GetInstance().UnregisterObserver(HiAppEventAniUtil::ParseStringValue(env, nameRef));
    AppEventStat::WriteApiEndEventAsync("removeWatcher", beginTime, AppEventStat::SUCCESS, ERR_CODE_SUCC);
    return;
}