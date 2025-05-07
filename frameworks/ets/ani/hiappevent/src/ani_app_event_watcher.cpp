/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_app_event_watcher.h"
#include "hiappevent_ani_util.h"

#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hiappevent_ffrt.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AniWatcher"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr ani_size REFERENCES_MAX_NUMBER = 16;

static ani_vm* GetAniVm(ani_env *env)
{
    ani_vm* vm = nullptr;
    if (ANI_OK != env->GetVM(&vm)) {
        HILOG_ERROR(LOG_CORE, "GetVM failed");
        return nullptr;
    }
    return vm;
}

static ani_env* GetAniEnv(ani_vm *vm)
{
    ani_env* env = nullptr;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        HILOG_ERROR(LOG_CORE, "GetEnv failed");
        return nullptr;
    }
    return env;
}

static ani_env* AttachAniEnv(ani_vm *vm)
{
    ani_env *workerEnv = nullptr;
    ani_options aniArgs {0, nullptr};
    if (ANI_OK != vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv)) {
        HILOG_ERROR(LOG_CORE, "Attach Env failed");
        return nullptr;
    }
    return workerEnv;
}

static void DetachAniEnv(ani_vm *vm)
{
    if (ANI_OK != vm->DetachCurrentThread()) {
        HILOG_ERROR(LOG_CORE, "Detach Env failed");
        return;
    }
}

void DeleteEventMappingAsync(int64_t observerSeq, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::vector<int64_t> eventSeqs;
    for (const auto& event : events) {
        eventSeqs.emplace_back(event->GetSeq());
    }
    HiAppEvent::Submit([observerSeq, eventSeqs]() {
        if (!AppEventStore::GetInstance().DeleteData(observerSeq, eventSeqs)) {
            HILOG_ERROR(LOG_CORE, "failed to delete mapping data, seq=%{public}" PRId64 ", event num=%{public}zu",
                observerSeq, eventSeqs.size());
        }
        }, {}, {}, ffrt::task_attr().name("appevent_del_map"));
}
}

OnTriggerContext::~OnTriggerContext()
{
    ani_env* env = GetAniEnv(vm);
    if (!HiAppEventAniUtil::IsRefUndefined(env, onTrigger)) {
        env->GlobalReference_Delete(onTrigger);
    }
    if (!HiAppEventAniUtil::IsRefUndefined(env, holder)) {
        env->GlobalReference_Delete(holder);
    }
}

OnReceiveContext::~OnReceiveContext()
{
    ani_env* env = GetAniEnv(vm);
    if (!HiAppEventAniUtil::IsRefUndefined(env, onReceive)) {
        env->GlobalReference_Delete(onReceive);
    }
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

AniAppEventWatcher::AniAppEventWatcher(
    const std::string& name,
    const std::vector<AppEventFilter>& filters,
    TriggerCondition cond)
    : AppEventWatcher(name, filters, cond), context_(nullptr)
{}

AniAppEventWatcher::~AniAppEventWatcher()
{
    HILOG_DEBUG(LOG_CORE, "start to destroy AniAppEventWatcher object");
    if (context_ != nullptr) {
        delete context_;
        return;
    }
}

void AniAppEventWatcher::InitHolder(ani_env *env, ani_ref holder)
{
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            return;
        }
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new(std::nothrow) OnTriggerContext();
        if (context_->triggerContext == nullptr) {
            return;
        }
    }
    context_->triggerContext->vm = GetAniVm(env);
    context_->triggerContext->holder = HiAppEventAniUtil::CreateGlobalReference(env, holder);
}

