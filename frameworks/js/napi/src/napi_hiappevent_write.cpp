/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <map>

#include "hiappevent_base.h"
#include "hiappevent_facade.h"
#include "napi_error.h"
#include "napi_util.h"

using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventWrite {
namespace {
constexpr size_t ERR_INDEX = 0;
constexpr size_t VALUE_INDEX = 1;
constexpr size_t RESULT_SIZE = 2;

napi_value BuildErrorByResult(const napi_env env, int result)
{
    const std::map<int, int> errMap = {
        { ErrorCode::ERROR_INVALID_EVENT_NAME, NapiError::ERR_INVALID_NAME },
        { ErrorCode::ERROR_INVALID_EVENT_DOMAIN, NapiError::ERR_INVALID_DOMAIN },
        { ErrorCode::ERROR_HIAPPEVENT_DISABLE, NapiError::ERR_DISABLE },
        { ErrorCode::ERROR_INVALID_PARAM_NAME, NapiError::ERR_INVALID_KEY },
        { ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH, NapiError::ERR_INVALID_STR_LEN },
        { ErrorCode::ERROR_INVALID_PARAM_NUM, NapiError::ERR_INVALID_PARAM_NUM },
        { ErrorCode::ERROR_INVALID_LIST_PARAM_SIZE, NapiError::ERR_INVALID_ARR_LEN },
        { ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM, NapiError::ERR_INVALID_CUSTOM_PARAM_NUM },
    };
    auto it = errMap.find(result);
    return  it == errMap.end() ? NapiUtil::CreateNull(env) :
        NapiUtil::CreateError(env, it->second, NapiError::GetErrorMsg(it->second));
}

void WriteExecute(napi_env env, void* data)
{
    HiAppEventAsyncContext* asyncContext = static_cast<HiAppEventAsyncContext*>(data);
    if (asyncContext->appEventPack != nullptr && asyncContext->result >= 0) {
        AppEventWriteFacade::FacadeWriteEvent(asyncContext->appEventPack);
    }
}

void WriteComplete(napi_env env, napi_status status, void* data)
{
    HiAppEventAsyncContext* asyncContext = static_cast<HiAppEventAsyncContext*>(data);
    napi_value results[RESULT_SIZE] = { 0 };
    if (asyncContext->result == 0) {
        results[ERR_INDEX] = NapiUtil::CreateNull(env);
        results[VALUE_INDEX] = NapiUtil::CreateInt32(env, asyncContext->result);
    } else {
        if (asyncContext->isV9) {
            results[ERR_INDEX] = BuildErrorByResult(env, asyncContext->result);
        } else {
            results[ERR_INDEX] = NapiUtil::CreateObject(env, "code",
                NapiUtil::CreateInt32(env, asyncContext->result));
        }
        results[VALUE_INDEX] = NapiUtil::CreateNull(env);
    }

    if (asyncContext->deferred != nullptr) { // promise
        if (asyncContext->result == 0) {
            (void)napi_resolve_deferred(env, asyncContext->deferred, results[VALUE_INDEX]);
        } else {
            (void)napi_reject_deferred(env, asyncContext->deferred, results[ERR_INDEX]);
        }
    } else { // callback
        napi_value callback = nullptr;
        (void)napi_get_reference_value(env, asyncContext->callback, &callback);
        napi_value retValue = nullptr;
        (void)napi_call_function(env, nullptr, callback, RESULT_SIZE, results, &retValue);
        (void)napi_delete_reference(env, asyncContext->callback);
    }
    (void)napi_delete_async_work(env, asyncContext->asyncWork);
    delete asyncContext;
}
}

void Write(const napi_env env, std::unique_ptr<HiAppEventAsyncContext> asyncContext)
{
    HiAppEventAsyncContext* data = asyncContext.release();
    napi_value resource = NapiUtil::CreateString(env, "NapiHiAppEventWriter");
    auto createStatus = napi_create_async_work(env, nullptr, resource, WriteExecute, WriteComplete,
        data, &data->asyncWork);
    if (createStatus != napi_ok || napi_queue_async_work_with_qos(env, data->asyncWork, napi_qos_default) != napi_ok) {
        if (createStatus == napi_ok) {
            (void)napi_delete_async_work(env, data->asyncWork);
        }
        napi_value results[RESULT_SIZE] = { 0 };
        if (data->isV9) {
            results[ERR_INDEX] = NapiUtil::CreateError(env, NapiError::ERR_PARAM,
                "failed to submit the write task.");
        } else {
            results[ERR_INDEX] = NapiUtil::CreateObject(env, "code",
                NapiUtil::CreateInt32(env, NapiError::ERR_PARAM));
        }
        results[VALUE_INDEX] = NapiUtil::CreateNull(env);
        if (data->deferred != nullptr) {
            (void)napi_reject_deferred(env, data->deferred, results[ERR_INDEX]);
        } else {
            napi_value callback = nullptr;
            (void)napi_get_reference_value(env, data->callback, &callback);
            napi_value retValue = nullptr;
            (void)napi_call_function(env, nullptr, callback, RESULT_SIZE, results, &retValue);
            (void)napi_delete_reference(env, data->callback);
        }
        delete data;
    }
}

void SetEventParam(const napi_env env, std::unique_ptr<HiAppEventAsyncContext> asyncContext)
{
    HiAppEventAsyncContext* data = asyncContext.release();
    napi_value resource = NapiUtil::CreateString(env, "NapiHiAppEventSetEventParam");
    auto createStatus = napi_create_async_work(env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventAsyncContext* asyncContext = static_cast<HiAppEventAsyncContext*>(data);
            if (asyncContext->appEventPack != nullptr && asyncContext->result == 0) {
                if (auto ret = AppEventWriteFacade::FacadeSetEventParam(asyncContext->appEventPack); ret > 0) {
                    asyncContext->result = ret;
                }
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventAsyncContext* asyncContext = static_cast<HiAppEventAsyncContext*>(data);
            napi_value results[RESULT_SIZE] = { 0 };
            if (asyncContext->deferred != nullptr) { // promise
                if (asyncContext->result == 0) {
                    results[ERR_INDEX] = NapiUtil::CreateNull(env);
                    results[VALUE_INDEX] = NapiUtil::CreateInt32(env, asyncContext->result);
                    (void)napi_resolve_deferred(env, asyncContext->deferred, results[VALUE_INDEX]);
                } else {
                    results[ERR_INDEX] = BuildErrorByResult(env, asyncContext->result);
                    results[VALUE_INDEX] = NapiUtil::CreateNull(env);
                    (void)napi_reject_deferred(env, asyncContext->deferred, results[ERR_INDEX]);
                }
            }
            (void)napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        data, &data->asyncWork);
    if (createStatus != napi_ok || napi_queue_async_work_with_qos(env, data->asyncWork, napi_qos_default) != napi_ok) {
        if (createStatus == napi_ok) {
            (void)napi_delete_async_work(env, data->asyncWork);
        }
        if (data->deferred != nullptr) {
            (void)napi_reject_deferred(env, data->deferred, NapiUtil::CreateError(env, NapiError::ERR_PARAM,
                "failed to submit the setEventParam task."));
        }
        delete data;
    }
}
} // namespace NapiHiAppEventWrite
} // namespace HiviewDFX
} // namespace OHOS
