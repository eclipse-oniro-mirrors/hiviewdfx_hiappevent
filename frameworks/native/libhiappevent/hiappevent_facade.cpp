/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hiappevent_facade.h"

#include "app_event_observer_mgr.h"
#include "app_event_stat.h"
#include "app_event_store.h"
#include "event_policy_mgr.h"
#include "file_util.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_userinfo.h"
#include "hiappevent_verify.h"
#include "hiappevent_write.h"
#include "time_util.h"

namespace OHOS {
namespace HiviewDFX {

// AppEventConfigFacade
bool AppEventConfigFacade::GetDisable()
{
    return HiAppEventConfig::GetInstance().GetDisable();
}

uint64_t AppEventConfigFacade::GetMaxStorageSize()
{
    return HiAppEventConfig::GetInstance().GetMaxStorageSize();
}

std::string AppEventConfigFacade::GetStorageDir()
{
    return HiAppEventConfig::GetInstance().GetStorageDir();
}

void AppEventConfigFacade::SetStorageDir(const std::string& dir)
{
    HiAppEventConfig::GetInstance().SetStorageDir(dir);
}

bool AppEventConfigFacade::SetConfigurationItem(const std::string& name, const std::string& value)
{
    return HiAppEventConfig::GetInstance().SetConfigurationItem(name, value);
}

std::string AppEventConfigFacade::GetRunningId()
{
    return HiAppEventConfig::GetInstance().GetRunningId();
}

void AppEventConfigFacade::RefreshFreeSize()
{
    return HiAppEventConfig::GetInstance().RefreshFreeSize();
}

// AppEventWriteFacade
int AppEventWriteFacade::FacadeSetEventParam(std::shared_ptr<AppEventPack> pack)
{
    return SetEventParam(pack);
}

void AppEventWriteFacade::FacadeWriteEvent(std::shared_ptr<AppEventPack> pack)
{
    WriteEvent(pack);
}

int AppEventWriteFacade::SetEventPolicy(const std::string& name,
    const std::map<std::string, std::string>& configMap)
{
    return EventPolicyMgr::GetInstance().SetEventPolicy(name, configMap);
}

int AppEventWriteFacade::SetEventPolicy(const std::string& name,
    const std::map<uint8_t, uint32_t>& configMap)
{
    return EventPolicyMgr::GetInstance().SetEventPolicy(name, configMap);
}

// AppEventObserverFacade
int64_t AppEventObserverFacade::AddProcessor(const std::string& name, const HiAppEvent::ReportConfig& conf)
{
    return AppEventObserverMgr::GetInstance().AddProcessor(name, conf);
}

int AppEventObserverFacade::RemoveObserver(int64_t observerSeq)
{
    return AppEventObserverMgr::GetInstance().RemoveObserver(observerSeq);
}

int AppEventObserverFacade::RemoveObserver(const std::string& observerName)
{
    return AppEventObserverMgr::GetInstance().RemoveObserver(observerName);
}

int AppEventObserverFacade::Load(const std::string& moduleName)
{
    return AppEventObserverMgr::GetInstance().Load(moduleName);
}

void AppEventObserverFacade::HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    AppEventObserverMgr::GetInstance().HandleEvents(events);
}

void AppEventObserverFacade::HandleTimeout()
{
    AppEventObserverMgr::GetInstance().HandleTimeout();
}

void AppEventObserverFacade::HandleBackground()
{
    AppEventObserverMgr::GetInstance().HandleBackground();
}

int64_t AppEventObserverFacade::AddWatcher(std::shared_ptr<AppEventWatcher> watcher)
{
    return AppEventObserverMgr::GetInstance().AddWatcher(watcher);
}

void AppEventObserverFacade::SubmitTaskToFFRTQueue(std::function<void()>&& task,
    const std::string& taskName)
{
    AppEventObserverMgr::GetInstance().SubmitTaskToFFRTQueue(std::move(task), taskName);
}

int AppEventObserverFacade::RegisterProcessor(const std::string& name,
    std::shared_ptr<HiAppEvent::AppEventProcessor> processor)
{
    return AppEventObserverMgr::GetInstance().RegisterProcessor(name, processor);
}

int AppEventObserverFacade::UnregisterProcessor(const std::string& name)
{
    return AppEventObserverMgr::GetInstance().UnregisterProcessor(name);
}

int AppEventObserverFacade::SetReportConfig(int64_t observerSeq, const HiAppEvent::ReportConfig& config)
{
    return AppEventObserverMgr::GetInstance().SetReportConfig(observerSeq, config);
}

int AppEventObserverFacade::GetReportConfig(int64_t observerSeq, HiAppEvent::ReportConfig& config)
{
    return AppEventObserverMgr::GetInstance().GetReportConfig(observerSeq, config);
}

// AppEventUserInfoFacade
int AppEventUserInfoFacade::SetUserId(const std::string& name, const std::string& value)
{
    return HiAppEvent::UserInfo::GetInstance().SetUserId(name, value);
}

int AppEventUserInfoFacade::GetUserId(const std::string& name, std::string& out)
{
    return HiAppEvent::UserInfo::GetInstance().GetUserId(name, out);
}

int AppEventUserInfoFacade::RemoveUserId(const std::string& name)
{
    return HiAppEvent::UserInfo::GetInstance().RemoveUserId(name);
}

int AppEventUserInfoFacade::SetUserProperty(const std::string& name, const std::string& value)
{
    return HiAppEvent::UserInfo::GetInstance().SetUserProperty(name, value);
}

int AppEventUserInfoFacade::GetUserProperty(const std::string& name, std::string& out)
{
    return HiAppEvent::UserInfo::GetInstance().GetUserProperty(name, out);
}

int AppEventUserInfoFacade::RemoveUserProperty(const std::string& name)
{
    return HiAppEvent::UserInfo::GetInstance().RemoveUserProperty(name);
}

int64_t AppEventUserInfoFacade::GetUserIdVersion()
{
    return HiAppEvent::UserInfo::GetInstance().GetUserIdVersion();
}

int64_t AppEventUserInfoFacade::GetUserPropertyVersion()
{
    return HiAppEvent::UserInfo::GetInstance().GetUserPropertyVersion();
}

// AppEventStoreFacade
int AppEventStoreFacade::QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events,
    int64_t observerSeq, int row)
{
    return AppEventStore::GetInstance().QueryEvents(events, observerSeq, row);
}

