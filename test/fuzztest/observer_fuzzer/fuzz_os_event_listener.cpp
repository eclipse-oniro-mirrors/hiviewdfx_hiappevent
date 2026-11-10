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
#include <vector>

#include "hiappevent_base.h"
#include "os_event_listener.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
}

void FuzzOsEventListener(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::vector<std::shared_ptr<OHOS::HiviewDFX::HiAppEvent::AppEventObserver>> observers;
    auto listener = std::make_shared<OHOS::HiviewDFX::OsEventListener>(observers);
    listener->AddListenedEvents(static_cast<uint64_t>(data[0]));
    listener->SetListenedEvents(static_cast<uint64_t>(data[0]));
    std::vector<std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>>> linkEvents;
    listener->GetLinkEvents(linkEvents);
    listener->RemoveOsEventDir();
}
