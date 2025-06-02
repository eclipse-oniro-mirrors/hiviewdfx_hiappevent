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
#include "processor_config_loader.h"

#include <fstream>
#include <map>
#include <unordered_set>

#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "json/json.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ProcessorConfigLoader"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
constexpr int ERR_CODE_SUCC = 0;
constexpr int ERR_CODE_PARAM_INVALID = -2;

const char* const DEBUG_MODE = "debugMode";
const char* const ROUTE_INFO = "routeInfo";
const char* const APP_ID = "appId";
const char* const START_REPORT = "onStartReport";
const char* const BACKGROUND_REPORT = "onBackgroundReport";
const char* const PERIOD_REPORT = "periodReport";
const char* const BATCH_REPORT = "batchReport";
const char* const USER_IDS = "userIds";
const char* const USER_PROPERTIES = "userProperties";
const char* const EVENT_CONFIGS = "eventConfigs";
const char* const EVENT_CONFIG_DOMAIN = "domain";
const char* const EVENT_CONFIG_NAME = "name";
const char* const EVENT_CONFIG_REALTIME = "isRealTime";
const char* const CONFIG_ID = "configId";
const char* const CUSTOM_CONFIG = "customConfigs";

const char* const PROCESSOR_CONFIG_PATH = "/system/etc/hiappevent/processor.json";

struct ConfigProp {
    const char* const key;
    std::function<int()> func;
};
}

class ProcessorConfigLoader::Impl {
public:
    bool LoadProcessorConfig(const std::string& processorName, const std::string& configName);
    ReportConfig GetReportConfig();

private:
    int ParseRouteInfoProp();
    int ParseAppIdProp();
    int ParseUserIdsProp();
    int ParseUserPropertiesProp();
    int ParseDebugModeProp();
    int ParseStartReportProp();
    int ParseBackgroundReportProp();
    int ParsePeriodReportProp();
    int ParseBatchReportProp();
    int ParseEventConfigsProp();
    int ParseConfigIdProp();
    int ParseCustomConfigsProp();

    bool ParseProcessorConfig();
    int ParseStringProp(const std::string& key, std::string& out, std::function<bool(std::string)> validationFunc);
    int ParseBoolProp(const std::string& key, bool& out);
    int ParseIntProp(const std::string& key, int& out, std::function<bool(int)> validationFunc);
    int ParseUnorderedSetProp(const std::string& key, std::unordered_set<std::string>& out,
        std::function<bool(std::string)> validationFunc);
    int ParseConfigReportProp(const Json::Value& eventConfig, HiAppEvent::EventConfig& reportConf);

private:
    ReportConfig conf_;
    Json::Value jsonConfig_;
};

ProcessorConfigLoader::ProcessorConfigLoader()
    : impl_(std::make_unique<Impl>())
{}

ProcessorConfigLoader::~ProcessorConfigLoader() = default;

ReportConfig ProcessorConfigLoader::GetReportConfig()
{
    return impl_->GetReportConfig();
}

bool ProcessorConfigLoader::LoadProcessorConfig(const std::string& processorName, const std::string& configName)
{
    return impl_->LoadProcessorConfig(processorName, configName);
}

