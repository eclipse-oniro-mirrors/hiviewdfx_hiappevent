/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "hiappevent_c.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "ndk_app_event_processor_service.h"
#include "ndk_app_event_watcher_service.h"

ParamList OH_HiAppEvent_CreateParamList(void)
{
    return HiAppEventCreateParamList();
}

void OH_HiAppEvent_DestroyParamList(ParamList list)
{
    HiAppEventDestroyParamList(list);
}

ParamList OH_HiAppEvent_AddBoolParam(ParamList list, const char* name, bool boolean)
{
    return AddBoolParamValue(list, name, boolean);
}

ParamList OH_HiAppEvent_AddBoolArrayParam(ParamList list, const char* name, const bool* booleans, int arrSize)
{
    return AddBoolArrayParamValue(list, name, booleans, arrSize);
}

ParamList OH_HiAppEvent_AddInt8Param(ParamList list, const char* name, int8_t num)
{
    return AddInt8ParamValue(list, name, num);
}

ParamList OH_HiAppEvent_AddInt8ArrayParam(ParamList list, const char* name, const int8_t* nums, int arrSize)
{
    return AddInt8ArrayParamValue(list, name, nums, arrSize);
}

ParamList OH_HiAppEvent_AddInt16Param(ParamList list, const char* name, int16_t num)
{
    return AddInt16ParamValue(list, name, num);
}

ParamList OH_HiAppEvent_AddInt16ArrayParam(ParamList list, const char* name, const int16_t* nums, int arrSize)
{
    return AddInt16ArrayParamValue(list, name, nums, arrSize);
}

ParamList OH_HiAppEvent_AddInt32Param(ParamList list, const char* name, int32_t num)
{
    return AddInt32ParamValue(list, name, num);
}

ParamList OH_HiAppEvent_AddInt32ArrayParam(ParamList list, const char* name, const int32_t* nums, int arrSize)
{
    return AddInt32ArrayParamValue(list, name, nums, arrSize);
}

ParamList OH_HiAppEvent_AddInt64Param(ParamList list, const char* name, int64_t num)
{
    return AddInt64ParamValue(list, name, num);
}

ParamList OH_HiAppEvent_AddInt64ArrayParam(ParamList list, const char* name, const int64_t* nums, int arrSize)
{
    return AddInt64ArrayParamValue(list, name, nums, arrSize);
}

ParamList OH_HiAppEvent_AddFloatParam(ParamList list, const char* name, float num)
{
    return AddFloatParamValue(list, name, num);
}

ParamList OH_HiAppEvent_AddFloatArrayParam(ParamList list, const char* name, const float* nums, int arrSize)
{
    return AddFloatArrayParamValue(list, name, nums, arrSize);
}

ParamList OH_HiAppEvent_AddDoubleParam(ParamList list, const char* name, double num)
{
    return AddDoubleParamValue(list, name, num);
}

ParamList OH_HiAppEvent_AddDoubleArrayParam(ParamList list, const char* name, const double* nums, int arrSize)
{
    return AddDoubleArrayParamValue(list, name, nums, arrSize);
}

ParamList OH_HiAppEvent_AddStringParam(ParamList list, const char* name, const char* str)
{
    return AddStringParamValue(list, name, str);
}

ParamList OH_HiAppEvent_AddStringArrayParam(ParamList list, const char* name, const char* const *strs, int arrSize)
{
    return AddStringArrayParamValue(list, name, strs, arrSize);
}

bool OH_HiAppEvent_Configure(const char* name, const char* value)
{
    return HiAppEventInnerConfigure(name, value);
}

struct HiAppEvent_Watcher* OH_HiAppEvent_CreateWatcher(const char *name)
{
    return CreateWatcher(name);
}

int OH_HiAppEvent_SetAppEventFilter(struct HiAppEvent_Watcher *watcher, const char *domain, uint8_t eventTypes,
                                    const char *const *names, int namesLen)
{
    return SetAppEventFilter(watcher, domain, eventTypes, names, namesLen);
}

int OH_HiAppEvent_SetTriggerCondition(struct HiAppEvent_Watcher* watcher, int row, int size, int timeOut)
{
    return SetTriggerCondition(watcher, row, size, timeOut);
}

