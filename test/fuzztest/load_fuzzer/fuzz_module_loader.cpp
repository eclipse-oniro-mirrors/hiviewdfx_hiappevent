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

#include <memory>
#include <string>
#include "module_loader.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
}

class AppEventProcessorTest : public OHOS::HiviewDFX::HiAppEvent::AppEventProcessor {
public:
    int OnReport(
        int64_t processorSeq,
        const std::vector<OHOS::HiviewDFX::HiAppEvent::UserId>& userIds,
        const std::vector<OHOS::HiviewDFX::HiAppEvent::UserProperty>& userProperties,
        const std::vector<OHOS::HiviewDFX::HiAppEvent::AppEventInfo>& events) override
    {
        return 0;
    }

    int ValidateUserId(const OHOS::HiviewDFX::HiAppEvent::UserId& userId) override
    {
        return 0;
    }

    int ValidateUserProperty(const OHOS::HiviewDFX::HiAppEvent::UserProperty& userProperty) override
    {
        return 0;
    }

    int ValidateEvent(const OHOS::HiviewDFX::HiAppEvent::AppEventInfo& event) override
    {
        return 0;
    }
};

static std::string FuzzStr(const uint8_t* data, size_t size, size_t maxLen = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(data), std::min(size, maxLen));
}

void FuzzModuleLoader(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string n = FuzzStr(data, size, DEFAULT_STR_MAX_LEN);
    OHOS::HiviewDFX::HiAppEvent::ModuleLoader loader;
    loader.Load(n);
    loader.Unload(n);
    loader.RegisterProcessor(n, std::make_shared<AppEventProcessorTest>());
    loader.CreateProcessorProxy(n);
    loader.UnregisterProcessor(n);
    loader.CreateProcessorProxy(n);
}
