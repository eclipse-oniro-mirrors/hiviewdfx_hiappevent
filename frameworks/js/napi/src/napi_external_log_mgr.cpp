/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "napi_external_log_mgr.h"

#include "hilog/log.h"
#include "napi_external_log_container.h"
#include "napi_external_log_wrapper.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "NapiExtLogMgr"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t CALLBACK_PARAM_NUM = 1;
}

OnCapacityReachedContext::~OnCapacityReachedContext()
{
    auto task = [env = env, onCapacityReached = onCapacityReached, logManagerRef = logManagerRef]() {
        if (onCapacityReached != nullptr) {
            napi_delete_reference(env, onCapacityReached);
        }
        if (logManagerRef != nullptr) {
            napi_delete_reference(env, logManagerRef);
        }
    };
    napi_send_event(env, task, napi_eprio_high);
}

NapiExternalLogMgr::~NapiExternalLogMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    context_ = nullptr;
}

void NapiExternalLogMgr::InitCallback(napi_env env, napi_value logManagerObj)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (context_ == nullptr) {
        context_ = std::make_shared<OnCapacityReachedContext>();
    }
    context_->env = env;
    context_->logManagerRef = NapiUtil::CreateReference(env, logManagerObj);

    napi_value callback = NapiUtil::GetProperty(env, logManagerObj, "onCapacityReached");
    if (callback != nullptr && NapiUtil::IsFunction(env, callback)) {
        context_->onCapacityReached = NapiUtil::CreateReference(env, callback);
    }
}

void NapiExternalLogMgr::OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (context_ == nullptr || context_->onCapacityReached == nullptr) {
        return;
    }

    auto work = [logInfos, context = context_]() {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(context->env, &scope);

        // Create container JS object
        napi_value containerConstructor = NapiUtil::GetReferenceValue(context->env,
            NapiExternalLogContainer::constructor_);
        if (containerConstructor == nullptr) {
            napi_close_handle_scope(context->env, scope);
            return;
        }
        napi_value containerObj = nullptr;
        if (napi_new_instance(context->env, containerConstructor, 0, nullptr, &containerObj) != napi_ok
            || containerObj == nullptr) {
            napi_close_handle_scope(context->env, scope);
            return;
        }
        auto* container = new NapiExternalLogContainer(logInfos);
        napi_wrap(context->env, containerObj, container,
            [](napi_env env, void* data, void* hint) {
                delete static_cast<NapiExternalLogContainer*>(data);
            },
            nullptr, nullptr);

        // Call onCapacityReached(container)
        napi_value callback = NapiUtil::GetReferenceValue(context->env, context->onCapacityReached);
        napi_value argv[CALLBACK_PARAM_NUM] = { containerObj };
        napi_value ret = nullptr;
        napi_call_function(context->env, nullptr, callback, CALLBACK_PARAM_NUM, argv, &ret);

        napi_close_handle_scope(context->env, scope);
    };
    napi_send_event(context_->env, work, napi_eprio_high);
}

} // namespace HiviewDFX
} // namespace OHOS