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

#ifndef HIAPPEVENT_IMPL_H
#define HIAPPEVENT_IMPL_H

#include <string>
#include <memory>

#include "app_event_observer.h"
#include "common.h"
#include "hiappevent_base.h"

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
class HiAppEventImpl {
public:
    std::shared_ptr<HiviewDFX::AppEventPack> appEventPack_;
    static int Configure(bool disable, const std::string& maxStorage);
    static int Write(std::shared_ptr<HiviewDFX::AppEventPack> eventPack);
    static int64_t AddProcessor(const OHOS::HiviewDFX::HiAppEvent::ReportConfig& conf);
    static int RemoveProcessor(int64_t processorId);
    static int SetUserId(const std::string& name, const std::string& value);
    static std::tuple<int, std::string> GetUserId(const std::string& name);
    static int SetUserProperty(const std::string& name, const std::string& value);
    static std::tuple<int, std::string> GetUserProperty(const std::string& name);
    static void ClearData();
    static std::tuple<int, int64_t> addWatcher(const std::string& name,
                                                const std::vector<HiviewDFX::HiAppEvent::AppEventFilter>& filters,
                                                const HiviewDFX::HiAppEvent::TriggerCondition& cond,
                                                void (*callbackOnTriggerRef)(int, int, int64_t),
                                                void (*callbackOnReceiveRef)(char*, CArrRetAppEventGroup));
    static void removeWatcher(const std::string& name);
    static int Load(const std::string& moduleName);
};
} // HiAppEvent
} // CJSystemapi
} // OHOS

#endif