bool AppEventStoreFacade::DeleteData(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    return AppEventStore::GetInstance().DeleteData(observerSeq, eventSeqs);
}

int AppEventStoreFacade::DeleteEventMapping(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    return AppEventStore::GetInstance().DeleteEventMapping(observerSeq, eventSeqs);
}

int64_t AppEventStoreFacade::QueryObserverSeq(const std::string& name)
{
    return AppEventStore::GetInstance().QueryObserverSeq(name);
}

int AppEventStoreFacade::QueryObserverSeqs(const std::string& name,
    std::vector<int64_t>& observerSeqs)
{
    return AppEventStore::GetInstance().QueryObserverSeqs(name, observerSeqs);
}

int AppEventStoreFacade::TakeEvents(std::vector<std::shared_ptr<AppEventPack>>& events,
    int64_t observerSeq, uint32_t size)
{
    return AppEventStore::GetInstance().TakeEvents(events, observerSeq, size);
}

int AppEventStoreFacade::InitDbStore()
{
    return AppEventStore::GetInstance().InitDbStore();
}

int AppEventStoreFacade::DestroyDbStore()
{
    return AppEventStore::GetInstance().DestroyDbStore();
}

void AppEventStoreFacade::CheckStorageSpace()
{
    HiAppEventClean::CheckStorageSpace();
}

void AppEventStoreFacade::ClearData(const std::string& dir)
{
    HiAppEventClean::ClearData(dir);
}

bool AppEventStoreFacade::IsStorageSpaceFull(const std::string& dir, uint64_t maxSize)
{
    return HiAppEventClean::IsStorageSpaceFull(dir, maxSize);
}

void AppEventStoreFacade::ReleaseSomeStorageSpace(const std::string& dir, uint64_t maxSize)
{
    HiAppEventClean::ReleaseSomeStorageSpace(dir, maxSize);
}

// AppEventVerifyFacade
int AppEventVerifyFacade::VerifyTheAppEvent(std::shared_ptr<AppEventPack> pack)
{
    return VerifyAppEvent(pack);
}

int AppEventVerifyFacade::VerifyTheCustomEventParams(std::shared_ptr<AppEventPack> pack)
{
    return VerifyCustomEventParams(pack);
}

