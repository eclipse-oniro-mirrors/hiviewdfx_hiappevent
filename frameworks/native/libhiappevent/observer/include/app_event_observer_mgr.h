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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_OBSERVER_APP_EVENT_OBSERVER_MGR_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_OBSERVER_APP_EVENT_OBSERVER_MGR_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "app_event_observer.h"
#include "app_event_processor.h"
#include "app_event_processor_proxy.h"
#include "app_event_watcher.h"
#include "ffrt.h"
#include "module_loader.h"
#include "nocopyable.h"

namespace OHOS {
namespace HiviewDFX {
class OsEventListener;
namespace HiAppEvent {
class AppStateCallback;
}
using HiAppEvent::AppEventObserver;
using HiAppEvent::AppStateCallback;
using HiAppEvent::AppEventProcessor;
using HiAppEvent::ModuleLoader;
using HiAppEvent::ReportConfig;
using HiAppEvent::AppEventProcessorProxy;

class AppEventObserverMgr : public NoCopyable {
public:
    static AppEventObserverMgr& GetInstance();

    int64_t AddWatcher(std::shared_ptr<AppEventWatcher> watcher);
    int64_t AddProcessor(const std::string& name, const ReportConfig& config = {});
    int RemoveObserver(int64_t observerSeq);
    int RemoveObserver(const std::string& observerName);
    int Load(const std::string& moduleName);
    int RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor);
    int UnregisterProcessor(const std::string& name);
    void HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events);
    void HandleTimeout();
    void HandleBackground();
    void HandleClearUp();
    int SetReportConfig(int64_t observerSeq, const ReportConfig& config);
    int GetReportConfig(int64_t observerSeq, ReportConfig& config);
    void SubmitTaskToFFRTQueue(std::function<void()>&& task, const std::string& taskName);

private:
    AppEventObserverMgr();
    ~AppEventObserverMgr();
    int64_t AddProcessorWithTimeLimited(const std::string& name, int64_t hashCode,
        std::shared_ptr<AppEventProcessorProxy> processor);
    void SendTimeoutTask();
    void SendRefreshFreeSizeTask();
    void RegisterAppStateCallback();
    void UnregisterAppStateCallback();
    bool InitWatcherFromListener(std::shared_ptr<AppEventWatcher> watcher, bool sendFlag);
    void UnregisterOsEventListener();
    void InitWatchers();
    void InitWatcherFromCache(std::shared_ptr<AppEventWatcher> watcher, bool& isExist);
    int64_t GetSeqFromWatchers(const std::string& name, std::string& filters);
    int64_t GetSeqFromProcessors(const std::string& name, int64_t hashCode);
    std::vector<std::shared_ptr<AppEventObserver>> GetObservers();
    void DeleteWatcher(int64_t observerSeq);
    void DeleteProcessor(int64_t observerSeq);
    bool IsExistInWatchers(int64_t observerSeq);
    bool IsExistInProcessors(int64_t observerSeq);

private:
    std::unique_ptr<ModuleLoader> moduleLoader_; // moduleLoader_ must declared before observers_, or lead to crash
    std::unordered_map<int64_t, std::shared_ptr<AppEventWatcher>> watchers_;
    std::unordered_map<int64_t, std::shared_ptr<AppEventProcessorProxy>> processors_;
    std::mutex watcherMutex_;
    std::mutex processorMutex_;
    std::shared_ptr<ffrt::queue> queue_ = nullptr;
    std::shared_ptr<AppStateCallback> appStateCallback_;
    std::shared_ptr<OsEventListener> listener_ = nullptr;
    bool isTimeoutTaskExist_ = false;
    std::mutex isTimeoutTaskExistMutex_;
    std::atomic<bool> isFirstAddProcessor_ = true;
    std::atomic<int> isDbInit_ = false;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_OBSERVER_APP_EVENT_OBSERVER_MGR_H