ani_status AniAppEventWatcher::AniSendEvent(const std::function<void()> cb, const std::string& name)
{
    if (cb == nullptr) {
        HILOG_WARN(LOG_CORE, "invalid callback function.");
        return ANI_INVALID_ARGS;
    }
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    if (!runner) {
        HILOG_WARN(LOG_CORE, "invalid main event runner.");
        return ANI_NOT_FOUND;
    }
    mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    mainHandler_->PostTask(cb, name, 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
    return ANI_OK;
}

void AniAppEventWatcher::OnTrigger(const TriggerCondition& triggerCond)
{
    HILOG_DEBUG(LOG_CORE, "onTrigger start");
    if (context_ == nullptr || context_->triggerContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "onTrigger context is null");
        return;
    }
    context_->triggerContext->row = triggerCond.row;
    context_->triggerContext->size = triggerCond.size;
    auto onTriggerWork = [triggerContext = context_->triggerContext, this] () {
        auto context = static_cast<OnTriggerContext*>(triggerContext);
        ani_size nr_refs = REFERENCES_MAX_NUMBER;
        ani_env* env = GetAniEnv(context->vm);
        env->CreateLocalScope(nr_refs);
        auto callback = context->onTrigger;
        if (HiAppEventAniUtil::IsRefUndefined(env, callback)) {
            HILOG_ERROR(LOG_CORE, "failed to get callback from the context");
            env->DestroyLocalScope();
            return;
        }
        std::vector<ani_ref> args = {
            HiAppEventAniUtil::CreateDouble(env, context->row),
            HiAppEventAniUtil::CreateDouble(env, context->size),
            context->holder
        };
        ani_ref ret {};
        if (ANI_OK != env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(callback),
            args.size(), args.data(), &ret)) {
            HILOG_ERROR(LOG_CORE, "failed to call onTrigger function");
        }
        env->DestroyLocalScope();
    };
    if (ANI_OK != AniSendEvent(onTriggerWork, "OnTrigger")) {
        HILOG_ERROR(LOG_CORE, "failed to send event OnTrigger.");
    }
}

void AniAppEventWatcher::InitTrigger(ani_env *env, ani_ref triggerFunc)
{
    HILOG_DEBUG(LOG_CORE, "start to init OnTrigger");
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            return;
        }
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new(std::nothrow) OnTriggerContext();
        if (context_->triggerContext == nullptr) {
            return;
        }
    }
    context_->triggerContext->vm = GetAniVm(env);
    context_->triggerContext->onTrigger = HiAppEventAniUtil::CreateGlobalReference(env, triggerFunc);
}

void AniAppEventWatcher::InitReceiver(ani_env *env, ani_ref receiveFunc)
{
    HILOG_DEBUG(LOG_CORE, "start to init onReceive");
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            return;
        }
    }
    if (context_->receiveContext == nullptr) {
        context_->receiveContext = new(std::nothrow) OnReceiveContext();
        if (context_->receiveContext == nullptr) {
            return;
        }
    }
    context_->receiveContext->vm = GetAniVm(env);
    context_->receiveContext->onReceive = HiAppEventAniUtil::CreateGlobalReference(env, receiveFunc);
}

void AniAppEventWatcher::OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    HILOG_DEBUG(LOG_CORE, "onEvents start, seq=%{public}" PRId64 ", event num=%{public}zu", GetSeq(), events.size());
    if (context_ == nullptr || context_->receiveContext == nullptr || events.empty()) {
        HILOG_ERROR(LOG_CORE, "onReceive context is null or events is empty");
        return;
    }
    context_->receiveContext->domain = events[0]->GetDomain();
    context_->receiveContext->events = events;
    context_->receiveContext->observerSeq = GetSeq();
    auto onReceiveWork = [receiveContext = context_->receiveContext, this] () {
        auto context = static_cast<OnReceiveContext*>(receiveContext);
        ani_size nr_refs = REFERENCES_MAX_NUMBER;
        ani_env* env = GetAniEnv(context->vm);
        env->CreateLocalScope(nr_refs);
        auto callback = context->onReceive;
        if (HiAppEventAniUtil::IsRefUndefined(env, callback)) {
            HILOG_ERROR(LOG_CORE, "failed to get callback from the context");
            env->DestroyLocalScope();
            return;
        }
        std::vector<ani_ref> args = {
            HiAppEventAniUtil::CreateAniString(env, context->domain),
            HiAppEventAniUtil::CreateEventGroups(env, context->events)
        };
        ani_ref ret {};
        if (ANI_OK == env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(callback),
            args.size(), args.data(), &ret)) {
            DeleteEventMappingAsync(context->observerSeq, context->events);
        } else {
            HILOG_ERROR(LOG_CORE, "failed to call onReceive function");
        }
        env->DestroyLocalScope();
    };
    if (ANI_OK != AniSendEvent(onReceiveWork, "OnReceive")) {
        HILOG_ERROR(LOG_CORE, "failed to send event OnReceive.");
    }
}

bool AniAppEventWatcher::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    return (context_ != nullptr && context_->receiveContext != nullptr);
}
} // namespace HiviewDFX
} // namespace OHOS
