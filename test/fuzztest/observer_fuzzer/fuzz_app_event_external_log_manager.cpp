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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "app_event_external_log_manager.h"
#include "hiappevent_base.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr int BOOL_VALUE_COUNT = 2;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t MAX_LEN_OF_FILE = 128;
constexpr int DEFAULT_EVENT_TYPE = 1;

class FuzzExternalLogCallback : public OHOS::HiviewDFX::ExternalLogManagerCallback {
public:
    void OnCapacityReached(const std::vector<OHOS::HiviewDFX::ExternalLogWrapperInfo>& logInfos) override {}
};
}

static std::string FS(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

void FuzzAppEventExternalLogManager(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string filePath = FS(data, size, MAX_LEN_OF_FILE);
    auto& mgr = OHOS::HiviewDFX::AppEventExternalLogManager::GetInstance();
    mgr.RegisterCallback(nullptr);
    mgr.IsRegistered();
    mgr.RegisterCallback(std::make_shared<FuzzExternalLogCallback>());
    mgr.IsRegistered();
    mgr.CheckCapacity();
    OHOS::HiviewDFX::ExternalLogManager logManager;
    OHOS::HiviewDFX::FileItem item;
    item.file = filePath;
    item.isUsed = data[0] % BOOL_VALUE_COUNT != 0;
    logManager.externalLogs.push_back(item);
    logManager.linkExternalLogs.push_back({item});
    auto event = std::make_shared<OHOS::HiviewDFX::AppEventPack>("d", "n", DEFAULT_EVENT_TYPE);
    event->SetExternalLogManager(logManager);
    event->GetExternalLogManager();
}
