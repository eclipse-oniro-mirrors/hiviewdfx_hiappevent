/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef APPEVENT_WATCHER_IMPL_H
#define APPEVENT_WATCHER_IMPL_H

#include <vector>
#include <functional>

#include "appevent_packageholder_impl.h"
#include "app_event_watcher.h"
#include "cj_lambda.h"
#include "hiappevent_base.h"
#include "json/json.h"
#include "native/ffi_remote_data.h"

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
constexpr uint8_t TYPE_INT = 0;
constexpr uint8_t TYPE_FLOAT = 1;
constexpr uint8_t TYPE_STRING = 2;
constexpr uint8_t TYPE_BOOL = 3;
constexpr uint8_t TYPE_ARRINT = 4;
constexpr uint8_t TYPE_ARRFLOAT = 5;
constexpr uint8_t TYPE_ARRSTRING = 6;
constexpr uint8_t TYPE_ARRBOOL = 7;

struct OnTriggerContext {
    ~OnTriggerContext();
    int row = 0;
    int size = 0;
    std::function<void(int, int, int64_t)> onTrigger = nullptr;
    AppEventPackageHolderImpl* holder = nullptr;
};

struct OnReceiveContext {
    ~OnReceiveContext();
    std::function<void(char*, CArrRetAppEventGroup)> onReceive = nullptr;
    std::string domain = "";
    std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>> events;
};

struct WatcherContext {
    ~WatcherContext();
    OnTriggerContext* triggerContext = nullptr;
    OnReceiveContext* receiveContext = nullptr;
};

class AppEventWatcherImpl : public HiviewDFX::AppEventWatcher {
public:
    AppEventWatcherImpl(
        const std::string& name,
        const std::vector<HiviewDFX::AppEventFilter>& filters,
        HiviewDFX::TriggerCondition cond);
    ~AppEventWatcherImpl() override;
    void InitTrigger(void (*callbackRef)(int, int, int64_t));
    void InitHolder(AppEventPackageHolderImpl* holder);
    void InitReceiver(void (*callbackRef)(char*, CArrRetAppEventGroup));
    void OnEvents(const std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>>& events) override;
    bool IsRealTimeEvent(std::shared_ptr<OHOS::HiviewDFX::AppEventPack> event) override;
protected:
    void OnTrigger(const HiviewDFX::TriggerCondition& triggerCond) override;
private:
    WatcherContext* context_;
};
} // HiAppEvent
} // CJSystemapi
} // OHOS

#endif
