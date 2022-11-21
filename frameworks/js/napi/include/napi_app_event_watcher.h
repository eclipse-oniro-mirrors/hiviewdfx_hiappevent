/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_APP_EVENT_WATCHER_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_APP_EVENT_WATCHER_H

#include "app_event_watcher.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
struct OnTriggerContext {
    OnTriggerContext();
    ~OnTriggerContext();
    napi_env env;
    napi_ref onTrigger;
    napi_ref holder;
    int row;
    int size;
};

class NapiAppEventWatcher : public AppEventWatcher {
public:
    NapiAppEventWatcher(const std::string& name, const std::map<std::string, unsigned int>& filters,
        TriggerCondition cond);
    ~NapiAppEventWatcher();
    void OnTrigger(int row, int size) override;
    void InitTrigger(const napi_env env, const napi_value trigger);
    void InitHolder(const napi_env env, const napi_value holder);

private:
    OnTriggerContext* context_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_APP_EVENT_WATCHER_H
