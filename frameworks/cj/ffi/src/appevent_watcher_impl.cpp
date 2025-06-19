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
 
#include "appevent_watcher_impl.h"
#include "event_json_util.h"
#include "log.h"
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
void FreeCParameters(CParameters par)
{
    free(par.key);
    free(par.value);
}

void FreeCArrParameters(CArrParameters par)
{
    if (par.head != nullptr) {
        for (int64_t i = 0; i < par.size; i++) {
            FreeCParameters(par.head[i]);
        }
        free(par.head);
    }
}

void FreeCAppEventInfo(CAppEventInfo info)
{
    free(const_cast<char*>(info.domain));
    free(const_cast<char*>(info.name));
    FreeCArrParameters(info.cArrParamters);
}

OnTriggerContext::~OnTriggerContext()
{
    if (onTrigger != nullptr) {
        onTrigger = nullptr;
    }
    if (holder != nullptr) {
        delete holder;
        holder = nullptr;
    }
}

OnReceiveContext::~OnReceiveContext()
{
    if (onReceive != nullptr) {
        onReceive = nullptr;
    }
}

WatcherContext::~WatcherContext()
{
    if (triggerContext != nullptr) {
        delete triggerContext;
        triggerContext = nullptr;
    }
    if (receiveContext != nullptr) {
        delete receiveContext;
        receiveContext = nullptr;
    }
}

AppEventWatcherImpl::~AppEventWatcherImpl()
{
    if (context_ != nullptr) {
        delete context_;
        context_ = nullptr;
    }
}

AppEventWatcherImpl::AppEventWatcherImpl(
    const std::string& name,
    const std::vector<AppEventFilter>& filters, TriggerCondition cond)
    : HiviewDFX::AppEventWatcher(name, filters, cond), context_(nullptr) {}

void AppEventWatcherImpl::InitTrigger(void (*callbackRef)(int, int, int64_t))
{
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            LOGE("InitTrigger is failed, context_ is null");
            return;
        }
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new(std::nothrow) OnTriggerContext();
        if (context_->triggerContext == nullptr) {
            delete context_;
            context_ = nullptr;
            LOGE("InitTrigger is failed, context_->triggerContext is null");
            return;
        }
    }
    context_->triggerContext->onTrigger = CJLambda::Create(callbackRef);
}

void AppEventWatcherImpl::InitHolder(AppEventPackageHolderImpl* holder)
{
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            LOGE("InitHolder is failed, context_ is null");
            return;
        }
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new(std::nothrow) OnTriggerContext();
        if (context_->triggerContext == nullptr) {
            delete context_;
            context_ = nullptr;
            LOGE("InitHolder is failed, context_->triggerContext is null");
            return;
        }
    }
    context_->triggerContext->holder = holder;
}

void AppEventWatcherImpl::InitReceiver(void (*callbackRef)(char*, CArrRetAppEventGroup))
{
    if (context_ == nullptr) {
        context_ = new(std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            LOGE("context_ is null");
            return;
        }
    }
    if (context_->receiveContext == nullptr) {
        context_->receiveContext = new(std::nothrow) OnReceiveContext();
        if (context_->receiveContext == nullptr) {
            delete context_;
            context_ = nullptr;
            LOGE("context_->receiveContext is null");
            return;
        }
    }
    context_->receiveContext->onReceive = CJLambda::Create(callbackRef);
}

void ConvertArrBool(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.valueType = TYPE_ARRBOOL;
    bool* retArrValue = static_cast<bool*>(malloc(sizeof(bool) * retValue.size));
    if (retArrValue == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        retArrValue[i] = cJSON_IsTrue(item);
    }
    retValue.value = retArrValue;
}

void ConvertArrInt(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.valueType = TYPE_ARRINT;
    int32_t* retArrValue = static_cast<int32_t*>(malloc(sizeof(int32_t) * retValue.size));
    if (retArrValue == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        if (item && EventJsonUtil::CJsonIsInt(item)) {
            retArrValue[i] = static_cast<int32_t>(item->valuedouble);
        }
    }
    retValue.value = retArrValue;
}

void ConvertArrInt64(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.valueType = TYPE_ARRINT64;
    int64_t* retArrValue = static_cast<int64_t*>(malloc(sizeof(int64_t) * retValue.size));
    if (retArrValue == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        if (item && EventJsonUtil::CJsonIsInt64(item)) {
            retArrValue[i] = static_cast<int64_t>(item->valuedouble);
        }
    }
    retValue.value = retArrValue;
}

void CovertArrDouble(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.valueType = TYPE_ARRFLOAT;
    double* retArrValue = static_cast<double*>(malloc(sizeof(double) * retValue.size));
    if (retArrValue == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        if (item && cJSON_IsNumber(item)) {
            retArrValue[i] = item->valuedouble;
        }
    }
    retValue.value = retArrValue;
}

void CovertArrString(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.valueType = TYPE_ARRSTRING;
    char** retArrValue = static_cast<char**>(malloc(sizeof(char*) * retValue.size));
    if (retArrValue == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        retArrValue[i] = MallocCString(item->valuestring);
    }
    retValue.value = retArrValue;
}

void CovertArrObjStr(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.valueType = TYPE_ARRSTRING;
    char** retArrValue = static_cast<char**>(malloc(sizeof(char*) * retValue.size));
    if (retArrValue == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        auto itemStr = cJSON_PrintUnformatted(item);
        std::string jsonStr = itemStr;
        retArrValue[i] = MallocCString(jsonStr);
        cJSON_free(itemStr);
    }
    retValue.value = retArrValue;
}

bool CheckArrBool(const cJSON *jsonValue)
{
    size_t nBool = 0;
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        if (cJSON_IsBool(item)) {
            nBool++;
        }
    }
    return nBool == jsonSize;
}

bool CheckArrInt(const cJSON *jsonValue)
{
    size_t nInt = 0;
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        if (EventJsonUtil::CJsonIsInt(cJSON_GetArrayItem(jsonValue, i))) {
            nInt++;
        }
    }
    return nInt == jsonSize;
}

bool CheckArrInt64(const cJSON *jsonValue)
{
    size_t nInt = 0;
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        cJSON *item = cJSON_GetArrayItem(jsonValue, i);
        if (EventJsonUtil::CJsonIsInt64(item) && !EventJsonUtil::CJsonIsUint(item)) {
            nInt++;
        }
    }
    return nInt == jsonSize;
}

bool CheckArrDouble(const cJSON *jsonValue)
{
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        if (!cJSON_IsNumber(cJSON_GetArrayItem(jsonValue, i))) {
            return false;
        }
    }
    return true;
}

bool CheckArrString(const cJSON *jsonValue)
{
    size_t nString = 0;
    size_t jsonSize = cJSON_GetArraySize(jsonValue);
    for (size_t i = 0; i < jsonSize; ++i) {
        if (cJSON_IsString(cJSON_GetArrayItem(jsonValue, i))) {
            nString++;
        }
    }
    return nString == jsonSize;
}

void CreatArr(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = cJSON_GetArraySize(jsonValue);
    if (CheckArrBool(jsonValue)) {
        ConvertArrBool(retValue, jsonValue);
    } else if (CheckArrInt(jsonValue)) {
        ConvertArrInt(retValue, jsonValue);
    } else if (CheckArrDouble(jsonValue)) {
        CovertArrDouble(retValue, jsonValue);
    } else if (CheckArrString(jsonValue)) {
        CovertArrString(retValue, jsonValue);
    } else if (CheckArrInt64(jsonValue)) {
        ConvertArrInt64(retValue, jsonValue);
    } else {
        CovertArrObjStr(retValue, jsonValue);
    }
}

void CreatElmInt(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = 1;
    retValue.valueType = TYPE_INT;
    int32_t* retInt = static_cast<int32_t*>(malloc(sizeof(int32_t) * retValue.size));
    if (retInt == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    retInt[0] = static_cast<int32_t>(jsonValue->valuedouble);
    retValue.value = retInt;
}

void CreatElmInt64(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = 1;
    retValue.valueType = TYPE_INT64;
    int64_t* retInt = static_cast<int64_t*>(malloc(sizeof(int64_t) * retValue.size));
    if (retInt == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    retInt[0] = static_cast<int64_t>(jsonValue->valuedouble);
    retValue.value = retInt;
}

void CreatElmBool(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = 1;
    retValue.valueType = TYPE_BOOL;
    bool* retBool = static_cast<bool*>(malloc(sizeof(bool) * retValue.size));
    if (retBool == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    retBool[0] = cJSON_IsTrue(jsonValue);
    retValue.value = retBool;
}

void CreatElmDou(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = 1;
    retValue.valueType = TYPE_FLOAT;
    double* retF = static_cast<double*>(malloc(sizeof(double) * retValue.size));
    if (retF == nullptr) {
        LOGE("malloc is failed");
        return;
    }
    retF[0] = jsonValue->valuedouble;
    retValue.value = retF;
}

void CreatElmStr(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = 1;
    retValue.valueType = TYPE_STRING;
    retValue.value = MallocCString(jsonValue->valuestring);
}

void CreatObjStr(CParameters &retValue, const cJSON *jsonValue)
{
    retValue.size = 1;
    retValue.valueType = TYPE_STRING;
    auto itemStr = cJSON_PrintUnformatted(jsonValue);
    std::string jsonStr = itemStr;
    retValue.value = MallocCString(jsonStr);
    cJSON_free(itemStr);
}

void CreateValueByJson(CParameters &retValue, const cJSON *jsonValue)
{
    if (cJSON_IsArray(jsonValue)) {
        CreatArr(retValue, jsonValue);
    } else if (EventJsonUtil::CJsonIsInt(jsonValue)) {
        CreatElmInt(retValue, jsonValue);
    } else if (cJSON_IsBool(jsonValue)) {
        CreatElmBool(retValue, jsonValue);
    } else if (cJSON_IsNumber(jsonValue)) {
        CreatElmDou(retValue, jsonValue);
    } else if (cJSON_IsString(jsonValue)) {
        CreatElmStr(retValue, jsonValue);
    } else if (EventJsonUtil::CJsonIsInt64(jsonValue) && !EventJsonUtil::CJsonIsUint(jsonValue)) {
        CreatElmInt64(retValue, jsonValue);
    } else {
        CreatObjStr(retValue, jsonValue);
    }
}

CArrParameters CreateValueByJsonStr(const std::string& jsonStr)
{
    CArrParameters pameters{0};
    cJSON *jsonValue = cJSON_Parse(jsonStr.c_str());
    if (!jsonValue) {
        LOGE("parse event detail info failed, please check the style of json");
        return pameters;
    }

    auto eventNameList = EventJsonUtil::CJsonGetMemberNames(jsonValue);
    pameters.size = static_cast<int64_t>(eventNameList.size());
    CParameters* retValue = static_cast<CParameters*>(malloc(sizeof(CParameters) * pameters.size));
    if (retValue == nullptr) {
        LOGE("malloc is failed");
        cJSON_Delete(jsonValue);
        return pameters;
    }
    int i = 0;
    for (const auto& it : eventNameList) {
        const auto& propertyName = it;
        retValue[i].key = MallocCString(propertyName);
        CreateValueByJson(retValue[i], cJSON_GetObjectItemCaseSensitive(jsonValue, propertyName.c_str()));
        ++i;
    }
    pameters.head = retValue;
    cJSON_Delete(jsonValue);
    return pameters;
}

void FreeRetValue(RetAppEventGroup* retValue, size_t index)
{
    if (retValue == nullptr) {
        return;
    }
    for (size_t i = 0; i < index; i++) {
        free(retValue[i].name);
        if (retValue[i].appEventInfos.head == nullptr) {
            continue;
        }
        for (int64_t j = 0; j < retValue[i].appEventInfos.size; j++) {
            FreeCAppEventInfo(retValue[i].appEventInfos.head[j]);
        }
    }
    free(retValue);
    LOGE("malloc is failed");
}

CArrRetAppEventGroup getEventGroups(const std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>>& events)
{
    CArrRetAppEventGroup eventGroups;
    std::unordered_map<std::string, std::vector<std::shared_ptr<AppEventPack>>> eventMap;
    for (const auto& event : events) {
        eventMap[event->GetName()].emplace_back(event);
    }
    eventGroups.size = static_cast<int64_t>(eventMap.size());
    eventGroups.head = nullptr;
    if (eventGroups.size > 0) {
        RetAppEventGroup* retValue1 = static_cast<RetAppEventGroup*>
                                        (malloc(sizeof(RetAppEventGroup) * eventGroups.size));
        if (retValue1 == nullptr) {
            LOGE("malloc is failed");
            return eventGroups;
        }
        size_t index = 0;
        bool fail = false;
        for (const auto& it : eventMap) {
            retValue1[index].name = MallocCString(it.first);
            CArrAppEventInfo appEventInfos;
            appEventInfos.size = static_cast<int64_t>(it.second.size());
            CAppEventInfo* retValue2 = static_cast<CAppEventInfo*>(malloc(sizeof(CAppEventInfo)
                                        * it.second.size()));
            if (retValue2 == nullptr) {
                free(retValue1[index].name);
                fail = true;
                break;
            }
            for (size_t i = 0; i < it.second.size(); ++i) {
                retValue2[i].domain = MallocCString(it.second[i]->GetDomain());
                retValue2[i].name = MallocCString(it.second[i]->GetName());
                retValue2[i].event = it.second[i]->GetType();
                retValue2[i].cArrParamters = CreateValueByJsonStr(it.second[i]->GetParamStr());
            }
            appEventInfos.head = retValue2;
            retValue1[index++].appEventInfos = appEventInfos;
        }
        if (fail) {
            FreeRetValue(retValue1, index);
            return CArrRetAppEventGroup{0};
        }
        eventGroups.head = retValue1;
    }
    return eventGroups;
}

void AppEventWatcherImpl::OnEvents(const std::vector<std::shared_ptr<OHOS::HiviewDFX::AppEventPack>>& events)
{
    if (context_ == nullptr || context_->receiveContext == nullptr) {
        LOGE("onReceive context is null");
        return;
    }
    if (events.empty()) {
        return;
    }
    context_->receiveContext->domain = events[0]->GetDomain();
    context_->receiveContext->events = events;
    CArrRetAppEventGroup eventGroups = getEventGroups(events);
    if (eventGroups.head == nullptr) {
        return;
    }
    char* cjDomain = MallocCString(context_->receiveContext->domain);
    if (cjDomain == nullptr) {
        free(eventGroups.head);
        LOGE("malloc is failed");
        return;
    }
    context_->receiveContext->onReceive(cjDomain, eventGroups);
}

void AppEventWatcherImpl::OnTrigger(const HiviewDFX::TriggerCondition& triggerCond)
{
    if (context_ == nullptr || context_->triggerContext == nullptr) {
        LOGE("onTrigger context is null");
        return;
    }
    context_->triggerContext->row = triggerCond.row;
    context_->triggerContext->size = triggerCond.size;
    context_->triggerContext->onTrigger(triggerCond.row, triggerCond.size,
                                        context_->triggerContext->holder->GetID());
}

bool AppEventWatcherImpl::IsRealTimeEvent(std::shared_ptr<OHOS::HiviewDFX::AppEventPack> event)
{
    return (context_ != nullptr && context_->receiveContext != nullptr);
}
} // HiAppEvent
} // CJSystemapi
} // OHOS

