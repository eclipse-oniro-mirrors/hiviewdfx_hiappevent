/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "app_event_observer_mgr.h"
#include "app_event_store.h"
#include "app_event_util.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "napi_env_watcher_manager.h"
#include "napi_util.h"
#include "uv.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiWatcher"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t CALLBACK_PARAM_NUM = 3;
constexpr size_t RECEIVE_PARAM_NUM = 2;

void DeleteEventMappingAsync(int64_t observerSeq, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::vector<int64_t> eventSeqs;
    for (const auto& event : events) {
        eventSeqs.emplace_back(event->GetSeq());
    }
    AppEventObserverMgr::GetInstance().SubmitTaskToFFRTQueue([observerSeq, eventSeqs]() {
        if (!AppEventStore::GetInstance().DeleteData(observerSeq, eventSeqs)) {
            HILOG_ERROR(LOG_CORE, "failed to delete mapping data, seq=%{public}" PRId64 ", event num=%{public}zu",
                observerSeq, eventSeqs.size());
        }
        }, "appevent_del_map");
}
}
OnTriggerContext::~OnTriggerContext()
{
    HILOG_DEBUG(LOG_CORE, "start to destroy OnTriggerContext object");
    auto task = [env = env, onTrigger = onTrigger, holder = holder] () {
        if (onTrigger != nullptr) {
            napi_delete_reference(env, onTrigger);
        }
        if (holder != nullptr) {
            napi_delete_reference(env, holder);
        }
    };
    if (napi_send_event(env, task, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
    }
}

OnReceiveContext::~OnReceiveContext()
{
    HILOG_DEBUG(LOG_CORE, "start to destroy OnReceiveContext object");
    auto task = [env = env, onReceive = onReceive] () {
        if (onReceive != nullptr) {
            napi_delete_reference(env, onReceive);
        }
    };
    if (napi_send_event(env, task, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
    }
}

NapiAppEventWatcher::NapiAppEventWatcher(
    const std::string& name,
    const std::vector<AppEventFilter>& filters,
    TriggerCondition cond)
    : AppEventWatcher(name, filters, cond)
{}

NapiAppEventWatcher::~NapiAppEventWatcher()
{
    HILOG_DEBUG(LOG_CORE, "start to destroy NapiAppEventWatcher object");
    EnvWatcherManager::GetInstance().RemoveEnvWatcherRecord(this);
}

void NapiAppEventWatcher::DeleteWatcherContext()
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    triggerContext_ = nullptr;
    receiveContext_ = nullptr;
}

void NapiAppEventWatcher::InitHolder(const napi_env env, const napi_value holder)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (triggerContext_ == nullptr) {
        triggerContext_ = std::make_shared<OnTriggerContext>();
    }
    triggerContext_->env = env;
    triggerContext_->holder = NapiUtil::CreateReference(env, holder);
}

void NapiAppEventWatcher::OnTrigger(const TriggerCondition& triggerCond)
{
    HILOG_DEBUG(LOG_CORE, "onTrigger start");
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (triggerContext_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "onTrigger context is null");
        return;
    }
    auto onTriggerWork = [row = triggerCond.row, size = triggerCond.size, triggerContext = triggerContext_] () {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(triggerContext->env, &scope);
        if (scope == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to open handle scope");
            return;
        }
        napi_value callback = NapiUtil::GetReferenceValue(triggerContext->env, triggerContext->onTrigger);
        if (callback == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to get callback from the context");
            napi_close_handle_scope(triggerContext->env, scope);
            return;
        }
        napi_value argv[CALLBACK_PARAM_NUM] = {
            NapiUtil::CreateInt32(triggerContext->env, row),
            NapiUtil::CreateInt32(triggerContext->env, size),
            NapiUtil::GetReferenceValue(triggerContext->env, triggerContext->holder)
        };
        napi_value ret = nullptr;
        if (napi_call_function(triggerContext->env, nullptr, callback, CALLBACK_PARAM_NUM, argv, &ret) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "failed to call onTrigger function");
        }
        napi_close_handle_scope(triggerContext->env, scope);
    };
    if (napi_send_event(triggerContext_->env, onTriggerWork, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
    }
}

void NapiAppEventWatcher::InitTrigger(const napi_env env, const napi_value triggerFunc)
{
    HILOG_DEBUG(LOG_CORE, "start to init OnTrigger");
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (triggerContext_ == nullptr) {
        triggerContext_ = std::make_shared<OnTriggerContext>();
    }
    triggerContext_->env = env;
    triggerContext_->onTrigger = NapiUtil::CreateReference(env, triggerFunc);
}

void NapiAppEventWatcher::InitReceiver(const napi_env env, const napi_value receiveFunc)
{
    HILOG_DEBUG(LOG_CORE, "start to init onReceive");
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (receiveContext_ == nullptr) {
        receiveContext_ = std::make_shared<OnReceiveContext>();
    }
    receiveContext_->env = env;
    receiveContext_->onReceive = NapiUtil::CreateReference(env, receiveFunc);
}

void NapiAppEventWatcher::OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    HILOG_DEBUG(LOG_CORE, "onEvents start, seq=%{public}" PRId64 ", event num=%{public}zu", GetSeq(), events.size());
    std::lock_guard<std::mutex> lockGuard(mutex_);
    if (receiveContext_ == nullptr || events.empty()) {
        HILOG_ERROR(LOG_CORE, "onReceive context is null or events is empty");
        return;
    }
    auto domain = events[0]->GetDomain();
    auto observerSeq = GetSeq();
    std::string watcherName = GetName();
    auto onReceiveWork = [watcherName, events, domain, observerSeq, receiveContext = receiveContext_] () {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(receiveContext->env, &scope);
        if (scope == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to open handle scope");
            return;
        }
        napi_value callback = NapiUtil::GetReferenceValue(receiveContext->env, receiveContext->onReceive);
        if (callback == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to get callback from the context");
            napi_close_handle_scope(receiveContext->env, scope);
            return;
        }
        napi_value argv[RECEIVE_PARAM_NUM] = {
            NapiUtil::CreateString(receiveContext->env, domain),
            NapiUtil::CreateEventGroups(receiveContext->env, events)
        };
        napi_value ret = nullptr;
        if (napi_call_function(receiveContext->env, nullptr, callback, RECEIVE_PARAM_NUM, argv, &ret) == napi_ok) {
            AppEventUtil::ReportAppEventReceive(events, watcherName, "onReceive");
            DeleteEventMappingAsync(observerSeq, events);
        } else {
            HILOG_ERROR(LOG_CORE, "failed to call onReceive function");
        }
        napi_close_handle_scope(receiveContext->env, scope);
    };
    if (napi_send_event(receiveContext_->env, onReceiveWork, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
    }
}

bool NapiAppEventWatcher::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    std::lock_guard<std::mutex> lockGuard(mutex_);
    return (receiveContext_ != nullptr);
}
} // namespace HiviewDFX
} // namespace OHOS
