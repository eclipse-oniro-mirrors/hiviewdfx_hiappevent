/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_WRITE_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_WRITE_H

#include <map>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
namespace NapiHiAppEventWrite {
struct HiAppEventAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::shared_ptr<AppEventPack> appEventPack;
    int result;
    bool isV9;

    HiAppEventAsyncContext(napi_env env)
    {
        this->env = env;
        this->asyncWork = nullptr;
        this->deferred = nullptr;
        this->callback = nullptr;
        this->appEventPack = nullptr;
        this->result = 0;
        this->isV9 = false;
    }
    ~HiAppEventAsyncContext() {}
};

struct HiAppEventConfigAsyncContext {
    napi_async_work asyncWork;
    napi_deferred deferred;
    std::map<std::string, std::string> eventConfigMap;
    std::string name;
    int result;

    HiAppEventConfigAsyncContext()
    {
        this->asyncWork = nullptr;
        this->deferred = nullptr;
        this->eventConfigMap.clear();
        this->name = "";
        this->result = 0;
    }
    ~HiAppEventConfigAsyncContext() {}
};

void Write(const napi_env env, HiAppEventAsyncContext* asyncContext);
void SetEventParam(const napi_env env, HiAppEventAsyncContext* asyncContext);
void SetEventConfig(const napi_env env, HiAppEventConfigAsyncContext* asyncContext);
} // namespace NapiHiAppEventWrite
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_WRITE_H
