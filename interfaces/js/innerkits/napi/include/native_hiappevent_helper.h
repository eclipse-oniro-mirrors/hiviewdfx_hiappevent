/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NATIVE_HIAPPEVENT_HELPER_H
#define NATIVE_HIAPPEVENT_HELPER_H

#include <memory>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

struct HiAppEventAsyncContext {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback;
    std::shared_ptr<AppEventPack> appEventPack;
    int32_t result;
};

constexpr int EVENT_NAME_INDEX = 0;
constexpr int EVENT_TYPE_INDEX = 1;
constexpr int JSON_OBJECT_INDEX = 2;
constexpr int MIN_PARAM_NUM = 1;
constexpr int WRITE_FUNC_MIN_PARAM_NUM = 2;
constexpr int WRITE_FUNC_MAX_PARAM_NUM = 100;
constexpr int WRITE_JSON_FUNC_MIN_PARAM_NUM = 3;
constexpr int WRITE_JSON_FUNC_MAX_PARAM_NUM = 4;
constexpr int SUCCESS_FLAG = 0;

std::shared_ptr<AppEventPack> CreateEventPackFromNapiValue(napi_env env, napi_value nameValue, napi_value typeValue);
int BuildAppEventPack(napi_env env, const napi_value params[], const int paramEndIndex,
    std::shared_ptr<AppEventPack>& appEventPack);
int BuildAppEventPackFromObject(napi_env env, const napi_value object, std::shared_ptr<AppEventPack>& appEventPack);
int CheckWriteParamsType(napi_env env, const napi_value params[], const int paramNum);
int CheckWriteJsonParamsType(napi_env env, const napi_value params[], const int paramNum);
void AsyncWriteEvent(napi_env env, HiAppEventAsyncContext* asyncContext);
bool ConfigureFromNapiValue(napi_env env, const napi_value name, const napi_value value);
void SetStorageDir(napi_env env, napi_callback_info info);
} // HiviewDFX
} // OHOS
#endif // NATIVE_HIAPPEVENT_HELPER_H