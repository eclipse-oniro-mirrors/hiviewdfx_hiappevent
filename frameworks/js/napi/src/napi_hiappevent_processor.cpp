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
#include "napi_hiappevent_processor.h"

#include <cinttypes>
#include <map>
#include <string>
#include <unordered_set>

#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "module_loader.h"
#include "napi_error.h"
#include "napi_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventProcessor {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Napi_HiAppEvent_Processor" };
constexpr size_t MAX_STRING_LEN = 8 * 1024 + 2; // 2 for '\0' and extra symbol

const std::string PROCESSOR_NAME = "name";
const std::string DEBUG_MODE = "debugMode";
const std::string ROUTE_INFO = "routeInfo";
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

const std::string CONFIG_PROP_TYPE_STR = "string";
const std::string CONFIG_PROP_TYPE_STR_ARRAY = "string array";
const std::string CONFIG_PROP_TYPE_BOOL = "boolean";
const std::string CONFIG_PROP_TYPE_NUM = "number";
const std::string CONFIG_PROP_TYPE_EVENT_CONFIG = "AppEventReportConfig array";
}

bool GenConfigStrProp(const napi_env env, const napi_value config, const std::string& key, std::string& out)
{
    if (NapiUtil::HasProperty(env, config, key)) {
        napi_value value = NapiUtil::GetProperty(env, config, key);
        if (value == nullptr || !NapiUtil::IsString(env, value)) {
            return false;
        }
        out = NapiUtil::GetString(env, value);
    }
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
        NapiUtil::GetStringsToSet(env, value, out, MAX_STRING_LEN);
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

bool GenConfigReportsProp(const napi_env env, const napi_value config, const std::string& key,
    std::vector<HiAppEvent::EventConfig>& out)
{
    if (NapiUtil::HasProperty(env, config, key)) {
        napi_value ary = NapiUtil::GetProperty(env, config, key);
        if (!NapiUtil::IsArray(env, ary)) {
            return false;
        }
        out.clear();
        uint32_t length = NapiUtil::GetArrayLength(env, ary);
        for (uint32_t i = 0; i < length; i++) {
            napi_value element = NapiUtil::GetElement(env, ary, i);
            HiAppEvent::EventConfig reportConf;
            if (!GenConfigStrProp(env, element, EVENT_CONFIG_DOMAIN, reportConf.domain)) {
                return false;
            }
            if (!GenConfigStrProp(env, element, EVENT_CONFIG_NAME, reportConf.name)) {
                return false;
            }
            if (!GenConfigBoolProp(env, element, EVENT_CONFIG_REALTIME, reportConf.isRealTime)) {
                return false;
            }
            out.push_back(reportConf);
        }
    }
    return true;
}

bool GenConfigNameProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigStrProp(env, config, key, out.name);
}

bool GenConfigRouteInfoProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigStrProp(env, config, key, out.routeInfo);
}

bool GenConfigUserIdsProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigStrsProp(env, config, key, out.userIdNames);
}

bool GenConfigUserPropertiesProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigStrsProp(env, config, key, out.userPropertyNames);
}

bool GenConfigDebugModeProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigBoolProp(env, config, key, out.debugMode);
}

bool GenConfigStartReportProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigBoolProp(env, config, key, out.triggerCond.onStartup);
}

bool GenConfigBackgroundReportProp(const napi_env env, const napi_value config, const std::string& key,
    ReportConfig& out)
{
    return GenConfigBoolProp(env, config, key, out.triggerCond.onBackground);
}

bool GenConfigPeriodReportProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigIntProp(env, config, key, out.triggerCond.timeout);
}

bool GenConfigBatchReportProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigIntProp(env, config, key, out.triggerCond.size);
}

bool GenConfigEventConfigsProp(const napi_env env, const napi_value config, const std::string& key, ReportConfig& out)
{
    return GenConfigReportsProp(env, config, key, out.eventConfigs);
}

typedef struct ConfigProp {
    std::string type;
    int code;
    std::string key;
    bool (*func)(const napi_env, const napi_value, const std::string&, ReportConfig&);
} ConfigProp;

