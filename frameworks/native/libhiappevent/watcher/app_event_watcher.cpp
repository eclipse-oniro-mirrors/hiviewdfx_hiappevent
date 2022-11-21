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
#include "app_event_watcher.h"

#include "app_event_cache.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, 0xD002D07, "HiAppEvent_Watcher" };
constexpr unsigned int BIT_MASK = 1;
}
AppEventWatcher::AppEventWatcher(const std::string& name, const std::map<std::string, unsigned int>& filters,
    TriggerCondition cond) : name_(name), filters_(filters), cond_(cond), status_({ 0 })
{}

void AppEventWatcher::OnTrigger(int row, int size)
{
    HiLog::Info(LABEL, "default OnTrigger implementation, row=%{public}d, size=%{public}d", row, size);
}

std::string AppEventWatcher::GetName() const
{
    return name_;
}

TriggerCondition AppEventWatcher::GetCond() const
{
    return cond_;
}

void AppEventWatcher::ProcessEvent(const std::string& domain, int type, const std::string& event)
{
    HiLog::Debug(LABEL, "watcher=%{public}s start to process event", name_.c_str());
    if (!IsInterestedEvent(domain, type)) {
        return;
    }
    auto block = AppEventCache::GetInstance()->GetBlock(name_);
    if (block == nullptr) {
        HiLog::Error(LABEL, "failed to get block=%{public}s from cache", name_.c_str());
        return;
    }
    if (block->Add(event) != 0) {
        HiLog::Error(LABEL, "failed to add event to the block=%{public}s", name_.c_str());
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    ++status_.row;
    status_.size += static_cast<int>(event.size());
    if ((cond_.row > 0 && status_.row >= cond_.row) || (cond_.size > 0 && status_.size >= cond_.size)) {
        OnTrigger(status_.row, status_.size);
        ResetStatus();
    }
}

void AppEventWatcher::ProcessTimeOut()
{
    HiLog::Debug(LABEL, "watcher=%{public}s start to process timeOut", name_.c_str());
    std::lock_guard<std::mutex> lock(mutex_);
    ++status_.timeOut;
    if ((cond_.timeOut > 0 && status_.timeOut >= cond_.timeOut) && status_.row > 0) {
        OnTrigger(status_.row, status_.size);
        ResetStatus();
    }
}

bool AppEventWatcher::IsInterestedEvent(const std::string& domain, int type)
{
    if (filters_.empty()) {
        return true;
    }
    if (auto it = filters_.find(domain); it != filters_.end() && (it->second & (BIT_MASK << type))) {
        return true;
    }
    return false;
}

void AppEventWatcher::ResetStatus()
{
    status_.row = 0;
    status_.size = 0;
    status_.timeOut = 0;
}
} // namespace HiviewDFX
} // namespace OHOS
