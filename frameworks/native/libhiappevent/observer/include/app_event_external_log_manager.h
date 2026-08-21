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
#ifndef APP_EVENT_EXTERNAL_LOG_MANAGER_H
#define APP_EVENT_EXTERNAL_LOG_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {

class ExternalLogManagerCallback {
public:
    virtual ~ExternalLogManagerCallback() = default;
    virtual void OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos) = 0;
};

class AppEventExternalLogManager {
public:
    static AppEventExternalLogManager& GetInstance();

    bool RegisterCallback(std::shared_ptr<ExternalLogManagerCallback> callback);
    bool IsRegistered();
    void CheckCapacity();

private:
    AppEventExternalLogManager() = default;
    ~AppEventExternalLogManager() = default;

    void ScanLogFiles(const std::string& dir, std::vector<ExternalLogWrapperInfo>& logInfos);
    ExternalLogWrapperInfo ParseLogFileInfo(const std::string& filePath) const;

    std::mutex mutex_;
    std::shared_ptr<ExternalLogManagerCallback> callback_ = nullptr;
};

} // namespace HiviewDFX
} // namespace OHOS
#endif // APP_EVENT_EXTERNAL_LOG_MANAGER_H