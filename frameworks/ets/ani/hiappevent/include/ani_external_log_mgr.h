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

#ifndef ANI_EXTERNAL_LOG_MGR_H
#define ANI_EXTERNAL_LOG_MGR_H

#include <ani.h>

#include "app_event_external_log_manager.h"
#include "event_handler.h"

namespace OHOS {
namespace HiviewDFX {

struct OnCapacityReachedContext {
    ~OnCapacityReachedContext();
    ani_vm* vm{};
    ani_ref onCapacityReached{};
    ani_ref logManagerRef{};
};

class AniExternalLogMgr : public ExternalLogManagerCallback {
public:
    AniExternalLogMgr() = default;
    ~AniExternalLogMgr() override;

    void InitCallback(ani_env *env, ani_object logManagerObj);
    void OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos) override;

private:
    ani_status AniSendEvent(const std::function<void()> cb, const std::string& name);

    std::mutex mutex_;
    std::shared_ptr<OnCapacityReachedContext> context_{};
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_{};
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_EXTERNAL_LOG_MGR_H