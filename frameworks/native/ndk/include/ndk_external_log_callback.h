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

#ifndef HIAPPEVENT_NDK_EXTERNAL_LOG_CALLBACK_H
#define HIAPPEVENT_NDK_EXTERNAL_LOG_CALLBACK_H

#include "app_event_external_log_manager.h"
#include "hiappevent/hiappevent.h"

namespace OHOS {
namespace HiviewDFX {

class NdkExternalLogCallback : public ExternalLogManagerCallback {
public:
    explicit NdkExternalLogCallback(OH_HiAppEvent_ExternalLogCapacityReachedCallback callback);
    ~NdkExternalLogCallback() override = default;

    void OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos) override;

private:
    static enum OH_HiAppEvent_SysEvent ConvertSysEvent(const std::string& sysEvent);

    OH_HiAppEvent_ExternalLogCapacityReachedCallback callback_;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_NDK_EXTERNAL_LOG_CALLBACK_H
