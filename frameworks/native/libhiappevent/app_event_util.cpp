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

#include <cinttypes>

#include "application_context.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "event_json_util.h"
#include "hilog/log.h"
#include "hisysevent_c.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "time_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "EventUtil"
#endif

namespace OHOS {
namespace HiviewDFX {
namespace AppEventUtil {

constexpr int MIN_APP_UID = 20000;
constexpr int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
constexpr int TEN_MS = 10;

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

sptr<AppExecFwk::IBundleMgr> GetBundleManager()
{
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemManager) {
        HILOG_ERROR(LOG_CORE, "Get system ability manager failed");
        return nullptr;
    }
    auto remoteObject = systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOG_ERROR(LOG_CORE, "Get system ability failed");
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    return bundleMgrProxy;
}

AppExecFwk::BundleInfo* GetBundleInfo()
{
    static std::mutex bundleInfoMutex;
    std::lock_guard<std::mutex> lock(bundleInfoMutex);
    static bool isInit = false;
    static AppExecFwk::BundleInfo bundleInfo;
    if (!isInit) {
        int64_t beginTime = TimeUtil::GetElapsedMilliSecondsSinceBoot();
        auto bundleInstance = GetBundleManager();
        if (bundleInstance == nullptr) {
            HILOG_ERROR(LOG_CORE, "bundleInstance is nullptr");
            return nullptr;
        }
        int32_t flag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_REQUESTED_PERMISSION);
        auto ret = bundleInstance->GetBundleInfoForSelf(flag, bundleInfo);
        if (ret != ERR_OK) {
            HILOG_ERROR(LOG_CORE, "GetBundleInfoForSelf failed! ret = %{public}d", ret);
            return nullptr;
        }
        isInit = true;
        int64_t nowTime = TimeUtil::GetElapsedMilliSecondsSinceBoot();
        if (nowTime >= beginTime && nowTime - beginTime > TEN_MS) {
            HILOG_INFO(LOG_CORE, "GetBundleInfo elapsed %{public}" PRId64 " ms", nowTime - beginTime);
        }
    }
    return &bundleInfo;
}

void GetApplicationInfo(std::string& bundleName, std::string& appVersion, std::string& runningId)
{
    if (getuid() < MIN_APP_UID) {
        HILOG_INFO(LOG_CORE, "The uid is not application UID");
        return;
    }
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "The context is null");
        return;
    }
    runningId = context->GetAppRunningUniqueId();
    int64_t beginTime = TimeUtil::GetElapsedMilliSecondsSinceBoot();
    auto bundleInfo = GetBundleInfo();
    int64_t nowTime = TimeUtil::GetElapsedMilliSecondsSinceBoot();
    if (nowTime >= beginTime && nowTime - beginTime > TEN_MS) {
        HILOG_INFO(LOG_CORE, "GetBundleInfo elapsed %{public}" PRId64 " ms", nowTime - beginTime);
    }
    if (bundleInfo == nullptr) {
        return;
    }
    bundleName = bundleInfo->name;
    appVersion = bundleInfo->versionName;
}

void SaveExternalLogSolidLink(std::shared_ptr<AppEventPack> event, const std::vector<std::string> &linkExternalLogs)
{
    Json::Value paramsJson;
    std::string paramString = event->GetParamStr();
    if (!EventJsonUtil::GetJsonObjectFromJsonString(paramsJson, paramString)) {
        HILOG_WARN(LOG_CORE, "hiappevent: parse param info failed, please check the style of json");
        return;
    }
    if (!paramsJson.isMember("external_log") || !paramsJson["external_log"].isArray()) {
        HILOG_INFO(LOG_CORE, "hiappevent: event %{public}s not have external_log.", event->GetName().c_str());
        return;
    }

    paramsJson["external_log"].clear();
    for (const std::string &linkExternalLog : linkExternalLogs) {
        paramsJson["external_log"].append(linkExternalLog);
    }

    event->SetParamStr(Json::FastWriter().write(paramsJson));
}
} // namespace AppEventUtil
} // namespace HiviewDFX
} // namespace OHOS
