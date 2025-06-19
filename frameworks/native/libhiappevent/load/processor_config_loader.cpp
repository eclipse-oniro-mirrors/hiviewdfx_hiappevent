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
#include "cJSON.h"
#include "file_ex.h"
#include "event_json_util.h"

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
    const ReportConfig& GetReportConfig() const;

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
    int ParseStringProp(const std::string& key, std::string& out,
            const std::function<bool(std::string)>& validationFunc);
    int ParseBoolProp(const std::string& key, bool& out);
    int ParseIntProp(const std::string& key, int& out, const std::function<bool(int)>& validationFunc);
    int ParseUnorderedSetProp(const std::string& key, std::unordered_set<std::string>& out,
        const std::function<bool(std::string)>& validationFunc);
    int ParseConfigReportProp(const cJSON *eventConfig, HiAppEvent::EventConfig& reportConf);

private:
    ReportConfig conf_;
    std::shared_ptr<cJSON> jsonConfig_;
};

ProcessorConfigLoader::ProcessorConfigLoader()
    : impl_(std::make_unique<Impl>())
{}

ProcessorConfigLoader::~ProcessorConfigLoader() = default;

const ReportConfig& ProcessorConfigLoader::GetReportConfig() const
{
    return impl_->GetReportConfig();
}

bool ProcessorConfigLoader::LoadProcessorConfig(const std::string& processorName, const std::string& configName)
{
    return impl_->LoadProcessorConfig(processorName, configName);
}

int ProcessorConfigLoader::Impl::ParseStringProp(const std::string& key, std::string& out,
    const std::function<bool(std::string)>& validationFunc)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(jsonConfig_.get(), key.c_str());
    if (!item) {
        out = "";
        return ERR_CODE_SUCC;
    }
    if (!cJSON_IsString(item) || !validationFunc(item->valuestring)) {
        return ERR_CODE_PARAM_INVALID;
    }
    out = item->valuestring;
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseBoolProp(const std::string& key, bool& out)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(jsonConfig_.get(), key.c_str());
    if (!item) {
        out = false;
        return ERR_CODE_SUCC;
    }
    if (cJSON_IsBool(item)) {
        out = cJSON_IsTrue(item);
        return ERR_CODE_SUCC;
    }
    return ERR_CODE_PARAM_INVALID;
}

int ProcessorConfigLoader::Impl::ParseIntProp(const std::string& key, int& out,
    const std::function<bool(int)>& validationFunc)
{
    cJSON* item = cJSON_GetObjectItemCaseSensitive(jsonConfig_.get(), key.c_str());
    if (!item) {
        out = 0;
        return ERR_CODE_SUCC;
    }
    if (!EventJsonUtil::CJsonIsInt(item) || !validationFunc(static_cast<int32_t>(item->valuedouble))) {
        return ERR_CODE_PARAM_INVALID;
    }
    out = static_cast<int32_t>(item->valuedouble);
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseUnorderedSetProp(const std::string& key, std::unordered_set<std::string>& out,
    const std::function<bool(std::string)>& validationFunc)
{
    std::unordered_set<std::string> curSet;
    cJSON* item = cJSON_GetObjectItemCaseSensitive(jsonConfig_.get(), key.c_str());
    if (!item) {
        out = std::move(curSet);
        return ERR_CODE_SUCC;
    }
    if (!cJSON_IsArray(item)) {
        return ERR_CODE_PARAM_INVALID;
    }
    size_t jsonSize = cJSON_GetArraySize(item);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *it = cJSON_GetArrayItem(item, i);
        if (!cJSON_IsString(it) || !validationFunc(it->valuestring)) {
            return ERR_CODE_PARAM_INVALID;
        }
        curSet.insert(it->valuestring);
    }
    out = std::move(curSet);
    return ERR_CODE_SUCC;
}

