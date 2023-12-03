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
constexpr size_t RECEIVE_PARAM_NUM = 2;

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

OnReceiveContext::OnReceiveContext()
{
    env = nullptr;
    onReceive = nullptr;
}

OnReceiveContext::~OnReceiveContext()
{
    if (onReceive != nullptr) {
        napi_delete_reference(env, onReceive);
    }
}

WatcherContext::WatcherContext()
{
    triggerContext = nullptr;
    receiveContext = nullptr;
}

WatcherContext::~WatcherContext()
{
    if (triggerContext != nullptr) {
        delete triggerContext;
    }
    if (receiveContext != nullptr) {
        delete receiveContext;
    }
}

NapiAppEventWatcher::NapiAppEventWatcher(
    const std::string& name,
    const std::vector<AppEventFilter>& filters,
    TriggerCondition cond)
    : AppEventWatcher(name, filters, cond), context_(nullptr)
{}

NapiAppEventWatcher::~NapiAppEventWatcher()
{
    HiLog::Debug(LABEL, "start to destroy NapiAppEventWatcher object");
    if (context_ == nullptr) {
        return;
    }
    napi_env env = nullptr;
    if (context_->receiveContext != nullptr) {
        env = context_->receiveContext->env;
    } else if (context_->triggerContext != nullptr) {
        env = context_->triggerContext->env;
    } else {
        return;
    }
    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    uv_work_t* work = new(std::nothrow) uv_work_t();
    work->data = (void*)context_;
    uv_queue_work_with_qos(
        loop,
        work,
        [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            WatcherContext* context = (WatcherContext*)work->data;
            HiLog::Debug(LABEL, "start to destroy WatcherContext object");
            delete context;
            SafeDeleteWork(work);
        },
        uv_qos_default);
}

void NapiAppEventWatcher::InitHolder(const napi_env env, const napi_value holder)
{
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new(std::nothrow) OnTriggerContext();
    }
    context_->triggerContext->env = env;
    context_->triggerContext->holder = NapiUtil::CreateReference(env, holder);
}

void NapiAppEventWatcher::OnTrigger(const TriggerCondition& triggerCond)
{
    HiLog::Debug(LABEL, "onTrigger start");
    if (context_ == nullptr || context_->triggerContext == nullptr) {
        HiLog::Error(LABEL, "onTrigger context is null");
        return;
    }
    context_->triggerContext->row = triggerCond.row;
    context_->triggerContext->size = triggerCond.size;

    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context_->triggerContext->env, &loop);
    uv_work_t* work = new(std::nothrow) uv_work_t();
    work->data = (void*)context_->triggerContext;
    uv_queue_work_with_qos(
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
        },
        uv_qos_default);
}

void NapiAppEventWatcher::InitTrigger(const napi_env env, const napi_value triggerFunc)
{
    HiLog::Debug(LABEL, "start to init OnTrigger");
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new(std::nothrow) OnTriggerContext();
    }
    context_->triggerContext->env = env;
    context_->triggerContext->onTrigger = NapiUtil::CreateReference(env, triggerFunc);
}

void NapiAppEventWatcher::InitReceiver(const napi_env env, const napi_value receiveFunc)
{
    HiLog::Debug(LABEL, "start to init onReceive");
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
    }
    if (context_->receiveContext == nullptr) {
        context_->receiveContext = new(std::nothrow) OnReceiveContext();
    }
    context_->receiveContext->env = env;
    context_->receiveContext->onReceive = NapiUtil::CreateReference(env, receiveFunc);
}

void NapiAppEventWatcher::OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    HiLog::Debug(LABEL, "onEvents start");
    if (context_ == nullptr || context_->receiveContext == nullptr) {
        HiLog::Error(LABEL, "onReceive context is null");
        return;
    }
    if (events.empty()) {
        return;
    }
    context_->receiveContext->domain = events[0]->GetDomain();
    context_->receiveContext->events = events;

    uv_loop_t* loop = nullptr;
    napi_get_uv_event_loop(context_->receiveContext->env, &loop);
    uv_work_t* work = new(std::nothrow) uv_work_t();
    work->data = (void*)context_->receiveContext;
    uv_queue_work_with_qos(
        loop,
        work,
        [] (uv_work_t* work) {},
        [] (uv_work_t* work, int status) {
            auto context = (OnReceiveContext*)work->data;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                HiLog::Error(LABEL, "failed to open handle scope");
                SafeDeleteWork(work);
                return;
            }
            napi_value callback = NapiUtil::GetReferenceValue(context->env, context->onReceive);
            if (callback == nullptr) {
                HiLog::Error(LABEL, "failed to get callback from the context");
                SafeDeleteWork(work);
                napi_close_handle_scope(context->env, scope);
                return;
            }
            napi_value argv[RECEIVE_PARAM_NUM] = {
                NapiUtil::CreateString(context->env, context->domain),
                NapiUtil::CreateEventGroups(context->env, context->events)
            };
            napi_value ret = nullptr;
            if (napi_call_function(context->env, nullptr, callback, RECEIVE_PARAM_NUM, argv, &ret) != napi_ok) {
                HiLog::Error(LABEL, "failed to call onReceive function");
            }
            napi_close_handle_scope(context->env, scope);
            SafeDeleteWork(work);
        },
        uv_qos_default);
}

bool NapiAppEventWatcher::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    return (context_ != nullptr && context_->receiveContext != nullptr);
}
} // namespace HiviewDFX
} // namespace OHOS
