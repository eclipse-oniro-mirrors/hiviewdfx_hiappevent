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

#include "ndk_external_log_callback.h"

#include <cstring>
#include <securec.h>

#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "NdkExtLogCb"

namespace OHOS {
namespace HiviewDFX {

NdkExternalLogCallback::NdkExternalLogCallback(OH_HiAppEvent_ExternalLogCapacityReachedCallback callback)
    : callback_(callback) {}

enum OH_HiAppEvent_SysEvent NdkExternalLogCallback::ConvertSysEvent(const std::string& sysEvent)
{
    if (sysEvent == EVENT_APP_CRASH) {
        return OH_APP_CRASH;
    }
    if (sysEvent == EVENT_APP_FREEZE) {
        return OH_APP_FREEZE;
    }
    if (sysEvent == EVENT_RESOURCE_OVERLIMIT) {
        return OH_RESOURCE_OVERLIMIT;
    }
    if (sysEvent == EVENT_ADDRESS_SANITIZER) {
        return OH_ADDRESS_SANITIZER;
    }
    if (sysEvent == EVENT_MAIN_THREAD_JANK) {
        return OH_MAIN_THREAD_JANK;
    }
    if (sysEvent == EVENT_APP_HICOLLIE) {
        return OH_APP_HICOLLIE;
    }
    if (sysEvent == EVENT_SCROLL_JANK) {
        return OH_SCROLL_JANK;
    }
    if (sysEvent == EVENT_CPU_USAGE_HIGH) {
        return OH_CPU_USAGE_HIGH;
    }
    return OH_APP_CRASH; // default fallback
}

void NdkExternalLogCallback::OnCapacityReached(const std::vector<ExternalLogWrapperInfo>& logInfos)
{
    if (callback_ == nullptr) {
        return;
    }
    if (logInfos.empty()) {
        return;
    }

    uint32_t arrLen = static_cast<uint32_t>(logInfos.size());
    auto* logArr = new (std::nothrow) OH_HiAppEvent_ExternalLog[arrLen];
    if (logArr == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to allocate external log array");
        return;
    }

    auto** filePaths = new (std::nothrow) char* [arrLen];
    if (filePaths == nullptr) {
        delete[] logArr;
        HILOG_ERROR(LOG_CORE, "failed to allocate filePaths array");
        return;
    }

    (void)memset_s(filePaths, sizeof(char*) * arrLen, 0, sizeof(char*) * arrLen);

    for (uint32_t i = 0; i < arrLen; ++i) {
        const auto& info = logInfos[i];
        size_t len = info.filePath.size();
        filePaths[i] = new (std::nothrow) char[len + 1];
        if (filePaths[i] != nullptr) {
            (void)memcpy_s(filePaths[i], len, info.filePath.c_str(), len);
            filePaths[i][len] = '\0';
        }
        logArr[i].filePath = filePaths[i];
        logArr[i].generationTs = info.generationTime;
        logArr[i].fileSize = static_cast<long>(info.sizeInKb);
        logArr[i].event = ConvertSysEvent(info.sysEvent);
    }

    callback_(logArr, arrLen);

    for (uint32_t i = 0; i < arrLen; ++i) {
        delete[] filePaths[i];
    }
    delete[] filePaths;
    delete[] logArr;
}

} // namespace HiviewDFX
} // namespace OHOS