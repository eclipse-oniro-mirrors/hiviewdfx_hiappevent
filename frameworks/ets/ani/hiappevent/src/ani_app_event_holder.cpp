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

#include "ani_app_event_holder.h"

#include <cinttypes>

#include "app_event_store.h"
#include "hilog/log.h"
#include "hiappevent_ani_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AniHolder"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t PARAM_NUM = 1;
constexpr int DEFAULT_ROW_NUM = 1;
constexpr int DEFAULT_SIZE = 512 * 1024; // 512 * 1024: 512KB
const std::string HOLDER_CLASS_NAME = "AppEventPackageHolder";

int64_t GetObserverSeqByName(const std::string& name)
{
    int64_t observerSeq = -1;
    if (observerSeq = AppEventStore::GetInstance().QueryObserverSeq(name); observerSeq <= 0) {
        HILOG_WARN(LOG_CORE, "failed to query seq by name=%{public}s", name.c_str());
        return -1;
    }
    return observerSeq;
}
}

AniAppEventHolder::AniAppEventHolder(const std::string& name, int64_t observerSeq)
    : name_(name), observerSeq_(observerSeq), hasSetRow_(false), hasSetSize_(false)
{
    takeRow_ = DEFAULT_ROW_NUM;
    takeSize_ = DEFAULT_SIZE;
    packageId_ = 0; // id is incremented from 0

    // if the seq is invalid, need to get seq by the name(for js constructor)
    if (observerSeq_ <= 0) {
        observerSeq_ = GetObserverSeqByName(name_);
    }
}

static ani_object Wrap(ani_env *env, ani_object aniObject, AniAppEventHolder *holder)
{
    if (env->Object_SetFieldByName_Long(aniObject, "nativeHolder", reinterpret_cast<ani_long>(holder)) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "New holder Fail");
        return nullptr;
    }
    return aniObject;
}

static AniAppEventHolder* Unwrap(ani_env *env, ani_object aniObject)
{
    ani_long context;
    if (env->Object_GetFieldByName_Long(aniObject, "nativeHolder", &context) != ANI_OK) {
        return nullptr;
    }
    return reinterpret_cast<AniAppEventHolder*>(context);
}

void AniAppEventHolder::AniConstructor(ani_env *env, ani_object aniObject, ani_string watcherName)
{
    auto holder = new(std::nothrow) AniAppEventHolder(HiAppEventAniUtil::ParseStringValue(env, watcherName));
    if (holder == nullptr) {
        HILOG_INFO(LOG_CORE, "AniConstructor  holder nullptr");
    }
    Wrap(env, aniObject, holder);
}

void AniAppEventHolder::AniFinalize(ani_env *env, ani_object object, ani_long nativeHolder)
{
    AniAppEventHolder* holder = reinterpret_cast<AniAppEventHolder*>(nativeHolder);
    delete holder;
}

void AniAppEventHolder::AniSetRow(ani_env *env, ani_object object, ani_int size)
{
    int num = static_cast<int>(size);
    if (num <= 0) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_SIZE, "Invalid size value.");
        return;
    }
    AniAppEventHolder* holder = Unwrap(env, object);
    if (holder != nullptr) {
        holder->SetRow(num);
    }
    return;
}

void AniAppEventHolder::AniSetSize(ani_env *env, ani_object object, ani_int size)
{
    int num = static_cast<int>(size);
    if (num < 0) {
        HiAppEventAniUtil::ThrowAniError(env, ERR_INVALID_SIZE, "Invalid size value.");
        return;
    }
    AniAppEventHolder* holder = Unwrap(env, object);
    if (holder != nullptr) {
        holder->SetSize(num);
    }
    return;
}

ani_object AniAppEventHolder::AniTakeNext(ani_env *env, ani_object object)
{
    ani_object packageObj = HiAppEventAniUtil::CreateObject(env, CLASS_NAME_EVENT_PACKAGE);
    AniAppEventHolder* holder = Unwrap(env, object);
    ani_ref result = nullptr;
    env->GetNull(&result);
    if (holder == nullptr) {
        return static_cast<ani_object>(result);
    }
    auto package = holder->TakeNext();
    if (package == nullptr) {
        return static_cast<ani_object>(result);
    }
    env->Object_SetPropertyByName_Int(packageObj, "packageId", package->packageId);
    env->Object_SetPropertyByName_Int(packageObj, "row", package->row);
    env->Object_SetPropertyByName_Int(packageObj, "size", package->size);
    env->Object_SetPropertyByName_Ref(packageObj, "data", HiAppEventAniUtil::CreateStrings(env, package->data));
    env->Object_SetPropertyByName_Ref(packageObj, "appEventInfos",
        HiAppEventAniUtil::CreateEventInfoArray(env, package->events));
    return packageObj;
}

void AniAppEventHolder::SetRow(int row)
{
    HILOG_INFO(LOG_CORE, "hodler seq=%{public}" PRId64 " set row=%{public}d", observerSeq_, row);
    takeRow_ = row;
    hasSetRow_ = true;
}

void AniAppEventHolder::SetSize(int size)
{
    HILOG_INFO(LOG_CORE, "hodler seq=%{public}" PRId64 " set size=%{public}d", observerSeq_, size);
    takeSize_ = size;
    hasSetSize_ = true;
}

std::shared_ptr<AppEventPackage> AniAppEventHolder::TakeNext()
{
    std::vector<std::shared_ptr<AppEventPack>> events;
    bool shouldTakeSize = hasSetSize_ && !hasSetRow_;
    int rowNum = shouldTakeSize ? 0 : takeRow_;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq_, rowNum) != 0) {
        HILOG_WARN(LOG_CORE, "failed to query events, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }
    if (events.empty()) {
        HILOG_DEBUG(LOG_CORE, "end to query events, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }

    std::vector<int64_t> eventSeqs;
    std::vector<std::string> eventStrs;
    size_t totalSize = 0;
    auto package = std::make_shared<AppEventPackage>();
    for (auto event : events) {
        std::string eventStr = event->GetEventStr();
        if (shouldTakeSize && static_cast<int>(totalSize + eventStr.size()) > takeSize_) {
            HILOG_INFO(LOG_CORE, "stop to take data, totalSize=%{public}zu, takeSize=%{public}d",
                totalSize, takeSize_);
            break;
        }
        totalSize += eventStr.size();
        eventStrs.emplace_back(eventStr);
        eventSeqs.emplace_back(event->GetSeq());
        package->events.emplace_back(event);
    }
    if (eventStrs.empty()) {
        HILOG_INFO(LOG_CORE, "take data is empty, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }
    if (!AppEventStore::GetInstance().DeleteData(observerSeq_, eventSeqs)) {
        HILOG_INFO(LOG_CORE, "failed to delete mapping data, seq=%{public}" PRId64, observerSeq_);
        return nullptr;
    }

    package->packageId = packageId_++;
    package->row = static_cast<int>(eventStrs.size());
    package->size = static_cast<int>(totalSize);
    package->data = eventStrs;
    return package;
}
} // namespace HiviewDFX
} // namespace OHOS
