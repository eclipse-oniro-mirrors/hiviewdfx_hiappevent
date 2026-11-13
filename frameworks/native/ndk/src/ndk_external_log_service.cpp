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

#include "ndk_external_log_service.h"

#include "app_event_stat.h"
#include "app_event_external_log_manager.h"
#include "hiappevent_base.h"
#include "ndk_external_log_callback.h"

using namespace OHOS::HiviewDFX;

namespace {
constexpr uint32_t REG_CALLBACK_RETCODE_UNKNOWN = 0;
constexpr uint32_t REG_CALLBACK_RETCODE_INVALID_PARAM = 1;
constexpr uint32_t MAX_REG_CALLBACK_RETCODE = REG_CALLBACK_RETCODE_INVALID_PARAM;
constexpr const char* REG_CALLBACK_API_NAME = "RegExternalLogCapacityReachedCallback";

uint32_t MapRegCallbackRetCode(int ret)
{
    if (ret == ErrorCode::ERROR_INVALID_PARAM_VALUE) {
        return REG_CALLBACK_RETCODE_INVALID_PARAM;
    }
    return REG_CALLBACK_RETCODE_UNKNOWN;
}

int RegCapacityReachedCallbackImpl(OH_HiAppEvent_ExternalLogCapacityReachedCallback callback)
{
    if (callback == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    auto ndkCallback = std::make_shared<NdkExternalLogCallback>(callback);
    if (!AppEventExternalLogManager::GetInstance().RegisterCallback(ndkCallback)) {
        return ErrorCode::ERROR_UNKNOWN;
    }
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}
}

int RegExternalLogCapacityReachedCallback(OH_HiAppEvent_ExternalLogCapacityReachedCallback callback)
{
    ApiCallScopedStat stat(REG_CALLBACK_API_NAME, ApiType::NATIVE, MAX_REG_CALLBACK_RETCODE);
    int result = RegCapacityReachedCallbackImpl(callback);
    if (result != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
        stat.SetErrorCode(MapRegCallbackRetCode(result));
    }
    return result;
}