int ProcessorConfigLoader::Impl::ParseStringProp(const std::string& key, std::string& out,
    std::function<bool(std::string)> validationFunc)
{
    if (!jsonConfig_.isMember(key)) {
        out = "";
        return ERR_CODE_SUCC;
    }
    if (!jsonConfig_[key].isString() || !validationFunc(jsonConfig_[key].asString())) {
        return ERR_CODE_PARAM_INVALID;
    }
    out = jsonConfig_[key].asString();
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseBoolProp(const std::string& key, bool& out)
{
    if (!jsonConfig_.isMember(key)) {
        out = false;
        return ERR_CODE_SUCC;
    }
    if (!jsonConfig_[key].isBool()) {
        return ERR_CODE_PARAM_INVALID;
    }
    out = jsonConfig_[key].asBool();
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseIntProp(const std::string& key, int& out, std::function<bool(int)> validationFunc)
{
    if (!jsonConfig_.isMember(key)) {
        out = 0;
        return ERR_CODE_SUCC;
    }
    if (!jsonConfig_[key].isIntegral() || !validationFunc(jsonConfig_[key].asInt())) {
        return ERR_CODE_PARAM_INVALID;
    }
    out = jsonConfig_[key].asInt();
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseUnorderedSetProp(const std::string& key, std::unordered_set<std::string>& out,
    std::function<bool(std::string)> validationFunc)
{
    std::unordered_set<std::string> curSet;
    if (!jsonConfig_.isMember(key)) {
        out = std::move(curSet);
        return ERR_CODE_SUCC;
    }
    if (!jsonConfig_[key].isArray()) {
        return ERR_CODE_PARAM_INVALID;
    }
    for (const auto& userId : jsonConfig_[key]) {
        if (!userId.isString() || !validationFunc(userId.asString())) {
            return ERR_CODE_PARAM_INVALID;
        }
        curSet.insert(userId.asString());
    }
    out = std::move(curSet);
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseConfigReportProp(const Json::Value& eventConfig,
    HiAppEvent::EventConfig& reportConf)
{
    if (eventConfig.isMember(EVENT_CONFIG_DOMAIN)) {
        if (!eventConfig[EVENT_CONFIG_DOMAIN].isString()) {
            HILOG_WARN(LOG_CORE, "Parameter error. The event domain parameter is invalid.");
            return ERR_CODE_PARAM_INVALID;
        }
        reportConf.domain = eventConfig[EVENT_CONFIG_DOMAIN].asString();
    }
    if (eventConfig.isMember(EVENT_CONFIG_NAME)) {
        if (!eventConfig[EVENT_CONFIG_NAME].isString()) {
            HILOG_WARN(LOG_CORE, "Parameter error. The event name parameter is invalid.");
            return ERR_CODE_PARAM_INVALID;
        }
        reportConf.name = eventConfig[EVENT_CONFIG_NAME].asString();
    }
    if (eventConfig.isMember(EVENT_CONFIG_REALTIME)) {
        if (!eventConfig[EVENT_CONFIG_REALTIME].isBool()) {
            HILOG_WARN(LOG_CORE, "Parameter error. The event isRealTime parameter is invalid.");
            return ERR_CODE_PARAM_INVALID;
        }
        reportConf.isRealTime = eventConfig[EVENT_CONFIG_REALTIME].asBool();
    }
    if (!IsValidEventConfig(reportConf)) {
        HILOG_WARN(LOG_CORE, "Parameter error. The event config is invalid, domain=%{public}s, name=%{public}s.",
            reportConf.domain.c_str(), reportConf.name.c_str());
        return ERR_CODE_PARAM_INVALID;
    }
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseRouteInfoProp()
{
    return ParseStringProp(ROUTE_INFO, conf_.routeInfo, IsValidRouteInfo);
}

int ProcessorConfigLoader::Impl::ParseAppIdProp()
{
    return ParseStringProp(APP_ID, conf_.appId, IsValidAppId);
}

int ProcessorConfigLoader::Impl::ParseUserIdsProp()
{
    return ParseUnorderedSetProp(USER_IDS, conf_.userIdNames, IsValidUserIdName);
}

int ProcessorConfigLoader::Impl::ParseUserPropertiesProp()
{
    return ParseUnorderedSetProp(USER_PROPERTIES, conf_.userPropertyNames, IsValidUserPropName);
}

int ProcessorConfigLoader::Impl::ParseDebugModeProp()
{
    return ParseBoolProp(DEBUG_MODE, conf_.debugMode);
}

int ProcessorConfigLoader::Impl::ParseStartReportProp()
{
    return ParseBoolProp(START_REPORT, conf_.triggerCond.onStartup);
}

int ProcessorConfigLoader::Impl::ParseBackgroundReportProp()
{
    return ParseBoolProp(BACKGROUND_REPORT, conf_.triggerCond.onBackground);
}

int ProcessorConfigLoader::Impl::ParsePeriodReportProp()
{
    return ParseIntProp(PERIOD_REPORT, conf_.triggerCond.timeout, IsValidPeriodReport);
}

int ProcessorConfigLoader::Impl::ParseBatchReportProp()
{
    return ParseIntProp(BATCH_REPORT, conf_.triggerCond.row, IsValidBatchReport);
}

int ProcessorConfigLoader::Impl::ParseConfigIdProp()
{
    return ParseIntProp(CONFIG_ID, conf_.configId, IsValidConfigId);
}

int ProcessorConfigLoader::Impl::ParseEventConfigsProp()
{
    std::vector<HiAppEvent::EventConfig> eventConfigs;
    if (!jsonConfig_.isMember(EVENT_CONFIGS)) {
        conf_.eventConfigs = std::move(eventConfigs);
        return ERR_CODE_SUCC;
    }
    if (!jsonConfig_[EVENT_CONFIGS].isArray()) {
        return ERR_CODE_PARAM_INVALID;
    }
    for (const auto& eventConfig : jsonConfig_[EVENT_CONFIGS]) {
        if (!eventConfig.isObject()) {
            return ERR_CODE_PARAM_INVALID;
        }
        HiAppEvent::EventConfig reportConf;
        if (ParseConfigReportProp(eventConfig, reportConf) != ERR_CODE_SUCC) {
            return ERR_CODE_PARAM_INVALID;
        }
        eventConfigs.push_back(reportConf);
    }
    conf_.eventConfigs = std::move(eventConfigs);
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseCustomConfigsProp()
{
    std::unordered_map<std::string, std::string> customConfigs;
    if (!jsonConfig_.isMember(CUSTOM_CONFIG)) {
        conf_.customConfigs = std::move(customConfigs);
        return ERR_CODE_SUCC;
    }
    if (!jsonConfig_[CUSTOM_CONFIG].isObject()) {
        return ERR_CODE_PARAM_INVALID;
    }
    const Json::Value::Members& members = jsonConfig_[CUSTOM_CONFIG].getMemberNames();
    for (const auto& name : members) {
        if (!jsonConfig_[CUSTOM_CONFIG][name].isString() ||
            !IsValidCustomConfig(name, jsonConfig_[CUSTOM_CONFIG][name].asString())) {
            return ERR_CODE_PARAM_INVALID;
        }
        customConfigs.insert(std::pair<std::string, std::string>(name, jsonConfig_[CUSTOM_CONFIG][name].asString()));
    }
    conf_.customConfigs = std::move(customConfigs);
    return ERR_CODE_SUCC;
}

bool ProcessorConfigLoader::Impl::ParseProcessorConfig()
{
    const ConfigProp CONFIG_PROPS[] = {
        { .key = ROUTE_INFO, .func = [this]() -> int { return this->ParseRouteInfoProp(); } },
        { .key = APP_ID, .func = [this]() -> int { return this->ParseAppIdProp(); } },
        { .key = USER_IDS, .func = [this]() -> int { return this->ParseUserIdsProp(); } },
        { .key = USER_PROPERTIES, .func = [this]() -> int { return this->ParseUserPropertiesProp(); } },
        { .key = DEBUG_MODE, .func = [this]() -> int { return this->ParseDebugModeProp(); } },
        { .key = START_REPORT, .func = [this]() -> int { return this->ParseStartReportProp(); } },
        { .key = BACKGROUND_REPORT, .func = [this]() -> int { return this->ParseBackgroundReportProp(); } },
        { .key = PERIOD_REPORT, .func = [this]() -> int { return this->ParsePeriodReportProp(); } },
        { .key = BATCH_REPORT, .func = [this]() -> int { return this->ParseBatchReportProp(); } },
        { .key = EVENT_CONFIGS, .func = [this]() -> int { return this->ParseEventConfigsProp(); } },
        { .key = CONFIG_ID, .func = [this]() -> int { return this->ParseConfigIdProp(); } },
        { .key = CUSTOM_CONFIG, .func = [this]() -> int { return this->ParseCustomConfigsProp(); } }
    };

    for (const auto& prop : CONFIG_PROPS) {
        if (prop.func() != ERR_CODE_SUCC) {
            HILOG_ERROR(LOG_CORE, "failed to load processor config item:%{public}s", prop.key);
            return false;
        }
    }
    return true;
}

bool ProcessorConfigLoader::Impl::LoadProcessorConfig(const std::string& processorName, const std::string& configName)
{
    std::ifstream file(PROCESSOR_CONFIG_PATH);
    if (!file.is_open()) {
        HILOG_ERROR(LOG_CORE, "failed to open the processor config file, path:%{public}s.", PROCESSOR_CONFIG_PATH);
        return false;
    }

    Json::Value jsonConfig;
    Json::CharReaderBuilder builder;
    Json::CharReaderBuilder::strictMode(&builder.settings_);
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, file, &jsonConfig, &errs)) {
        HILOG_ERROR(LOG_CORE, "failed to parse the processor config file, path:%{public}s.", PROCESSOR_CONFIG_PATH);
        return false;
    }
    if (jsonConfig.empty() || !jsonConfig.isObject()) {
        HILOG_ERROR(LOG_CORE, "the processor config file is invalid.");
        return false;
    }
    if (!jsonConfig.isMember(configName) || !jsonConfig[configName].isObject()) {
        HILOG_ERROR(LOG_CORE, "the configName=%{public}s is invalid in config file.", configName.c_str());
        return false;
    }
    conf_.name = processorName;
    conf_.configName = configName;
    jsonConfig_ = jsonConfig[configName];

    return ParseProcessorConfig();
}

ReportConfig ProcessorConfigLoader::Impl::GetReportConfig()
{
    return conf_;
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS