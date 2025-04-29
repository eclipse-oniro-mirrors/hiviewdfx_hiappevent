/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "app_event_watcher.h"

#include "event_json_util.h"
#include "hiappevent_common.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Watcher"

namespace OHOS {
namespace HiviewDFX {
AppEventWatcher::AppEventWatcher(const std::string& name) : AppEventObserver(name) {};

AppEventWatcher::AppEventWatcher(
    const std::string& name,
    const std::vector<AppEventFilter>& filters,
    TriggerCondition cond)
    : AppEventObserver(name, filters, cond) {};

uint64_t AppEventWatcher::GetOsEventsMask()
{
    uint64_t mask = 0;
    auto filters = GetFilters();
    std::for_each(filters.begin(), filters.end(), [&mask](const auto& filter) {
        mask |= filter.GetOsEventsMask();
    });
    return mask;
}

std::string AppEventWatcher::GetFiltersStr()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (!filtersStr_.empty()) {
        return filtersStr_;
    }
    Json::Value filtersJson(Json::arrayValue);
    auto filters = GetFilters();
    for (const auto& filter : filters) {
        Json::Value filterJson;
        filterJson[HiAppEvent::DOMAIN_PROPERTY] = filter.domain;
        Json::Value namesJson(Json::arrayValue);
        for (const auto& name : filter.names) {
            namesJson.append(name);
        }
        filterJson[HiAppEvent::NAMES_PROPERTY] = namesJson;
        filterJson[HiAppEvent::TYPES_PROPERTY] = filter.types;
        filtersJson.append(filterJson);
    }
    filtersStr_ = Json::FastWriter().write(filtersJson);
    return filtersStr_;
}

void AppEventWatcher::SetFiltersStr(const std::string& jsonStr)
{
    std::vector<AppEventFilter> filters;
    if (jsonStr.empty()) {
        return;
    }
    Json::Value filtersJson;
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, filtersJson)) {
        HILOG_ERROR(LOG_CORE, "parse filters failed, please check the style of json");
        return;
    }
    if (!filtersJson.isArray() || filtersJson.empty()) {
        HILOG_WARN(LOG_CORE, "filters is empty");
        return;
    }
    for (Json::ArrayIndex i = 0; i < filtersJson.size(); ++i) {
        if (filtersJson[i].isObject()) {
            std::string domain = EventJsonUtil::ParseString(filtersJson[i], HiAppEvent::DOMAIN_PROPERTY);
            std::unordered_set<std::string> names;
            EventJsonUtil::ParseStrings(filtersJson[i], HiAppEvent::NAMES_PROPERTY, names);
            uint32_t types = EventJsonUtil::ParseUInt32(filtersJson[i], HiAppEvent::TYPES_PROPERTY);
            filters.emplace_back(AppEventFilter(domain, names, types));
        }
    }
    SetFilters(filters);
    std::lock_guard<std::mutex> lockGuard(mutex_);
    filtersStr_ = jsonStr;
}
} // namespace HiviewDFX
} // namespace OHOS
