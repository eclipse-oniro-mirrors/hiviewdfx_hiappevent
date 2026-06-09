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

#ifndef HIAPPEVENT_FACADE_H
#define HIAPPEVENT_FACADE_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "hiappevent_base.h"
#include "app_event_processor.h"
#include "app_event_watcher.h"

namespace OHOS {
namespace HiviewDFX {

class AppEventConfigFacade {
public:
    static bool GetDisable();
    static uint64_t GetMaxStorageSize();
    static std::string GetStorageDir();
    static void SetStorageDir(const std::string& dir);
    static bool SetConfigurationItem(const std::string& name, const std::string& value);
    static std::string GetRunningId();
    static void RefreshFreeSize();
};

class AppEventWriteFacade {
public:
    static int FacadeSetEventParam(std::shared_ptr<AppEventPack> pack);
    static void FacadeWriteEvent(std::shared_ptr<AppEventPack> pack);
    static int SetEventPolicy(const std::string& name, const std::map<std::string, std::string>& configMap);
    static int SetEventPolicy(const std::string& name, const std::map<uint8_t, uint32_t>& configMap);
};

class AppEventObserverFacade {
public:
    static int64_t AddProcessor(const std::string& name, const HiAppEvent::ReportConfig& conf = {});
    static int RemoveObserver(int64_t observerSeq);
    static int RemoveObserver(const std::string& observerName);
    static int Load(const std::string& moduleName);
    static void HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events);
    static void HandleTimeout();
    static void HandleBackground();
    static int64_t AddWatcher(std::shared_ptr<AppEventWatcher> watcher);
    static void SubmitTaskToFFRTQueue(std::function<void()>&& task, const std::string& taskName);
    static int RegisterProcessor(const std::string& name, std::shared_ptr<HiAppEvent::AppEventProcessor> processor);
    static int UnregisterProcessor(const std::string& name);
    static int SetReportConfig(int64_t observerSeq, const HiAppEvent::ReportConfig& config);
    static int GetReportConfig(int64_t observerSeq, HiAppEvent::ReportConfig& config);
};

class AppEventUserInfoFacade {
public:
    static int SetUserId(const std::string& name, const std::string& value);
    static int GetUserId(const std::string& name, std::string& out);
    static int RemoveUserId(const std::string& name);
    static int SetUserProperty(const std::string& name, const std::string& value);
    static int GetUserProperty(const std::string& name, std::string& out);
    static int RemoveUserProperty(const std::string& name);
    static int64_t GetUserIdVersion();
    static int64_t GetUserPropertyVersion();
};

class AppEventStoreFacade {
public:
    static int QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, int row = 0);
    static bool DeleteData(int64_t observerSeq, const std::vector<int64_t>& eventSeqs);
    static int DeleteEventMapping(int64_t observerSeq, const std::vector<int64_t>& eventSeqs);
    static int64_t QueryObserverSeq(const std::string& name);
    static int QueryObserverSeqs(const std::string& name, std::vector<int64_t>& observerSeqs);
    static int TakeEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t size);
    static int InitDbStore();
    static int DestroyDbStore();
    static void CheckStorageSpace();
    static void ClearData(const std::string& dir);
    static bool IsStorageSpaceFull(const std::string& dir, uint64_t maxSize);
    static void ReleaseSomeStorageSpace(const std::string& dir, uint64_t maxSize);
};

class AppEventVerifyFacade {
public:
    static int VerifyTheAppEvent(std::shared_ptr<AppEventPack> pack);
    static int VerifyTheCustomEventParams(std::shared_ptr<AppEventPack> pack);
    static int VerifyTheReportConfig(HiAppEvent::ReportConfig& config);
    static bool VerifyIsApp();
    static bool VerifyIsValidAppId(const std::string& name);
    static bool VerifyIsValidBatchReport(int count);
    static bool VerifyIsValidConfigId(int configId);
    static bool VerifyIsValidConfigNameLength(const std::string& configName);
    static bool VerifyIsValidCustomConfig(const std::string& name, const std::string& value);
    static bool VerifyIsValidCustomConfigsNum(size_t num);
    static bool VerifyIsValidDomain(const std::string& domain);
    static bool VerifyIsValidEventConfig(const HiAppEvent::EventConfig& eventCfg);
    static bool VerifyIsValidEventName(const std::string& eventName);
    static bool VerifyIsValidEventType(int eventType);
    static bool VerifyIsValidPeriodReport(int timeout);
    static bool VerifyIsValidProcessorName(const std::string& name);
    static bool VerifyIsValidRouteInfo(const std::string& name);
    static bool VerifyIsValidUserIdName(const std::string& name);
    static bool VerifyIsValidUserIdValue(const std::string& value);
    static bool VerifyIsValidUserPropName(const std::string& name);
    static bool VerifyIsValidUserPropValue(const std::string& value);
    static bool VerifyIsValidWatcherName(const std::string& watcherName);
};

class AppEventUtilityFacade {
public:
    static void WriteApiEndEventAsync(const std::string& apiName, uint64_t beginTime, int result, int errCode);
    static int WriteApiEndMetric(const std::string& apiName, uint64_t beginTime, int result, int errCode);
    static std::string GetDate();
    static uint64_t GetElapsedMilliSecondsSinceBoot();
    static bool ForceCreateDirectory(const std::string& dir);
    static std::string GetFilePathByDir(const std::string& dir, const std::string& fileName);
    static bool IsFileExists(const std::string& file);
    static bool SaveStringToFile(const std::string& file, const std::string& content);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FACADE_H
