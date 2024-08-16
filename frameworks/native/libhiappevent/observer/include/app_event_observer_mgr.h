/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "ffrt.h"
#include "module_loader.h"
#include "nocopyable.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventHandler;
class OsEventListener;
namespace HiAppEvent {
class AppStateCallback;
}
using HiAppEvent::AppEventObserver;
using HiAppEvent::AppStateCallback;
using HiAppEvent::AppEventProcessor;
using HiAppEvent::ModuleLoader;
using HiAppEvent::ReportConfig;

class AppEventObserverMgr : public NoCopyable {
public:
    static AppEventObserverMgr& GetInstance();

    void CreateEventHandler();
    void DestroyEventHandler();
    int64_t RegisterObserver(std::shared_ptr<AppEventObserver> observer);
    int64_t RegisterObserver(const std::string& observerName, const ReportConfig& config = {});
    int UnregisterObserver(int64_t observerSeq);
    int UnregisterObserver(const std::string& observerName);
    int Load(const std::string& moduleName);
    int RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor);
    int UnregisterProcessor(const std::string& name);
    void HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events);
    void HandleTimeout();
    void HandleBackground();
    void HandleClearUp();

    int SetReportConfig(int64_t observerSeq, const ReportConfig& config);
    int GetReportConfig(int64_t observerSeq, ReportConfig& config);

private:
    AppEventObserverMgr();
    ~AppEventObserverMgr();
    void SendEventToHandler();
    void RegisterAppStateCallback();
    void UnregisterAppStateCallback();
    int64_t InitObserver(std::shared_ptr<AppEventObserver> observer);
    bool InitObserverFromListener(std::shared_ptr<AppEventObserver> observer, bool sendFlag);
    void UnregisterOsEventListener();

private:
    std::unique_ptr<ModuleLoader> moduleLoader_; // moduleLoader_ must declared before observers_, or lead to crash
    std::unordered_map<int64_t, std::shared_ptr<AppEventObserver>> observers_;
    std::shared_ptr<AppEventHandler> handler_;
    std::shared_ptr<AppStateCallback> appStateCallback_;
    ffrt::mutex observerMutex_;
    std::shared_ptr<OsEventListener> listener_;
    ffrt::mutex listenerMutex_;
    bool hasHandleTimeout_ = false;

private:
    static ffrt::mutex instanceMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_OBSERVER_APP_EVENT_OBSERVER_MGR_H
