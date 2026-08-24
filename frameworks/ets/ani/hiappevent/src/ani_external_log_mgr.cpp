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

#include "ani_external_log_mgr.h"

#include "ani_external_log_container.h"
#include "hiappevent_ani_util.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "AniExtLogMgr"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr ani_size REFERENCES_MAX_NUMBER = 16;

static ani_vm* GetAniVm(ani_env *env)
{
    ani_vm* vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "GetVM failed");
        return nullptr;
    }
    return vm;
}

static ani_env* GetAniEnv(ani_vm *vm)
{
    ani_env* env = nullptr;
    if (vm == nullptr || vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "GetEnv failed");
        return nullptr;
    }
    return env;
}
}

OnCapacityReachedContext::~OnCapacityReachedContext()
{
    ani_env* env = GetAniEnv(vm);
    if (env != nullptr && !HiAppEventAniUtil::IsRefUndefined(env, onCapacityReached)) {
        if (env->GlobalReference_Delete(onCapacityReached) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "GlobalReference_Delete onCapacityReached failed");
        }
    }
    if (env != nullptr && !HiAppEventAniUtil::IsRefUndefined(env, logManagerRef)) {
        if (env->GlobalReference_Delete(logManagerRef) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "GlobalReference_Delete logManagerRef failed");
        }
    }
}

AniExternalLogMgr::~AniExternalLogMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    context_ = nullptr;
}

void AniExternalLogMgr::InitCallback(ani_env *env, ani_object logManagerObj)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (context_ == nullptr) {
        context_ = std::make_shared<OnCapacityReachedContext>();
    }
    context_->vm = GetAniVm(env);
    context_->logManagerRef = HiAppEventAniUtil::CreateGlobalReference(env, logManagerObj);

    ani_ref callback = HiAppEventAniUtil::GetProperty(env, logManagerObj, "onCapacityReached");
    if (!HiAppEventAniUtil::IsRefUndefined(env, callback)) {
        context_->onCapacityReached = HiAppEventAniUtil::CreateGlobalReference(env, callback);
    }
}

ani_status AniExternalLogMgr::AniSendEvent(const std::function<void()> cb, const std::string& name)
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

void AniExternalLogMgr::OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (context_ == nullptr ||
        HiAppEventAniUtil::IsRefUndefined(GetAniEnv(context_->vm), context_->onCapacityReached)) {
        HILOG_WARN(LOG_CORE, "onCapacityReached context is null or callback is undefined");
        return;
    }
    auto work = [logInfos, context = context_]() {
        ani_size nr_refs = REFERENCES_MAX_NUMBER;
        ani_env* env = GetAniEnv(context->vm);
        if (env == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to get env from OnCapacityReached context");
            return;
        }
        if (env->CreateLocalScope(nr_refs) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to create local scope from OnCapacityReached context");
            return;
        }
        auto callback = context->onCapacityReached;
        if (HiAppEventAniUtil::IsRefUndefined(env, callback)) {
            HILOG_ERROR(LOG_CORE, "onCapacityReached callback is undefined");
            env->DestroyLocalScope();
            return;
        }
        // Create container ANI object
        ani_object containerObj = AniExternalLogContainer::CreateAniObject(env, logInfos);
        if (containerObj == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to create ExternalLogContainer object");
            env->DestroyLocalScope();
            return;
        }
        // Call onCapacityReached(container)
        std::vector<ani_ref> args = { containerObj };
        ani_ref ret {};
        if (env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(callback),
            args.size(), args.data(), &ret) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to call onCapacityReached function");
        }
        env->DestroyLocalScope();
    };
    if (AniSendEvent(work, "OnCapacityReached") != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to send event OnCapacityReached.");
    }
}

} // namespace HiviewDFX
} // namespace OHOS