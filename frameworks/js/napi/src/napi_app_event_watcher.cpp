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
#include "napi_app_event_watcher.h"

#include "hiappevent_base.h"
#include "hilog/log.h"
#include "napi_util.h"
#include "uv.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Napi_HiAppEvent_Watcher" };
constexpr size_t CALLBACK_PARAM_NUM = 3;

void SafeDeleteWork(uv_work_t* work)
{
    if (work != nullptr) {
        delete work;
    }
}
}
OnTriggerContext::OnTriggerContext()
{
    env = nullptr;
    onTrigger = nullptr;
    holder = nullptr;
    row = 0;
    size = 0;
}

OnTriggerContext::~OnTriggerContext()
{
    if (onTrigger != nullptr) {
        napi_delete_reference(env, onTrigger);
    }
    if (holder != nullptr) {
        napi_delete_reference(env, holder);
    }
}

NapiAppEventWatcher::NapiAppEventWatcher(const std::string& name, const std::map<std::string, unsigned int>& filters,
    TriggerCondition cond) : AppEventWatcher(name, filters, cond), context_(nullptr)
{}

NapiAppEventWatcher::~NapiAppEventWatcher()
{
    HiLog::Debug(LABEL, "start to destroy NapiAppEventWatcher object");
    if (context_ == nullptr) {
        return;
    }

    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context_->env, &loop);
    uv_work_t* work = new(std::nothrow) uv_work_t();
    work->data = (void*)context_;
    uv_queue_work(
        loop,
        work,
        [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            OnTriggerContext* context = (OnTriggerContext*)work->data;
            HiLog::Debug(LABEL, "start to destroy OnTriggerContext object");
            delete context;
            SafeDeleteWork(work);
        }
    );
}

void NapiAppEventWatcher::InitHolder(const napi_env env, const napi_value holder)
{
    if (context_ == nullptr) {
        context_ = new(std::nothrow) OnTriggerContext();
    }
    context_->env = env;
    context_->holder = NapiUtil::CreateReference(env, holder);
}

void NapiAppEventWatcher::OnTrigger(int row, int size)
{
    HiLog::Debug(LABEL, "onTrigger start");
    if (context_ == nullptr) {
        HiLog::Error(LABEL, "onTrigger context is null");
        return;
    }
    context_->row = row;
    context_->size = size;

    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context_->env, &loop);
    uv_work_t* work = new(std::nothrow) uv_work_t();
    work->data = (void*)context_;
    uv_queue_work(
        loop,
        work,
        [] (uv_work_t* work) {},
        [] (uv_work_t* work, int status) {
            auto context = (OnTriggerContext*)work->data;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                HiLog::Error(LABEL, "failed to open handle scope");
                SafeDeleteWork(work);
                return;
            }
            napi_value callback = NapiUtil::GetReferenceValue(context->env, context->onTrigger);
            if (callback == nullptr) {
                HiLog::Error(LABEL, "failed to get callback from the context");
                SafeDeleteWork(work);
                napi_close_handle_scope(context->env, scope);
                return;
            }
            napi_value argv[CALLBACK_PARAM_NUM] = {
                NapiUtil::CreateInt32(context->env, context->row),
                NapiUtil::CreateInt32(context->env, context->size),
                NapiUtil::GetReferenceValue(context->env, context->holder)
            };
            napi_value ret = nullptr;
            if (napi_call_function(context->env, nullptr, callback, CALLBACK_PARAM_NUM, argv, &ret) != napi_ok) {
                HiLog::Error(LABEL, "failed to call onTrigger function");
            }
            napi_close_handle_scope(context->env, scope);
            SafeDeleteWork(work);
        }
    );
}

void NapiAppEventWatcher::InitTrigger(const napi_env env, const napi_value triggerFunc)
{
    HiLog::Debug(LABEL, "start to init OnTrigger");
    if (context_ == nullptr) {
        context_ = new(std::nothrow) OnTriggerContext();
    }
    context_->env = env;
    context_->onTrigger = NapiUtil::CreateReference(env, triggerFunc);
}
} // namespace HiviewDFX
} // namespace OHOS