int OH_HiAppEvent_SetWatcherOnTrigger(struct HiAppEvent_Watcher *watcher, OH_HiAppEvent_OnTrigger onTrigger)
{
    return SetWatcherOnTrigger(watcher, onTrigger);
}

int OH_HiAppEvent_SetWatcherOnReceive(struct HiAppEvent_Watcher *watcher, OH_HiAppEvent_OnReceive onReceiver)
{
    return SetWatcherOnReceiver(watcher, onReceiver);
}

int OH_HiAppEvent_AddWatcher(struct HiAppEvent_Watcher* watcher)
{
    return AddWatcher(watcher);
}

int OH_HiAppEvent_TakeWatcherData(struct HiAppEvent_Watcher *watcher, uint32_t size, OH_HiAppEvent_OnTake onTake)
{
    return TakeWatcherData(watcher, size, onTake);
}

void OH_HiAppEvent_ClearData()
{
    ClearData();
}

int OH_HiAppEvent_RemoveWatcher(struct HiAppEvent_Watcher *watcher)
{
    return RemoveWatcher(watcher);
}

void OH_HiAppEvent_DestroyWatcher(struct HiAppEvent_Watcher *watcher)
{
    DestroyWatcher(watcher);
}

int OH_HiAppEvent_Write(const char* domain, const char* name, enum EventType type, const ParamList list)
{
    return HiAppEventInnerWrite(domain, name, type, list);
}

struct HiAppEvent_Processor* OH_HiAppEvent_CreateProcessor(const char* name)
{
    return CreateProcessor(name);
}

int OH_HiAppEvent_SetReportRoute(struct HiAppEvent_Processor* processor, const char* appId, const char* routeInfo)
{
    return SetReportRoute(processor, appId, routeInfo);
}

int OH_HiAppEvent_SetReportPolicy(struct HiAppEvent_Processor* processor, int periodReport, int batchReport,
    bool onStartReport, bool onBackgroundReport)
{
    return SetReportPolicy(processor, periodReport, batchReport, onStartReport, onBackgroundReport);
}

int OH_HiAppEvent_SetReportEvent(struct HiAppEvent_Processor* processor, const char* domain, const char* name,
    bool isRealTime)
{
    return SetReportEvent(processor, domain, name, isRealTime);
}

int OH_HiAppEvent_SetCustomConfig(struct HiAppEvent_Processor* processor, const char* key, const char* value)
{
    return SetCustomConfig(processor, key, value);
}

int OH_HiAppEvent_SetConfigId(struct HiAppEvent_Processor* processor, int configId)
{
    return SetConfigId(processor, configId);
}

int OH_HiAppEvent_SetConfigName(struct HiAppEvent_Processor* processor, const char* configName)
{
    return SetConfigName(processor, configName);
}

int OH_HiAppEvent_SetReportUserId(struct HiAppEvent_Processor* processor, const char* const * userIdNames, int size)
{
    return SetReportUserId(processor, userIdNames, size);
}

int OH_HiAppEvent_SetReportUserProperty(struct HiAppEvent_Processor* processor, const char* const * userPropertyNames,
    int size)
{
    return SetReportUserProperty(processor, userPropertyNames, size);
}

int64_t OH_HiAppEvent_AddProcessor(struct HiAppEvent_Processor* processor)
{
    return AddProcessor(processor);
}

void OH_HiAppEvent_DestoryProcessor(struct HiAppEvent_Processor* processor)
{
    DestroyProcessor(processor);
}

void OH_HiAppEvent_DestroyProcessor(struct HiAppEvent_Processor* processor)
{
    DestroyProcessor(processor);
}

int OH_HiAppEvent_RemoveProcessor(int64_t processorId)
{
    return RemoveProcessor(processorId);
}

HiAppEvent_Config* OH_HiAppEvent_CreateConfig(void)
{
    return HiAppEventCreateConfig();
}

int OH_HiAppEvent_SetConfigItem(HiAppEvent_Config* config, const char* itemName, const char* itemValue)
{
    return HiAppEventSetConfigItem(config, itemName, itemValue);
}

int OH_HiAppEvent_SetEventConfig(const char* name, HiAppEvent_Config* config)
{
    return HiAppEventSetEventConfig(name, config);
}

void OH_HiAppEvent_DestroyConfig(HiAppEvent_Config* config)
{
    HiAppEventDestroyConfig(config);
}