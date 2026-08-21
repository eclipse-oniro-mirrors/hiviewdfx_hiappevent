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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_MGR_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_MGR_H

#include <mutex>
#include <vector>

#include "app_event_external_log_manager.h"
#include "hiappevent_base.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace HiviewDFX {

struct OnCapacityReachedContext {
    ~OnCapacityReachedContext();
    napi_env env = nullptr;
    napi_ref onCapacityReached = nullptr;
    napi_ref logManagerRef = nullptr;
};

class NapiExternalLogMgr : public ExternalLogManagerCallback {
public:
    NapiExternalLogMgr() = default;
    ~NapiExternalLogMgr() override;

    void InitCallback(napi_env env, napi_value logManagerObj);
    void OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos) override;

private:
    std::mutex mutex_;
    std::shared_ptr<OnCapacityReachedContext> context_ = nullptr;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_EXTERNAL_LOG_MGR_H