const ConfigProp CONFIG_PROPS[] = {
    {
        .type = CONFIG_PROP_TYPE_STR,
        .code = NapiError::ERR_INVALID_PROCESSOR_NAME,
        .key = PROCESSOR_NAME,
        .func = GenConfigNameProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR,
        .code = NapiError::ERR_INVALID_PROCESSOR_ROUTE_INFO,
        .key = ROUTE_INFO,
        .func = GenConfigRouteInfoProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR_ARRAY,
        .code = NapiError::ERR_INVALID_PROCESSOR_USER_IDS,
        .key = USER_IDS,
        .func = GenConfigUserIdsProp
    },
    {
        .type = CONFIG_PROP_TYPE_STR_ARRAY,
        .code = NapiError::ERR_INVALID_PROCESSOR_USER_PROPERTIES,
        .key = USER_PROPERTIES,
        .func = GenConfigUserPropertiesProp
    },
    {
        .type = CONFIG_PROP_TYPE_BOOL,
        .code = NapiError::ERR_INVALID_PROCESSOR_DEBUG_MODE,
        .key = DEBUG_MODE,
        .func = GenConfigDebugModeProp
    },
    {
        .type = CONFIG_PROP_TYPE_BOOL,
        .code = NapiError::ERR_INVALID_PROCESSOR_START_REPORT,
        .key = START_REPORT,
        .func = GenConfigStartReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_BOOL,
        .code = NapiError::ERR_INVALID_PROCESSOR_BACKGROUND_REPORT,
        .key = BACKGROUND_REPORT,
        .func = GenConfigBackgroundReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_NUM,
        .code = NapiError::ERR_INVALID_PROCESSOR_PERIOD_REPORT,
        .key = PERIOD_REPORT,
        .func = GenConfigPeriodReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_NUM,
        .code = NapiError::ERR_INVALID_PROCESSOR_BATCH_REPORT,
        .key = BATCH_REPORT,
        .func = GenConfigBatchReportProp
    },
    {
        .type = CONFIG_PROP_TYPE_EVENT_CONFIG,
        .code = NapiError::ERR_INVALID_PROCESSOR_EVENT_CONFIGS,
        .key = EVENT_CONFIGS,
        .func = GenConfigEventConfigsProp
    }
};

int TransConfig(const napi_env env, const napi_value config, ReportConfig& out)
{
    if (!NapiUtil::IsObject(env, config)) {
        HiLog::Error(LABEL, "failed to add processor, params format error");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("config", "Processor"));
        return -1;
    }
    for (auto prop : CONFIG_PROPS) {
        if (!(prop.func)(env, config, prop.key, out)) {
        HiLog::Error(LABEL, "failed to add processor, params format error");
            NapiUtil::ThrowError(env, prop.code, NapiUtil::CreateErrMsg(prop.key, prop.type));
            return -1;
        }
    }
    return 0;
}

bool AddProcessor(const napi_env env, const napi_value config, napi_value& out)
{
    ReportConfig conf;
    int ret = TransConfig(env, config, conf);
    if (ret != 0) {
        HiLog::Error(LABEL, "failed to add processor, params format error");
        return false;
    }
    std::string name = conf.name;
    if (name.empty()) {
        HiLog::Error(LABEL, "processor name can not be empty.");
        return false;
    }
    if (HiAppEvent::ModuleLoader::GetInstance().Load(name) != 0) {
        HiLog::Warn(LABEL, "failed to add processor=%{public}s, name no found", name.c_str());
        return true;
    }
    int64_t processorId = AppEventObserverMgr::GetInstance().RegisterObserver(name);
    if (processorId <= 0) {
        HiLog::Warn(LABEL, "failed to add processor=%{public}s, register processor error", name.c_str());
        return false;
    }
    if (AppEventObserverMgr::GetInstance().SetReportConfig(processorId, conf) != 0) {
        HiLog::Warn(LABEL, "failed to add processor=%{public}s, set config error", name.c_str());
        return false;
    }
    out = NapiUtil::CreateInt64(env, processorId);
    return true;
}

bool RemoveProcessor(const napi_env env, const napi_value id)
{
    if (!NapiUtil::IsNumber(env, id)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("id", "number"));
        HiLog::Warn(LABEL, "failed to remove processor, params format error");
        return false;
    }
    int64_t processorId = NapiUtil::GetInt64(env, id);
    if (processorId < 0) {
        HiLog::Error(LABEL, "processor id invalid.");
        return false;
    }
    if (processorId == 0) {
        HiLog::Debug(LABEL, "failed to remove processor id=%{public}" PRId64, processorId);
        return true;
    }
    if (AppEventObserverMgr::GetInstance().UnregisterObserver(processorId) != 0) {
        HiLog::Debug(LABEL, "failed to remove processor id=%{public}" PRId64, processorId);
        return false;
    }
    return true;
}
} // namespace NapiHiAppEventProcessor
} // namespace HiviewDFX
} // namespace OHOS
