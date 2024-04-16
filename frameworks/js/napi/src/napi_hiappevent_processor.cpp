/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "napi_hiappevent_processor.h"

#include <cinttypes>
#include <map>
#include <string>
#include <unordered_set>

#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "module_loader.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiHiAppEventProcessor"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventProcessor {
namespace {
constexpr int ERR_CODE_SUCC = 0;
constexpr int ERR_CODE_PARAM_FORMAT = -1;
constexpr int ERR_CODE_PARAM_INVALID = -2;

const std::string PROCESSOR_NAME = "name";
const std::string DEBUG_MODE = "debugMode";
const std::string ROUTE_INFO = "routeInfo";
const std::string APP_ID = "appId";
const std::string START_REPORT = "onStartReport";
const std::string BACKGROUND_REPORT = "onBackgroundReport";
const std::string PERIOD_REPORT = "periodReport";
const std::string BATCH_REPORT = "batchReport";
const std::string USER_IDS = "userIds";
const std::string USER_PROPERTIES = "userProperties";
const std::string EVENT_CONFIGS = "eventConfigs";
const std::string EVENT_CONFIG_DOMAIN = "domain";
const std::string EVENT_CONFIG_NAME = "name";
const std::string EVENT_CONFIG_REALTIME = "isRealTime";
const std::string CONFIG_ID = "configId";
const std::string CUSTOM_CONFIG = "customConfigs";

const std::string CONFIG_PROP_TYPE_STR = "string";
const std::string CONFIG_PROP_TYPE_STR_ARRAY = "string array";
const std::string CONFIG_PROP_TYPE_BOOL = "boolean";
const std::string CONFIG_PROP_TYPE_NUM = "number";
const std::string CONFIG_PROP_TYPE_EVENT_CONFIG = "AppEventReportConfig array";
const std::string CONFIG_PROP_TYPE_RECORD_STRING = "Record string";
}

bool GenConfigStrProp(const napi_env env, const napi_value config, const std::string& key, std::string& out,
    const bool optional = true)
{
    if (!NapiUtil::HasProperty(env, config, key)) {
        return optional;
    }
    napi_value value = NapiUtil::GetProperty(env, config, key);
    if (value == nullptr || !NapiUtil::IsString(env, value)) {
        return false;
    }
    out = NapiUtil::GetString(env, value);
    return true;
}

bool GenConfigStrsProp(const napi_env env, const napi_value config, const std::string& key,
    std::unordered_set<std::string>& out)
{
    if (NapiUtil::HasProperty(env, config, key)) {
        napi_value value = NapiUtil::GetProperty(env, config, key);
        if (value == nullptr || !NapiUtil::IsArray(env, value) || !NapiUtil::IsArrayType(env, value, napi_string)) {
            return false;
        }
        NapiUtil::GetStringsToSet(env, value, out);
    }
    return true;
}

bool GenConfigBoolProp(const napi_env env, const napi_value config, const std::string& key, bool& out)
{
    if (NapiUtil::HasProperty(env, config, key)) {
        napi_value value = NapiUtil::GetProperty(env, config, key);
        if (value == nullptr || !NapiUtil::IsBoolean(env, value)) {
            return false;
        }
        out = NapiUtil::GetBoolean(env, value);
    }
    return true;
}

bool GenConfigIntProp(const napi_env env, const napi_value config, const std::string& key, int32_t& out)
{
    if (NapiUtil::HasProperty(env, config, key)) {
        napi_value value = NapiUtil::GetProperty(env, config, key);
        if (value == nullptr || !NapiUtil::IsNumber(env, value)) {
            return false;
        }
        out = NapiUtil::GetInt32(env, value);
    }
    return true;
}

int GenConfigNameProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    std::string name;
    if (!GenConfigStrProp(env, config, key, name, false)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg(key, CONFIG_PROP_TYPE_STR));
        return ERR_CODE_PARAM_FORMAT;
    }
    if (!IsValidProcessorName(name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Invalid processor name.");
        return ERR_CODE_PARAM_FORMAT;
    }
    out.name = name;
    return ERR_CODE_SUCC;
}

int GenConfigRouteInfoProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    std::string routeInfo;
    if (!GenConfigStrProp(env, config, key, routeInfo) || !IsValidRouteInfo(routeInfo)) {
        return ERR_CODE_PARAM_INVALID;
    }
    out.routeInfo = routeInfo;
    return ERR_CODE_SUCC;
}

int GenConfigAppIdProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    std::string appId;
    if (!GenConfigStrProp(env, config, key, appId) || !IsValidAppId(appId)) {
        return ERR_CODE_PARAM_INVALID;
    }
    out.appId = appId;
    return ERR_CODE_SUCC;
}

int GenConfigUserIdsProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    std::unordered_set<std::string> userIdNames;
    if (!GenConfigStrsProp(env, config, key, userIdNames)) {
        return ERR_CODE_PARAM_INVALID;
    }
    for (auto userId : userIdNames) {
        if (!IsValidUserIdName(userId)) {
            return ERR_CODE_PARAM_INVALID;
        }
    }
    out.userIdNames = userIdNames;
    return ERR_CODE_SUCC;
}

int GenConfigUserPropertiesProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    std::unordered_set<std::string> userPropertyNames;
    if (!GenConfigStrsProp(env, config, key, userPropertyNames)) {
        return ERR_CODE_PARAM_INVALID;
    }
    for (auto userProperty : userPropertyNames) {
        if (!IsValidUserPropName(userProperty)) {
            return ERR_CODE_PARAM_INVALID;
        }
    }
    out.userPropertyNames = userPropertyNames;
    return ERR_CODE_SUCC;
}

int GenConfigDebugModeProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigBoolProp(env, config, key, out.debugMode) ? ERR_CODE_SUCC : ERR_CODE_PARAM_INVALID;
}

int GenConfigStartReportProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigBoolProp(env, config, key, out.triggerCond.onStartup) ? ERR_CODE_SUCC : ERR_CODE_PARAM_INVALID;
}

int GenConfigBackgroundReportProp(const napi_env env, const napi_value config, const std::string& key,
    ReportConfig& out)
{
    return GenConfigBoolProp(env, config, key, out.triggerCond.onBackground) ? ERR_CODE_SUCC : ERR_CODE_PARAM_INVALID;
}

int GenConfigPeriodReportProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    int timeout = 0;
    if (!GenConfigIntProp(env, config, key, timeout) || !IsValidPeriodReport(timeout)) {
        return ERR_CODE_PARAM_INVALID;
    }
    out.triggerCond.timeout = timeout;
    return ERR_CODE_SUCC;
}

int GenConfigBatchReportProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    int row = 0;
    if (!GenConfigIntProp(env, config, key, row) || !IsValidBatchReport(row)) {
        return ERR_CODE_PARAM_INVALID;
    }
    out.triggerCond.row = row;
    return ERR_CODE_SUCC;
}

int GenConfigReportProp(const napi_env env, const napi_value config, HiAppEvent::EventConfig& out)
{
    HiAppEvent::EventConfig reportConf;
    if (!GenConfigStrProp(env, config, EVENT_CONFIG_DOMAIN, reportConf.domain)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The event domain parameter is invalid.");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!GenConfigStrProp(env, config, EVENT_CONFIG_NAME, reportConf.name)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The event name parameter is invalid.");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!GenConfigBoolProp(env, config, EVENT_CONFIG_REALTIME, reportConf.isRealTime)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The event isRealTime parameter is invalid.");
        return ERR_CODE_PARAM_INVALID;
    }
    if (!IsValidEventConfig(reportConf)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The event config is invalid, domain=%{public}s, name=%{public}s.",
            reportConf.domain.c_str(), reportConf.name.c_str());
        return ERR_CODE_PARAM_INVALID;
    }
    out = reportConf;
    return ERR_CODE_SUCC;
}

int GenConfigEventConfigsProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    if (NapiUtil::HasProperty(env, config, key)) {
        napi_value napiArr = NapiUtil::GetProperty(env, config, key);
        if (!NapiUtil::IsArray(env, napiArr)) {
            return ERR_CODE_PARAM_INVALID;
        }
        std::vector<HiAppEvent::EventConfig> eventConfigs;
        uint32_t length = NapiUtil::GetArrayLength(env, napiArr);
        for (uint32_t i = 0; i < length; i++) {
            napi_value element = NapiUtil::GetElement(env, napiArr, i);
            HiAppEvent::EventConfig reportConf;
            int ret = GenConfigReportProp(env, element, reportConf);
            if (ret != ERR_CODE_SUCC) {
                return ret;
            }
            eventConfigs.push_back(reportConf);
        }
        out.eventConfigs = eventConfigs;
    }
    return ERR_CODE_SUCC;
}

int GenConfigIdProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    if (!GenConfigIntProp(env, config, key, out.configId) || !IsValidConfigId(out.configId)) {
        HILOG_WARN(LOG_CORE, "invalid configId");
        return ERR_CODE_PARAM_INVALID;
    }
    return ERR_CODE_SUCC;
}

int GenConfigCustomConfigsProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    if (!NapiUtil::HasProperty(env, config, key)) {
        return ERR_CODE_SUCC;
    }
    napi_value napiObject = NapiUtil::GetProperty(env, config, key);
    if (napiObject == nullptr || !NapiUtil::IsObject(env, napiObject)) {
        HILOG_WARN(LOG_CORE, "invalid customConfigs");
        return ERR_CODE_PARAM_INVALID;
    }
    std::vector<std::string> keys;
    NapiUtil::GetPropertyNames(env, napiObject, keys);
    if (!IsValidCustomConfigsNum(keys.size())) {
        HILOG_WARN(LOG_CORE, "invalid keys size=%{public}zu", keys.size());
        return ERR_CODE_PARAM_INVALID;
    }
    std::unordered_map<std::string, std::string> customConfigs;
    for (const auto& localKey : keys) {
        std::string value;
        if (!GenConfigStrProp(env, napiObject, localKey, value) || !IsValidCustomConfig(localKey, value)) {
            HILOG_WARN(LOG_CORE, "invalid key name=%{public}s", localKey.c_str());
            return ERR_CODE_PARAM_INVALID;
        }
        customConfigs.insert(std::pair<std::string, std::string>(localKey, value));
    }
    out.customConfigs = customConfigs;
    return ERR_CODE_SUCC;
}

typedef struct ConfigProp {
    std::string type;
    std::string key;
    int (*func)(const napi_env, const napi_value, const std::string&, ReportConfig&);
} ConfigProp;

const ConfigProp CONFIG_PROPS[] = {
    {
        .type = CONFIG_PROP_TYPE_STR,
        .key = PROCESSOR_NAME,
        .func = GenConfigNameProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR,
        .key = ROUTE_INFO,
        .func = GenConfigRouteInfoProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR,
        .key = APP_ID,
        .func = GenConfigAppIdProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR_ARRAY,
        .key = USER_IDS,
        .func = GenConfigUserIdsProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR_ARRAY,
        .key = USER_PROPERTIES,
        .func = GenConfigUserPropertiesProp
    },
    {
        .type = CONFIG_PROP_TYPE_BOOL,
        .key = DEBUG_MODE,
        .func = GenConfigDebugModeProp
    },
    {
        .type = CONFIG_PROP_TYPE_BOOL,
        .key = START_REPORT,
        .func = GenConfigStartReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_BOOL,
        .key = BACKGROUND_REPORT,
        .func = GenConfigBackgroundReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_NUM,
        .key = PERIOD_REPORT,
        .func = GenConfigPeriodReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_NUM,
        .key = BATCH_REPORT,
        .func = GenConfigBatchReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_EVENT_CONFIG,
        .key = EVENT_CONFIGS,
        .func = GenConfigEventConfigsProp
    },
    {
        .type = CONFIG_PROP_TYPE_NUM,
        .key = CONFIG_ID,
        .func = GenConfigIdProp
    },
    {
        .type = CONFIG_PROP_TYPE_RECORD_STRING,
        .key = CUSTOM_CONFIG,
        .func = GenConfigCustomConfigsProp
    }
};

int TransConfig(const napi_env env, const napi_value config, ReportConfig& out)
{
    if (!NapiUtil::IsObject(env, config)) {
        HILOG_ERROR(LOG_CORE, "failed to add processor, params format error");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("config", "Processor"));
        return -1;
    }
    for (auto prop : CONFIG_PROPS) {
        int ret = (prop.func)(env, config, prop.key, out);
        if (ret == ERR_CODE_PARAM_FORMAT) {
            HILOG_ERROR(LOG_CORE, "failed to add processor, params format error");
            return -1;
        } else if (ret == ERR_CODE_PARAM_INVALID) {
            HILOG_WARN(LOG_CORE, "Parameter error. The %{public}s parameter is invalid.", prop.key.c_str());
        }
    }
    return 0;
}

bool AddProcessor(const napi_env env, const napi_value config, napi_value& out)
{
    ReportConfig conf;
    int ret = TransConfig(env, config, conf);
    if (ret != 0) {
        out = NapiUtil::CreateInt64(env, -1);
        return false;
    }
    std::string name = conf.name;
    if (name.empty()) {
        HILOG_ERROR(LOG_CORE, "processor name can not be empty.");
        out = NapiUtil::CreateInt64(env, -1);
        return false;
    }
    if (HiAppEvent::ModuleLoader::GetInstance().Load(name) != 0) {
        HILOG_WARN(LOG_CORE, "failed to add processor=%{public}s, name no found", name.c_str());
        out = NapiUtil::CreateInt64(env, -1);
        return true;
    }
    int64_t processorId = AppEventObserverMgr::GetInstance().RegisterObserver(name, conf);
    if (processorId <= 0) {
        HILOG_WARN(LOG_CORE, "failed to add processor=%{public}s, register processor error", name.c_str());
        out = NapiUtil::CreateInt64(env, -1);
        return false;
    }
    out = NapiUtil::CreateInt64(env, processorId);
    return true;
}

bool RemoveProcessor(const napi_env env, const napi_value id)
{
    if (!NapiUtil::IsNumber(env, id)) {
        HILOG_WARN(LOG_CORE, "failed to remove processor, params format error");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("id", "number"));
        return false;
    }
    int64_t processorId = NapiUtil::GetInt64(env, id);
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
} // namespace NapiHiAppEventProcessor
} // namespace HiviewDFX
} // namespace OHOS