int ProcessorConfigLoader::Impl::ParseConfigReportProp(const cJSON *eventConfig,
    HiAppEvent::EventConfig& reportConf)
{
    cJSON* configDomain = cJSON_GetObjectItemCaseSensitive(eventConfig, EVENT_CONFIG_DOMAIN);
    if (configDomain) {
        if (!cJSON_IsString(configDomain)) {
            HILOG_WARN(LOG_CORE, "Parameter error. The event domain parameter is invalid.");
            return ERR_CODE_PARAM_INVALID;
        }
        reportConf.domain = configDomain->valuestring;
    }
    cJSON* configName = cJSON_GetObjectItemCaseSensitive(eventConfig, EVENT_CONFIG_NAME);
    if (configName) {
        if (!cJSON_IsString(configName)) {
            HILOG_WARN(LOG_CORE, "Parameter error. The event name parameter is invalid.");
            return ERR_CODE_PARAM_INVALID;
        }
        reportConf.name = configName->valuestring;
    }
    cJSON* configRealtime = cJSON_GetObjectItemCaseSensitive(eventConfig, EVENT_CONFIG_REALTIME);
    if (configRealtime) {
        if (!cJSON_IsBool(configRealtime)) {
            HILOG_WARN(LOG_CORE, "Parameter error. The event isRealTime parameter is invalid.");
            return ERR_CODE_PARAM_INVALID;
        }
        reportConf.isRealTime = cJSON_IsTrue(configRealtime);
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
    cJSON* configs = cJSON_GetObjectItemCaseSensitive(jsonConfig_.get(), EVENT_CONFIGS);
    if (!configs) {
        conf_.eventConfigs = std::move(eventConfigs);
        return ERR_CODE_SUCC;
    }
    if (!cJSON_IsArray(configs)) {
        return ERR_CODE_PARAM_INVALID;
    }
    size_t jsonSize = cJSON_GetArraySize(configs);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *eventConfig = cJSON_GetArrayItem(configs, i);
        if (!cJSON_IsObject(eventConfig)) {
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
    cJSON* configs = cJSON_GetObjectItemCaseSensitive(jsonConfig_.get(), CUSTOM_CONFIG);
    if (!configs) {
        conf_.customConfigs = std::move(customConfigs);
        return ERR_CODE_SUCC;
    }
    if (!cJSON_IsObject(configs)) {
        return ERR_CODE_PARAM_INVALID;
    }
    const std::vector<std::string> members = EventJsonUtil::CJsonGetMemberNames(configs);
    for (const auto& name : members) {
        const cJSON *item = cJSON_GetObjectItemCaseSensitive(configs, name.c_str());
        if (!cJSON_IsString(item) || !IsValidCustomConfig(name, item->valuestring)) {
            return ERR_CODE_PARAM_INVALID;
        }
        customConfigs.insert(std::pair<std::string, std::string>(name, item->valuestring));
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
    std::string content;
    if (!LoadStringFromFile(PROCESSOR_CONFIG_PATH, content)) {
        HILOG_ERROR(LOG_CORE, "failed to open the processor config file, path:%{public}s.", PROCESSOR_CONFIG_PATH);
        return false;
    }

    std::shared_ptr<cJSON> jsonConfig(cJSON_Parse(content.c_str()), [](cJSON *object) {
        if (object != nullptr) {
            cJSON_Delete(object);
        }
    });
    if (!jsonConfig) {
        HILOG_ERROR(LOG_CORE, "failed to parse the processor config file, path:%{public}s.", PROCESSOR_CONFIG_PATH);
        return false;
    }
    if (cJSON_IsNull(jsonConfig.get()) || !cJSON_IsObject(jsonConfig.get())) {
        HILOG_ERROR(LOG_CORE, "the processor config file is invalid.");
        return false;
    }
    cJSON* item = cJSON_GetObjectItemCaseSensitive(jsonConfig.get(), configName.c_str());
    if (!item || !cJSON_IsObject(item)) {
        HILOG_ERROR(LOG_CORE, "the configName=%{public}s is invalid in config file.", configName.c_str());
        return false;
    }
    conf_.name = processorName;
    conf_.configName = configName;
    jsonConfig_ = std::shared_ptr<cJSON>(cJSON_Duplicate(item, true), [](cJSON *object) {
        if (object != nullptr) {
            cJSON_Delete(object);
        }
    });
    return ParseProcessorConfig();
}

const ReportConfig& ProcessorConfigLoader::Impl::GetReportConfig() const
{
    return conf_;
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS