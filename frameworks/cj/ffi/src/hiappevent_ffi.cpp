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

#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

#include "appevent_watcher_impl.h"
#include "cj_ffi/cj_common_ffi.h"
#include "ffi_remote_data.h"
#include "hiappevent_ffi.h"
#include "hiappevent_impl.h"
#include "hiappevent_verify.h"
#include "log.h"
#include "module_loader.h"
#include "error.h"
using namespace OHOS::HiviewDFX;
using namespace OHOS::CJSystemapi::HiAppEvent;

namespace {
constexpr int ERR_CODE_PARAM_FORMAT = -1;
constexpr int ERR_CODE_PARAM_INVALID = -2;
const int8_t INT32_VALUE = 0;
const int8_t DOUBLE_VALUE = 1;
const int8_t STRING_VALUE = 2;
const int8_t BOOL_VALUE = 3;
const int8_t INT32_ARRAY_VALUE = 4;
const int8_t DOUBLE_ARRAY_VALUE = 5;
const int8_t STRING_ARRAY_VALUE = 6;
const int8_t BOOL_ARRAY_VALUE = 7;
constexpr int BIT_MASK = 1;
constexpr unsigned int BIT_ALL_TYPES = 0xff;

int CheckCondition(TriggerCondition &cond, CTriggerCondition triggerCondition)
{
    int ret = ERR_PARAM;
    if (cond.row < 0) {
        ret = ERR_INVALID_COND_ROW;
        return ret;
    }
    if (cond.size < 0) {
        ret = ERR_INVALID_COND_SIZE;
        return ret;
    }
    constexpr int scale = 30; // step of time is 30s
    if (triggerCondition.timeOut * scale < 0) {
        ret = ERR_INVALID_COND_TIMEOUT;
        return ret;
    }
    cond.row = triggerCondition.row;
    cond.size = triggerCondition.size;
    cond.timeout = triggerCondition.timeOut * scale;
    return ret;
}

void CharPtrToVector(char** charPtr, int size, std::vector<std::string> &result)
{
    for (int i = 0; i < size; i++) {
        result.push_back(std::string(charPtr[i]));
    }
}

std::unordered_set<std::string> genArrString2Set(const CArrString& str)
{
    std::vector<std::string> strVec;
    CharPtrToVector(str.head, str.size, strVec);
    std::unordered_set<std::string> res(strVec.begin(), strVec.end());
    return res;
}

int ConvertConfigReportProp(const CAppEventReportConfig& config, EventConfig reportConf)
{
    if (config.domain != nullptr) {
        reportConf.domain = config.domain;
    }
    if (config.name != nullptr) {
        reportConf.name = config.name;
    }
    reportConf.isRealTime = config.isRealTime;
    if (!IsValidEventConfig(reportConf)) {
        return ERR_CODE_PARAM_INVALID;
    }
    return SUCCESS_CODE;
}

int ConvertReportConfig(const CProcessor& processor, ReportConfig& conf)
{
    if (processor.name != nullptr && IsValidProcessorName(std::string(processor.name))) {
        conf.name = processor.name;
    } else {
        return ERR_CODE_PARAM_FORMAT;
    }
    conf.debugMode = processor.debugMode;
    conf.routeInfo = processor.routeInfo;
    conf.appId = processor.appId;
    conf.triggerCond.onStartup = processor.onStartReport;
    conf.triggerCond.onBackground = processor.onBackgroundReport;
    if (!IsValidPeriodReport(processor.periodReport)) {
        conf.triggerCond.timeout = 0;
    } else {
        conf.triggerCond.timeout = processor.periodReport;
    }
    if (!IsValidBatchReport(processor.batchReport)) {
        conf.triggerCond.row = 0;
    } else {
        conf.triggerCond.row = processor.batchReport;
    }
    conf.userIdNames = genArrString2Set(processor.userIds);
    conf.userPropertyNames = genArrString2Set(processor.userProperties);
    std::vector<EventConfig> eventConfigs;
    CAppEventReportConfig* configPtr = processor.eventConfigs.head;
    for (int i = 0; i < processor.eventConfigs.size; i++) {
        EventConfig reportConf;
        int res =  ConvertConfigReportProp(configPtr[i], reportConf);
        if (res != 0) {
            return res;
        }
        eventConfigs.push_back(reportConf);
    }
    conf.eventConfigs = eventConfigs;
    return SUCCESS_CODE;
}

void AddParams2EventPack(const CArrParameters& params, std::shared_ptr<AppEventPack> appEventPack_)
{
    for (int i = 0; i < params.size; i++) {
        auto head = params.head + i;
        switch (head->valueType) {
            case INT32_VALUE: // int32_t
                appEventPack_->AddParam(std::string(head->key), *(int32_t*)head->value);
                break;
            case DOUBLE_VALUE: // double
                appEventPack_->AddParam(std::string(head->key), *(double*)head->value);
                break;
            case STRING_VALUE: // std::string
                appEventPack_->AddParam(std::string(head->key), std::string((char*)head->value));
                break;
            case BOOL_VALUE: // bool
                appEventPack_->AddParam(std::string(head->key), *(bool*)head->value);
                break;
            case INT32_ARRAY_VALUE: { // int32_array
                int* intArr = (int*)head->value;
                std::vector<int> intVec(intArr, intArr + head->size);
                appEventPack_->AddParam(std::string(head->key), intVec);
                break;
            }
            case DOUBLE_ARRAY_VALUE: { // double_array
                double* doubleArr = (double*)head->value;
                std::vector<double> doubleVec(doubleArr, doubleArr + head->size);
                appEventPack_->AddParam(std::string(head->key), doubleVec);
                break;
            }
            case STRING_ARRAY_VALUE: { // string_array
                char** strPtr = (char**)head->value;
                std::vector<std::string> strVec;
                CharPtrToVector(strPtr, head->size, strVec);
                appEventPack_->AddParam(std::string(head->key), strVec);
                break;
            }
            case BOOL_ARRAY_VALUE: { // bool_array
                bool* boolArr = (bool*)head->value;
                std::vector<bool> boolVec(boolArr, boolArr + head->size);
                appEventPack_->AddParam(std::string(head->key), boolVec);
                break;
            }
            default:
                break;
        }
    }
}
}

extern "C" {
int FfiOHOSHiAppEventConfigure(CConfigOption config)
{
    int code = HiAppEventImpl::Configure(config.disable, config.maxStorage);
    if (code != SUCCESS_CODE) {
        LOGE("FfiOHOSHiAppEventConfigure failed");
        return code;
    }
    return SUCCESS_CODE;
}

int FfiOHOSHiAppEventWrite(CAppEventInfo info)
{
    auto appEventPack_ = std::make_shared<AppEventPack>(info.domain, info.name, info.event);
    AddParams2EventPack(info.cArrParamters, appEventPack_);
    int code = HiAppEventImpl::Write(appEventPack_);
    if (code != SUCCESS_CODE) {
        LOGE("HiAppEvent::FfiOHOSHiAppEventWrite failed");
        return GetErrorCode(code);
    }
    return code;
}

RetDataBool FfiOHOSHiAppEventAddProcessor(CProcessor processor)
{
    RetDataBool ret = { .code = ErrorCode::ERROR_UNKNOWN, .data = false };
    ReportConfig conf;
    int res = ConvertReportConfig(processor, conf);
    if (res == ERR_CODE_PARAM_FORMAT) {
        LOGE("failed to add processor, params format error");
        ret.code = ERR_PARAM;
        ret.data = false;
        return ret;
    }
    if (HiAppEvent::ModuleLoader::GetInstance().Load(conf.name) != 0) {
        LOGE("failed to add processor=%{public}s, name no found", conf.name.c_str());
        return {ERR_CODE_PARAM_FORMAT, true};
    }
    int64_t processorId = HiAppEventImpl::AddProcessor(conf);
    if (processorId <= 0) {
        LOGE("HiAppEvent::FfiOHOSHiAppEventAddProcessor failed");
    }
    ret.code = processorId;
    ret.data = true;
    return ret;
}

int FfiOHOSHiAppEventRemoveProcessor(int64_t id)
{
    int res = HiAppEventImpl::RemoveProcessor(id);
    return res;
}

int FfiOHOSHiAppEventSetUserId(const char* name, const char* value)
{
    if (!IsValidUserIdName(std::string(name))) {
        return ERR_PARAM;
    }
    if (!IsValidUserIdValue(std::string(value))) {
        return ERR_PARAM;
    }
    int res = HiAppEventImpl::SetUserId(std::string(name), std::string(value));
    if (res != 0) {
        return ERR_PARAM;
    }
    return SUCCESS_CODE;
}

RetDataCString FfiOHOSHiAppEventGetUserId(const char* name)
{
    RetDataCString ret = { .code = ERR_PARAM, .data = nullptr };
    if (!IsValidUserIdName(std::string(name))) {
        ret.code = ERR_PARAM;
        ret.data = nullptr;
        return ret;
    }
    auto [status, userId] = HiAppEventImpl::GetUserId(std::string(name));
    if (status != 0) {
        LOGE("HiAppEvent::FfiOHOSHiAppEventGetUserId error");
        ret.code = status;
        ret.data = nullptr;
        return ret;
    }
    ret.code = status;
    ret.data = MallocCString(userId);
    return ret;
}

int FfiOHOSHiAppEventSetUserProperty(const char* name, const char* value)
{
    if (!IsValidUserPropName(name)) {
        return ERR_PARAM;
    }
    if (!IsValidUserPropValue(value)) {
        return ERR_PARAM;
    }
    int res = HiAppEventImpl::SetUserProperty(std::string(name), std::string(value));
    if (res != 0) {
        return ERR_PARAM;
    }
    return res;
}

RetDataCString FfiOHOSHiAppEventgetUserProperty(const char* name)
{
    RetDataCString ret = { .code = ERR_PARAM, .data = nullptr };
    if (!IsValidUserPropName(std::string(name))) {
        ret.code = ERR_PARAM;
        ret.data = nullptr;
        return ret;
    }
    auto [status, propertyId] = HiAppEventImpl::GetUserProperty(std::string(name));
    if (status != 0) {
        LOGE("HiAppEvent::FfiOHOSHiAppEventgetUserProperty error");
        ret.code = status;
        ret.data = nullptr;
        return ret;
    }
    ret.code = status;
    ret.data = MallocCString(propertyId);
    return ret;
}

void FfiOHOSHiAppEventclearData()
{
    HiAppEventImpl::ClearData();
}

int64_t FfiOHOSHiAppEventConstructor(char* cWatcherName)
{
    auto nativeHolder = OHOS::FFI::FFIData::Create<AppEventPackageHolderImpl>(cWatcherName, -1L);
    return nativeHolder->GetID();
}
 
int FfiOHOSHiAppEventSetSize(int64_t id, int size)
{
    auto nativeAppEventPackageHolder = OHOS::FFI::FFIData::GetData<AppEventPackageHolderImpl>(id);
    int ret = SUCCESS_CODE;
    if (size >= 0) {
        nativeAppEventPackageHolder->SetSize(size);
    } else {
        ret = ERR_INVALID_SIZE;
    }
    return ret;
}
 
ReTakeNext FfiOHOSHiAppEventTakeNext(int64_t id)
{
    auto nativeAppEventPackageHolder = OHOS::FFI::FFIData::GetData<AppEventPackageHolderImpl>(id);
    auto [state, package] = nativeAppEventPackageHolder->TakeNext();
    ReTakeNext ret;
    ret.status = state;
    ret.event = package;
    return ret;
}

RetDataI64 FfiOHOSHiAppEventAddWatcher(CWatcher watcher)
{
    RetDataI64 ret = { .code = ERR_PARAM, .data = 0 };
    // check isValid
    std::string name = std::string(watcher.name);
    if (!IsValidWatcherName(name)) {
        ret.code = ERR_INVALID_WATCHER_NAME;
        return ret;
    }
    TriggerCondition cond;
    ret.code = CheckCondition(cond, watcher.triggerCondition);
    if (ret.code != ERR_PARAM) {
        return ret;
    }
    std::vector<AppEventFilter> filters;
    std::unordered_set<std::string> names;
    for (int i = 0; i < watcher.appEventFilters.size; i++) {
        for (int j = 0; j < watcher.appEventFilters.head[i].names.size; j++) {
            names.insert(std::string(watcher.appEventFilters.head[i].names.head[j]));
        }
        std::string domain = std::string(watcher.appEventFilters.head[i].domain);
        if (!IsValidDomain(domain)) {
            ret.code = ERR_INVALID_FILTER_DOMAIN;
            return ret;
        }
        uint32_t types = 0;
        for (int k = 0; k < watcher.appEventFilters.head[i].eventTypes.size; k++) {
            types |= (BIT_MASK << watcher.appEventFilters.head[i].eventTypes.head[k]);
        }
        types = types > 0 ? types : BIT_ALL_TYPES;
        filters.emplace_back(AppEventFilter(domain, names, types));
    }

    auto [status, holderId] = HiAppEventImpl::addWatcher(name, filters, cond,
                                                         watcher.callbackOnTriggerRef, watcher.callbackOnReceiveRef);
    if (status != 0) {
        ret.code = status;
        return ret;
    }
    if (holderId == -1) {
        ret.data = -1;
        return ret;
    }
    ret.code = status;
    ret.data = holderId;
    return ret;
}

int FfiOHOSHiAppEventRemoveWatcher(CWatcher watcher)
{
    std::string name = std::string(watcher.name);
    if (!IsValidWatcherName(name)) {
        return ERR_INVALID_WATCHER_NAME;
    }
    HiAppEventImpl::removeWatcher(name);
    return SUCCESS_CODE;
}
}