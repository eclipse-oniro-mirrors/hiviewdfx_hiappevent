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
    cJSON *filtersJson = cJSON_CreateArray();
    if (!filtersJson) {
        HILOG_ERROR(LOG_CORE, "Create filtersJson cJson array failed.");
        return filtersStr_;
    }
    auto filters = GetFilters();
    for (const auto& filter : filters) {
        cJSON *filterJson = cJSON_CreateObject();
        if (!filterJson) {
            HILOG_ERROR(LOG_CORE, "Create filterJson cJson object failed, continue.");
            continue;
        }
        cJSON_AddStringToObject(filterJson, HiAppEvent::DOMAIN_PROPERTY, filter.domain.c_str());
        cJSON *namesJson = cJSON_CreateArray();
        if (!namesJson) {
            HILOG_ERROR(LOG_CORE, "Create namesJson cJson array failed, continue.");
            continue;
        }
        for (const auto& name : filter.names) {
            cJSON_AddItemToArray(namesJson, cJSON_CreateString(name.c_str()));
        }
        cJSON_AddItemToObject(filterJson, HiAppEvent::NAMES_PROPERTY, namesJson);
        cJSON_AddNumberToObject(filterJson, HiAppEvent::TYPES_PROPERTY, filter.types);
        cJSON_AddItemToArray(filtersJson, filterJson);
    }
    auto itemStr = cJSON_PrintUnformatted(filtersJson);
    filtersStr_ = itemStr;
    cJSON_free(itemStr);
    return filtersStr_;
}

void AppEventWatcher::SetFiltersStr(const std::string& jsonStr)
{
    std::vector<AppEventFilter> filters;
    if (jsonStr.empty()) {
        return;
    }
    cJSON *filtersJson = cJSON_Parse(jsonStr.c_str());
    if (!filtersJson) {
        HILOG_ERROR(LOG_CORE, "parse filters failed, please check the style of json");
        return;
    }
    if (!cJSON_IsArray(filtersJson) || cJSON_IsNull(filtersJson)) {
        HILOG_WARN(LOG_CORE, "filters is empty");
        cJSON_Delete(filtersJson);
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(filtersJson);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(filtersJson, i);
        if (item && cJSON_IsObject(item)) {
            std::string domain = EventJsonUtil::ParseString(item, HiAppEvent::DOMAIN_PROPERTY);
            std::unordered_set<std::string> names;
            EventJsonUtil::ParseStrings(item, HiAppEvent::NAMES_PROPERTY, names);
            uint32_t types = EventJsonUtil::ParseUInt32(item, HiAppEvent::TYPES_PROPERTY);
            filters.emplace_back(AppEventFilter(domain, names, types));
        }
    }
    SetFilters(filters);
    std::lock_guard<std::mutex> lockGuard(mutex_);
    filtersStr_ = jsonStr;
    cJSON_Delete(filtersJson);
}
} // namespace HiviewDFX
} // namespace OHOS
