/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "app_event.h"

#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hiappevent_write.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
Event::Event(const std::string& domain, const std::string& name, EventType type)
{
    eventPack_ = std::make_shared<AppEventPack>(domain, name, type);
}

void Event::AddParam(const std::string& key, bool value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, int32_t value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, int64_t value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, double value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, const std::string& value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, const std::vector<bool>& value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, const std::vector<int32_t>& value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, const std::vector<int64_t>& value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, const std::vector<double>& value)
{
    eventPack_->AddParam(key, value);
}

void Event::AddParam(const std::string& key, const std::vector<std::string>& value)
{
    eventPack_->AddParam(key, value);
}

int Write(const Event& event)
{
    int ret = VerifyAppEvent(event.eventPack_);
    if (ret >= 0) {
        WriteEvent(event.eventPack_);
    }
    return ret;
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
