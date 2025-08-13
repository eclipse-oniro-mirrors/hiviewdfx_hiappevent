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

#ifndef HIAPPEVENT_C_H
#define HIAPPEVENT_C_H

#include "hiappevent/hiappevent.h"

#ifdef __cplusplus
extern "C" {
#endif
ParamList HiAppEventCreateParamList();
void HiAppEventDestroyParamList(ParamList list);

ParamList AddBoolParamValue(ParamList list, const char* name, bool boolean);
ParamList AddBoolArrayParamValue(ParamList list, const char* name, const bool* booleans, int arrSize);
ParamList AddInt8ParamValue(ParamList list, const char* name, int8_t num);
ParamList AddInt8ArrayParamValue(ParamList list, const char* name, const int8_t* nums, int arrSize);
ParamList AddInt16ParamValue(ParamList list, const char* name, int16_t num);
ParamList AddInt16ArrayParamValue(ParamList list, const char* name, const int16_t* nums, int arrSize);
ParamList AddInt32ParamValue(ParamList list, const char* name, int32_t num);
ParamList AddInt32ArrayParamValue(ParamList list, const char* name, const int32_t* nums, int arrSize);
ParamList AddInt64ParamValue(ParamList list, const char* name, int64_t num);
ParamList AddInt64ArrayParamValue(ParamList list, const char* name, const int64_t* nums, int arrSize);
ParamList AddFloatParamValue(ParamList list, const char* name, float num);
ParamList AddFloatArrayParamValue(ParamList list, const char* name, const float* nums, int arrSize);
ParamList AddDoubleParamValue(ParamList list, const char* name, double num);
ParamList AddDoubleArrayParamValue(ParamList list, const char* name, const double* nums, int arrSize);
ParamList AddStringParamValue(ParamList list, const char* name, const char* str);
ParamList AddStringArrayParamValue(ParamList list, const char* name, const char* const *strs, int arrSize);

bool HiAppEventInnerConfigure(const char* name, const char* value);

int HiAppEventInnerWrite(const char* domain, const char* name, enum EventType type, const ParamList list);

void ClearData();

HiAppEvent_Config* HiAppEventCreateConfig();
int HiAppEventSetConfigItem(HiAppEvent_Config* config, const char* itemName, const char* itemValue);
int HiAppEventSetEventConfig(const char* name, HiAppEvent_Config* config);
void HiAppEventDestroyConfig(HiAppEvent_Config* config);
#ifdef __cplusplus
}
#endif
#endif // HIAPPEVENT_C_H
