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
#include "napi_app_event_holder.h"

#include "app_event_cache.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Napi_HiAppEvent_Holder" };
constexpr size_t PARAM_NUM = 1;
const std::string HOLDER_CLASS_NAME = "AppEventPackageHolder";
}
thread_local napi_ref NapiAppEventHolder::constructor_ = nullptr;

NapiAppEventHolder::NapiAppEventHolder(std::string name) : name_(name)
{
    takeSize_ = 512 * 1024; // 512 * 1024: 512KB
    packageId_ = 0; // id is incremented from 0
}

napi_value NapiAppEventHolder::NapiConstructor(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM;
    napi_value params[PARAM_NUM] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));

    if (paramNum < PARAM_NUM) {
        HiLog::Error(LABEL, "hodler failed to construct: invalid param num");
        return thisVar;
    }
    auto holder = new(std::nothrow) NapiAppEventHolder(NapiUtil::GetString(env, params[0]));
    napi_wrap(
        env, thisVar, holder,
        [](napi_env env, void* data, void* hint) {
            NapiAppEventHolder* holder = (NapiAppEventHolder*)data;
            delete holder;
        },
        nullptr, nullptr);
    return thisVar;
}

napi_value NapiAppEventHolder::NapiExport(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("setSize", NapiSetSize),
        DECLARE_NAPI_FUNCTION("takeNext", NapiTakeNext)
    };
    napi_value holderClass = nullptr;
    napi_define_class(env, HOLDER_CLASS_NAME.c_str(), HOLDER_CLASS_NAME.size(), NapiConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &holderClass);
    NapiUtil::SetNamedProperty(env, exports, HOLDER_CLASS_NAME, holderClass);
    constructor_ = NapiUtil::CreateReference(env, holderClass);
    return exports;
}

napi_value NapiAppEventHolder::NapiSetSize(napi_env env, napi_callback_info info)
{
    size_t paramNum = PARAM_NUM;
    napi_value params[PARAM_NUM] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNum, params, &thisVar, nullptr));
    if (paramNum < PARAM_NUM) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("size"));
        return nullptr;
    }
    if (!NapiUtil::IsNumber(env, params[0])) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("size", "number"));
        return nullptr;
    }
    if (int num = NapiUtil::GetInt32(env, params[0]); num >= 0) {
        NapiAppEventHolder* holder = nullptr;
        napi_unwrap(env, thisVar, (void**)&holder);
        holder->SetSize(num);
    } else {
        NapiUtil::ThrowError(env, NapiError::ERR_INVALID_SIZE, "Invalid size value.");
    }
    return NapiUtil::CreateUndefined(env);
}

napi_value NapiAppEventHolder::NapiTakeNext(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
    NapiAppEventHolder* holder = nullptr;
    napi_unwrap(env, thisVar, (void**)&holder);
    auto package = holder->TakeNext();
    if (package == nullptr) {
        return NapiUtil::CreateNull(env);
    }
    napi_value packageObj = NapiUtil::CreateObject(env);
    NapiUtil::SetNamedProperty(env, packageObj, "packageId", NapiUtil::CreateInt32(env, package->packageId));
    NapiUtil::SetNamedProperty(env, packageObj, "row", NapiUtil::CreateInt32(env, package->row));
    NapiUtil::SetNamedProperty(env, packageObj, "size", NapiUtil::CreateInt32(env, package->size));
    NapiUtil::SetNamedProperty(env, packageObj, "data", NapiUtil::CreateStrings(env, package->events));
    return packageObj;
}

void NapiAppEventHolder::SetSize(int size)
{
    HiLog::Info(LABEL, "hodler=%{public}s set size=%{public}d", name_.c_str(), size);
    takeSize_ = size;
}

std::shared_ptr<AppEventPackage> NapiAppEventHolder::TakeNext()
{
    auto block = AppEventCache::GetInstance()->GetBlock(name_);
    if (block == nullptr) {
        HiLog::Error(LABEL, "hodler=%{public}s failed to get block", name_.c_str());
        return nullptr;
    }
    std::vector<std::string> events;
    int result = block->Take(takeSize_, events);
    if (result <= 0) {
        HiLog::Info(LABEL, "hodler=%{public}s end to take data", name_.c_str());
        return nullptr;
    }
    auto package = std::make_shared<AppEventPackage>();
    package->packageId = packageId_++;
    package->row = static_cast<int>(events.size());
    package->size = result;
    package->events = events;
    return package;
}
} // namespace HiviewDFX
} // namespace OHOS
