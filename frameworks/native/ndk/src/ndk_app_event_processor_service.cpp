/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <cinttypes>

#include "ndk_app_event_processor_service.h"

#include "app_event_observer_mgr.h"
#include "base_type.h"
#include "hilog/log.h"
#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "ndk_app_event_processor.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NdkProcessorService"

#ifndef CHECK_PROCESSOR_PTR_AND_RETURN_VOID
#define CHECK_PROCESSOR_PTR_AND_RETURN_VOID(ptr)  \
    if ((ptr) == nullptr) {                       \
        return;                                   \
    }
#endif

#ifndef CHECK_PROCESSOR_PTR_AND_RETURN
#define CHECK_PROCESSOR_PTR_AND_RETURN(ptr, ret)  \
    if ((ptr) == nullptr) {                       \
        return ret;                               \
    }
#endif

using namespace OHOS::HiviewDFX;

struct HiAppEvent_Processor* CreateProcessor(const char* name)
{
    if (!IsApp()) {
        HILOG_DEBUG(LOG_CORE, "caller is not app");
        return nullptr;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(name, nullptr)
    if (!IsValidProcessorName(name)) {
        HILOG_DEBUG(LOG_CORE, "Invalid processor name");
        return nullptr;
    }
    auto ndkProcessorPtr = new NdkAppEventProcessor(name);
    return reinterpret_cast<HiAppEvent_Processor*>(ndkProcessorPtr);
}

int SetReportRoute(struct HiAppEvent_Processor* processor, const char* appId, const char* routeInfo)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (appId == nullptr || routeInfo == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    if (!IsValidAppId(appId) || !IsValidRouteInfo(routeInfo)) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH;
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetReportRoute(appId, routeInfo);
}

int SetReportPolicy(struct HiAppEvent_Processor* processor, int periodReport, int batchReport, bool onStartReport,
    bool onBackgroundReport)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (!IsValidPeriodReport(periodReport) || !IsValidBatchReport(batchReport)) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetReportPolicy(periodReport, batchReport, onStartReport, onBackgroundReport);
}

int SetReportEvent(struct HiAppEvent_Processor* processor, const char* domain, const char* name, bool isRealTime)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (domain == nullptr || name == nullptr || !IsValidDomain(domain) || !IsValidEventName(name)) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetReportEvent(domain, name, isRealTime);
}

int SetCustomConfig(struct HiAppEvent_Processor* processor, const char* key, const char* value)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (key == nullptr || value == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    if (!IsValidCustomConfig(key, value)) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH;
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetCustomConfig(key, value);
}

int SetConfigId(struct HiAppEvent_Processor* processor, int configId)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (!IsValidConfigId(configId)) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetConfigId(configId);
}

int SetReportUserId(struct HiAppEvent_Processor* processor, const char* const * userIdNames, int size)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (userIdNames == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    for (int i = 0; i < size; ++i) {
        if (userIdNames[i] == nullptr) {
            return ErrorCode::ERROR_INVALID_PARAM_VALUE;
        }
        if (!IsValidUserIdName(userIdNames[i])) {
            return ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH;
        }
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetReportUserId(userIdNames, size);
}

int SetReportUserProperty(struct HiAppEvent_Processor* processor, const char* const * userPropertyNames, int size)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    if (userPropertyNames == nullptr) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    for (int i = 0; i < size; ++i) {
        if (userPropertyNames[i] == nullptr) {
            return ErrorCode::ERROR_INVALID_PARAM_VALUE;
        }
        if (!IsValidUserPropName(userPropertyNames[i])) {
            return ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH;
        }
    }
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    return ndkProcessorPtr->SetReportUserProperty(userPropertyNames, size);
}

int64_t AddProcessor(struct HiAppEvent_Processor* processor)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN(processor, ErrorCode::ERROR_INVALID_PROCESSOR)
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    HiAppEvent::ReportConfig config = ndkProcessorPtr->GetConfig();
    if (VerifyReportConfig(config) != 0) {
        HILOG_DEBUG(LOG_CORE, "faied to add processor=%{public}s, reportConfig is invalid", config.name.c_str());
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    if (AppEventObserverMgr::GetInstance().Load(config.name) != 0) {
        HILOG_DEBUG(LOG_CORE, "faied to add processor=%{public}s, name not found", config.name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    int64_t processorId = AppEventObserverMgr::GetInstance().RegisterObserver(config.name, config);
    if (processorId <= 0) {
        HILOG_DEBUG(LOG_CORE, "faied to add processor=%{public}s, register processor error", config.name.c_str());
        return ErrorCode::ERROR_UNKNOWN;
    }
    return processorId;
}

void DestoryProcessor(struct HiAppEvent_Processor* processor)
{
    if (!IsApp()) {
        return;
    }
    CHECK_PROCESSOR_PTR_AND_RETURN_VOID(processor)
    auto ndkProcessorPtr = reinterpret_cast<NdkAppEventProcessor*>(processor);
    delete ndkProcessorPtr;
}

int RemoveProcessor(int64_t processorId)
{
    if (!IsApp()) {
        return ErrorCode::ERROR_NOT_APP;
    }
    if (processorId <= 0) {
        HILOG_DEBUG(LOG_CORE, "Failed to remove processor id=%{public}" PRId64, processorId);
        return ErrorCode::ERROR_PROCESSOR_NOT_ADDED;
    }
    if (AppEventObserverMgr::GetInstance().UnregisterObserver(processorId, ObserverType::PROCESSOR) != 0) {
        HILOG_DEBUG(LOG_CORE, "Failed to remove processor id=%{public}" PRId64, processorId);
        return ErrorCode::ERROR_UNKNOWN;
    }
    return 0;
}
