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
#include "app_event_util.h"
#include "event_json_util.h"
#include "hilog/log.h"
#include "hisysevent_c.h"
#include "parameters.h"

#ifdef LOG_MAIN
#undef LOG_MAIN
#define LOG_MAIN 0xD002D07
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "EventUtil"
#endif

namespace OHOS {
namespace HiviewDFX {
namespace AppEventUtil {

bool IsBetaVersion()
{
    constexpr char keyVersionType[] = "const.logsystem.versiontype";
    static bool isBetaVersion = OHOS::system::GetParameter(keyVersionType, "unknown").find("beta") !=
                                std::string::npos;
    return isBetaVersion;
}

void ReportAppEventReceive(const std::vector<std::shared_ptr<AppEventPack>>& appEventInfos,
                           const std::string& watcherName, const std::string& callback)
{
    if (!IsBetaVersion()) {
        HILOG_DEBUG(LOG_CORE, "no need to report APP_EVENT_RECEIVE event");
        return;
    }

    HiSysEventParam receiveParams[] = {
        { .name = "BUNDLENAME",       .t = HISYSEVENT_STRING,    .arraySize = 0, },
        { .name = "BUNDLEVERSION",    .t = HISYSEVENT_STRING,    .arraySize = 0, },
        { .name = "CALLBACK",         .t = HISYSEVENT_STRING,    .arraySize = 0, },
        { .name = "EVENTTYPE",        .t = HISYSEVENT_UINT8,     .arraySize = 0, },
        { .name = "CRASHTYPE",        .t = HISYSEVENT_STRING,    .arraySize = 0, },
        { .name = "WATCHERNAME",      .t = HISYSEVENT_STRING,    .arraySize = 0, },
        { .name = "EXTERNALLOG",      .t = HISYSEVENT_BOOL,      .arraySize = 0, }
    };
    for (const auto& appEvent : appEventInfos) {
        std::string eventName = appEvent->GetName();
        if (eventName != "APP_FREEZE" && eventName != "APP_CRASH") {
            HILOG_DEBUG(LOG_CORE, "only report APP_EVENT_SEND event for APP_FREEZE and APP_CRASH");
            continue;
        }
        Json::Value eventJson;
        std::string paramString = appEvent->GetParamStr();
        if (!EventJsonUtil::GetJsonObjectFromJsonString(eventJson, paramString) ||
            !eventJson.isMember("external_log") || !eventJson["external_log"].isArray()) {
            HILOG_WARN(LOG_CORE, "parse event detail info failed, please check the style of json");
            return;
        }
        std::string bundleName = EventJsonUtil::ParseString(eventJson, "bundle_name");
        std::string bundleVersion = EventJsonUtil::ParseString(eventJson, "bundle_version");
        std::string crashType = EventJsonUtil::ParseString(eventJson, "crash_type");
        uint64_t index = 0;
        receiveParams[index++].v = { .s = const_cast<char *>(bundleName.c_str()) };
        receiveParams[index++].v = { .s = const_cast<char *>(bundleVersion.c_str()) };
        receiveParams[index++].v = { .s = const_cast<char *>(callback.c_str()) };
        receiveParams[index++].v = { .ui8 = eventName == "APP_CRASH" ? 0 : 1 };
        receiveParams[index++].v = { .s = const_cast<char *>(crashType.c_str()) };
        receiveParams[index++].v = { .s = const_cast<char *>(watcherName.c_str()) };
        receiveParams[index++].v = { .b = eventJson["external_log"].size() > 0 };
        int ret = OH_HiSysEvent_Write("HIVIEWDFX", "APP_EVENT_RECEIVE", HISYSEVENT_STATISTIC, receiveParams,
                                      sizeof(receiveParams) / sizeof(receiveParams[0]));
        if (ret != 0) {
            HILOG_WARN(LOG_CORE, "fail to report APP_EVENT_RECEIVE event, ret =%{public}d", ret);
        }
    }
}
} // namespace AppEventUtil
} // namespace HiviewDFX
} // namespace OHOS
