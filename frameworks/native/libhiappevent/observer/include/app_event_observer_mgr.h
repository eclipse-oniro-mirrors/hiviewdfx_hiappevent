/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace HiviewDFX {
class AppEventHandler;
namespace HiAppEvent {
class AppStateCallback;
}
using HiAppEvent::AppEventObserver;
using HiAppEvent::AppStateCallback;
using HiAppEvent::ReportConfig;

class AppEventObserverMgr {
public:
    AppEventObserverMgr();
    ~AppEventObserverMgr();
    static AppEventObserverMgr& GetInstance();

    int64_t RegisterObserver(std::shared_ptr<AppEventObserver> observer);
    int64_t RegisterObserver(const std::string& observerName, const ReportConfig& config = {});
    int UnregisterObserver(int64_t observerSeq);
    int UnregisterObserver(const std::string& observerName);
    void HandleEvent(std::shared_ptr<AppEventPack> event);
    void HandleTimeout();
    void HandleBackground();
    void HandleClearUp();

    int SetReportConfig(int64_t observerSeq, const ReportConfig& config);
    int GetReportConfig(int64_t observerSeq, ReportConfig& config);

private:
    void CreateEventHandler();
    void DestroyEventHandler();
    void RegisterAppStateCallback();
    void UnregisterAppStateCallback();

private:
    std::unordered_map<int64_t, std::shared_ptr<AppEventObserver>> observers_;
    std::unordered_map<int64_t, ReportConfig> configs_;
    std::shared_ptr<AppEventHandler> handler_;
    std::shared_ptr<AppStateCallback> appStateCallback_;
    std::mutex observerMutex_;

private:
    static std::mutex instanceMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_OBSERVER_APP_EVENT_OBSERVER_MGR_H
