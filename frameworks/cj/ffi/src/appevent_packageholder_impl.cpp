/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "appevent_packageholder_impl.h"
#include "log.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "error.h"

using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
    
char* MallocCString(const std::string& origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto length = origin.length() + 1;
    char* res =  static_cast<char*>(malloc(sizeof(char) * length));
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), length);
}

CArrString ConvertArrString(std::vector<std::string>& vecStr)
{
    CArrString arrStr{0};
    if (vecStr.empty()) {
        return arrStr;
    }
    arrStr.size = static_cast<int64_t>(vecStr.size());
    char **retValue = static_cast<char**>(malloc(sizeof(char *) * arrStr.size));
    if (retValue == nullptr) {
        return arrStr;
    }
    for (int i = 0; i < arrStr.size; ++i) {
        retValue[i] = MallocCString(vecStr[i]);
    }
    arrStr.head = retValue;
    return arrStr;
}

AppEventPackageHolderImpl::AppEventPackageHolderImpl(const std::string& name, int64_t observerSeq)
    : name_(name), observerSeq_(observerSeq)
{
    takeSize_ = 512 * 1024; // 512 * 1024: 512KB
    packageId_ = 0; // id is incremented from 0
    // if the seq is invalid, need to get seq by the name(for js constructor)
    if (observerSeq_ <= 0) {
        observerSeq_ = GetObserverSeqByName(name_);
    }
}

void AppEventPackageHolderImpl::SetSize(int size)
{
    LOGI("hodler seq=%{public}" PRIi64 ", set size=%{public}d", observerSeq_, size);
    takeSize_ = size;
}

std::tuple<int32_t, RetAppEventPackage> AppEventPackageHolderImpl::TakeNext()
{
    std::vector<std::shared_ptr<AppEventPack>> events;
    int32_t ret = ERR_PARAM;
    RetAppEventPackage package;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq_) != 0) {
        LOGE("failed to query events, seq=%{public}" PRId64, observerSeq_);
        return {ret, package};
    }
    if (events.empty()) {
        LOGE("end to query events, seq=%{public}" PRId64, observerSeq_);
        return {ret, package};
    }
    std::vector<int64_t> eventSeqs;
    std::vector<std::string> eventStrs;
    size_t totalSize = 0;
    for (const auto& event : events) {
        std::string eventStr = event->GetEventStr();
        if (static_cast<int>(totalSize + eventStr.size()) > takeSize_) {
            LOGI("stop to take data, totalSize=%{public}zu, takeSize=%{public}" PRIi64 "", totalSize, takeSize_);
            break;
        }
        totalSize += eventStr.size();
        eventStrs.emplace_back(eventStr);
        eventSeqs.emplace_back(event->GetSeq());
    }
    if (eventStrs.empty()) {
        LOGE("take data is empty, seq=%{public}" PRId64, observerSeq_);
        return {ret, package};
    }
    if (AppEventStore::GetInstance().DeleteEventMapping(observerSeq_, eventSeqs) < 0) {
        LOGE("failed to delete mapping data, seq=%{public}" PRId64, observerSeq_);
        return {ret, package};
    }
    package.packageId = packageId_++;
    package.row = static_cast<int>(eventStrs.size());
    package.size = static_cast<int>(totalSize);
    package.events = ConvertArrString(eventStrs);
    ret = SUCCESS_CODE;
    return {ret, package};
}

int64_t GetObserverSeqByName(const std::string& name)
{
    int64_t observerSeq = ERR_CODE_PARAM_INVALID;
    if (observerSeq = AppEventStore::GetInstance().QueryObserverSeq(name); observerSeq <= 0) {
        LOGE("failed to query seq by name= %{public}s", name.c_str());
        return ERR_CODE_PARAM_INVALID;
    }
    return observerSeq;
}
} // HiAppEvent
} // CJSystemapi
} // OHOS