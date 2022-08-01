/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "napi_hiappevent_write.h"

#include <memory>
#include <new>

#include "hiappevent_write.h"
#include "napi_util.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventWrite {
namespace {
constexpr size_t ERR_INDEX = 0;
constexpr size_t VALUE_INDEX = 1;
constexpr size_t RESULT_SIZE = 2;
}

void Write(const napi_env env, HiAppEventAsyncContext* asyncContext)
{
    napi_value resource = NapiUtil::CreateString(env, "NapiHiAppEventWriter");
    napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            if (asyncContext->appEventPack != nullptr && asyncContext->result >= 0) {
                WriterEvent(asyncContext->appEventPack);
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            napi_value results[RESULT_SIZE] = { 0 };
            if (asyncContext->result == 0) {
                results[ERR_INDEX] = NapiUtil::CreateUndefined(env);
                results[VALUE_INDEX] = NapiUtil::CreateInt32(env, asyncContext->result);
            } else {
                results[ERR_INDEX] = NapiUtil::CreateObject(env, "code",
                    NapiUtil::CreateInt32(env, asyncContext->result));
                results[VALUE_INDEX] = NapiUtil::CreateUndefined(env);
            }

            if (asyncContext->deferred != nullptr) { // promise
                if (asyncContext->result == 0) {
                    napi_resolve_deferred(env, asyncContext->deferred, results[VALUE_INDEX]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, results[ERR_INDEX]);
                }
            } else { // callback
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callback, &callback);
                napi_value retValue = nullptr;
                napi_call_function(env, nullptr, callback, RESULT_SIZE, results, &retValue);
                napi_delete_reference(env, asyncContext->callback);
            }
            napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->asyncWork);
    napi_queue_async_work(env, asyncContext->asyncWork);
}
} // namespace NapiHiAppEventWrite
} // namespace HiviewDFX
} // namespace OHOS