int AppEventVerifyFacade::VerifyTheReportConfig(HiAppEvent::ReportConfig& config)
{
    return VerifyReportConfig(config);
}

bool AppEventVerifyFacade::VerifyIsApp()
{
    return IsApp();
}

bool AppEventVerifyFacade::VerifyIsValidAppId(const std::string& name)
{
    return IsValidAppId(name);
}

bool AppEventVerifyFacade::VerifyIsValidBatchReport(int count)
{
    return IsValidBatchReport(count);
}

bool AppEventVerifyFacade::VerifyIsValidConfigId(int configId)
{
    return IsValidConfigId(configId);
}

bool AppEventVerifyFacade::VerifyIsValidConfigNameLength(const std::string& configName)
{
    return IsValidConfigNameLength(configName);
}

bool AppEventVerifyFacade::VerifyIsValidCustomConfig(const std::string& name, const std::string& value)
{
    return IsValidCustomConfig(name, value);
}

bool AppEventVerifyFacade::VerifyIsValidCustomConfigsNum(size_t num)
{
    return IsValidCustomConfigsNum(num);
}

bool AppEventVerifyFacade::VerifyIsValidDomain(const std::string& domain)
{
    return IsValidDomain(domain);
}

bool AppEventVerifyFacade::VerifyIsValidEventConfig(const HiAppEvent::EventConfig& eventCfg)
{
    return IsValidEventConfig(eventCfg);
}

bool AppEventVerifyFacade::VerifyIsValidEventName(const std::string& eventName)
{
    return IsValidEventName(eventName);
}

bool AppEventVerifyFacade::VerifyIsValidEventType(int eventType)
{
    return IsValidEventType(eventType);
}

bool AppEventVerifyFacade::VerifyIsValidPeriodReport(int timeout)
{
    return IsValidPeriodReport(timeout);
}

bool AppEventVerifyFacade::VerifyIsValidProcessorName(const std::string& name)
{
    return IsValidProcessorName(name);
}

bool AppEventVerifyFacade::VerifyIsValidRouteInfo(const std::string& name)
{
    return IsValidRouteInfo(name);
}

bool AppEventVerifyFacade::VerifyIsValidUserIdName(const std::string& name)
{
    return IsValidUserIdName(name);
}

bool AppEventVerifyFacade::VerifyIsValidUserIdValue(const std::string& value)
{
    return IsValidUserIdValue(value);
}

bool AppEventVerifyFacade::VerifyIsValidUserPropName(const std::string& name)
{
    return IsValidUserPropName(name);
}

bool AppEventVerifyFacade::VerifyIsValidUserPropValue(const std::string& value)
{
    return IsValidUserPropValue(value);
}

bool AppEventVerifyFacade::VerifyIsValidWatcherName(const std::string& watcherName)
{
    return IsValidWatcherName(watcherName);
}

// AppEventUtilityFacade
void AppEventUtilityFacade::WriteApiEndEventAsync(const std::string& apiName,
    uint64_t beginTime, int result, int errCode)
{
    AppEventStat::WriteApiEndEventAsync(apiName, beginTime, result, errCode);
}

int AppEventUtilityFacade::WriteApiEndMetric(const std::string& apiName,
    uint64_t beginTime, int result, int errCode)
{
    return AppEventStat::WriteApiEndMetric(apiName, beginTime, result, errCode);
}

std::string AppEventUtilityFacade::GetDate()
{
    return TimeUtil::GetDate();
}

uint64_t AppEventUtilityFacade::GetElapsedMilliSecondsSinceBoot()
{
    return static_cast<uint64_t>(TimeUtil::GetElapsedMilliSecondsSinceBoot());
}

bool AppEventUtilityFacade::ForceCreateDirectory(const std::string& dir)
{
    return FileUtil::ForceCreateDirectory(dir);
}

std::string AppEventUtilityFacade::GetFilePathByDir(const std::string& dir, const std::string& fileName)
{
    return FileUtil::GetFilePathByDir(dir, fileName);
}

bool AppEventUtilityFacade::IsFileExists(const std::string& file)
{
    return FileUtil::IsFileExists(file);
}

bool AppEventUtilityFacade::SaveStringToFile(const std::string& file, const std::string& content)
{
    return FileUtil::SaveStringToFile(file, content);
}
} // namespace HiviewDFX
} // namespace OHOS
