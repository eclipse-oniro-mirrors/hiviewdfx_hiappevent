/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_HIAPPEVENT_H
#define HIVIEWDFX_HIAPPEVENT_H
/**
 * @addtogroup HiAppEvent
 * @{
 *
 * @brief Provides application event logging functions.
 *
 * Provides the event logging function for applications to log the fault, statistical, security, and user behavior
 * events reported during running. Based on event information, you will be able to analyze the running status of
 * applications.
 *
 * @syscap SystemCapability.HiviewDFX.HiAppEvent
 *
 * @since 8
 * @version 1.0
 */

/**
 * @file hiappevent.h
 *
 * @brief Defines the application event logging functions of the HiAppEvent module.
 *
 * Before performing application event logging, you must construct a ParamList object to store the input
 * event parameters and specify the event domain, event name, and event type.
 *
 * <p>Event domain: a string used to identify the domain of event logging.
 * <p>Event name: a string used to identify the event name of event logging.
 * <p>Event type: FAULT, STATISTIC, SECURITY, BEHAVIOR.
 * <p>ParamList: a linked list used to store event parameters, each of which is comprised of the parameter name and
 * parameter value.
 *
 * Sample code:
 * 00 Including the header file:
 * <pre>
 *     #include "hiappevent/hiappevent.h"
 * </pre>
 * 01 create a ParamList pointer.
 * <pre>
 *     ParamList list = OH_HiAppEvent_CreateParamList();
 * </pre>
 * 02 add params to the ParamList.
 * <pre>
 *     bool boolean = true;
 *     OH_HiAppEvent_AddBoolParam(list, "bool_key", boolean);
 *     int32_t nums[] = {1, 2, 3};
 *     OH_HiAppEvent_AddInt32ArrayParam(list, "int32_arr_key", nums, sizeof(nums) / sizeof(nums[0]));
 * </pre>
 * 03 performing event logging.
 * <pre>
 *     int res = OH_HiAppEvent_Write("test_domain", "test_event", BEHAVIOR, list);
 * </pre>
 * 04 destroy the ParamList pointer.
 * <pre>
 *     OH_HiAppEvent_DestroyParamList(list);
 * </pre>
 *
 * @since 8
 * @version 1.0
 */

#include <stdbool.h>
#include <stdint.h>

#include "hiappevent_cfg.h"
#include "hiappevent_event.h"
#include "hiappevent_param.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event types.
 *
 * You are advised to select event types based on their respective usage scenarios.
 *
 * @since 8
 * @version 1.0
 */
enum EventType {
    /* Fault event type */
    FAULT = 1,

    /* Statistic event type */
    STATISTIC = 2,

    /* Security event type */
    SECURITY = 3,

    /* Behavior event type */
    BEHAVIOR = 4
};

/**
 * @brief Definition of HiAppEvent_AppEventInfo object.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @since 12
 * @version 1.0
 */
struct HiAppEvent_AppEventInfo {
    /* The domain of the event. */
    const char* domain;
	/* The name of the event. */
    const char* name;
	/* The type of the event. */
    enum EventType type;
	/* The json string of the parameter. */
    const char* params;
};

/**
 * @brief Defines the event group.
 *
 * @syscap SystemCapability.HiviewDFX.HiAppEvent
 * @since 12
 * @version 1.0
 */
struct HiAppEvent_AppEventGroup {
	/* The name of the event. */
    const char* name;
	/* The event array which is group by the name. */
    const struct HiAppEvent_AppEventInfo* appEventInfos;
	/* The length of appEventInfos array. */
    uint32_t infoLen;
};

/**
 * @brief Event param list node.
 *
 * @since 8
 * @version 1.0
 */
typedef struct ParamListNode* ParamList;

/**
 * @brief HiAppEvent_Watcher node.
 *
 * @syscap SystemCapability.HiviewDFX.HiAppEvent
 * @since 12
 * @version 1.0
 */
typedef struct HiAppEvent_Watcher HiAppEvent_Watcher;

/**
 * @brief Called when watcher receive the event.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param domain The domain of the event.
 * @param appEventGroups The event group by the domain.
 * @param groupLen The length of appEventGroups array.
 * @since 12
 * @version 1.0
 */
typedef void (*OH_HiAppEvent_OnReceive)(
    const char* domain, const struct HiAppEvent_AppEventGroup* appEventGroups, uint32_t groupLen);

/**
 * @brief Called when watcher receive the event meet the condition.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param row The row of events received by watcher.
 * @param size The size of events received by watcher.
 * @since 12
 * @version 1.0
 */
typedef void (*OH_HiAppEvent_OnTrigger)(int32_t row, int32_t size);

/**
 * @brief Called when watcher take the events.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param events The event json string array.
 * @param eventLen The length of events array.
 * @since 12
 * @version 1.0
 */
typedef void (*OH_HiAppEvent_OnTake)(const char * const * events, int32_t eventLen);

/**
 * @brief Create a pointer to the ParamList.
 *
 * @return Pointer to the ParamList.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_CreateParamList(void);

/**
 * @brief Destroy a pointer to the ParamList.
 *
 * @param list Event param list.
 * @since 8
 * @version 1.0
 */
void OH_HiAppEvent_DestroyParamList(ParamList list);

/**
 * @brief Add bool param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param boolean The bool value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddBoolParam(ParamList list, const char* name, bool boolean);

/**
 * @brief Add bool array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param booleans The bool array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddBoolArrayParam(ParamList list, const char* name, const bool* booleans, int arrSize);

/**
 * @brief Add int8_t param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param num The int8_t value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt8Param(ParamList list, const char* name, int8_t num);

/**
 * @brief Add int8_t array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param nums The int8_t array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt8ArrayParam(ParamList list, const char* name, const int8_t* nums, int arrSize);

/**
 * @brief Add int16_t param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param num The int16_t value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt16Param(ParamList list, const char* name, int16_t num);

/**
 * @brief Add int16_t array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param nums The int16_t array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt16ArrayParam(ParamList list, const char* name, const int16_t* nums, int arrSize);

/**
 * @brief Add int32_t param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param num The int32_t value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt32Param(ParamList list, const char* name, int32_t num);

/**
 * @brief Add int32_t array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param nums The int32_t array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt32ArrayParam(ParamList list, const char* name, const int32_t* nums, int arrSize);

/**
 * @brief Add int64_t param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param num The int64_t value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt64Param(ParamList list, const char* name, int64_t num);

/**
 * @brief Add int64_t array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param nums The int64_t array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddInt64ArrayParam(ParamList list, const char* name, const int64_t* nums, int arrSize);

/**
 * @brief Add float param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param num The float value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddFloatParam(ParamList list, const char* name, float num);

/**
 * @brief Add float array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param nums The float array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddFloatArrayParam(ParamList list, const char* name, const float* nums, int arrSize);

/**
 * @brief Add double param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param num The double value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddDoubleParam(ParamList list, const char* name, double num);

/**
 * @brief Add double array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param nums The double array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddDoubleArrayParam(ParamList list, const char* name, const double* nums, int arrSize);

/**
 * @brief Add string param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param str The string value of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddStringParam(ParamList list, const char* name, const char* str);

/**
 * @brief Add string array param to the ParamList.
 *
 * @param list The ParamList of params to be added.
 * @param name The name of the param to be added.
 * @param strs The string array value of the param to be added.
 * @param arrSize The array size of the param to be added.
 * @return ParamList after the param is added.
 * @since 8
 * @version 1.0
 */
ParamList OH_HiAppEvent_AddStringArrayParam(ParamList list, const char* name, const char * const *strs, int arrSize);

/**
 * @brief Implements logging of application events whose parameters are of the list type.
 *
 * Before logging an application event, this API will first verify parameters of this event.
 * If the verification is successful, the API will write the event to the event file.
 *
 * @param domain Indicates the event domain. You can customize the event domain as needed.
 * @param name Indicates the event name. You can customize the event name as needed.
 * @param type Indicates the event type, which is defined in {@link EventType}.
 * @param list Indicates a linked list of event parameters, each of which is comprised of the parameter name and
 * parameter value.
 * @return Returns {@code 0} if the event parameter verification is successful, and the event will be written to
 * the event file; returns a positive integer if invalid parameters are present in the event, and
 * the event will be written to the event file after the invalid parameters are ignored; returns a
 * negative integer if the event parameter verification fails, and the event will not be written to the event file.
 * @since 8
 * @version 1.0
 */
int OH_HiAppEvent_Write(const char* domain, const char* name, enum EventType type, const ParamList list);

/**
 * @brief Implements the configuration function of application events logging.
 *
 * Application event logging configuration interface, which is used to configure event logging switch,
 * event file directory storage quota size and other functions.
 *
 * @param name Configuration item name.
 * @param value Configuration item value.
 * @return Configuration result.
 * @since 8
 * @version 1.0
 */
bool OH_HiAppEvent_Configure(const char* name, const char* value);

/**
 * @brief Create a HiAppEvent_Watcher handler pointer to set the property.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param name The name of the watcher.
 * @return Returns a pointer to the HiAppEvent_Watcher instance.
 * @since 12
 * @version 1.0
 */
HiAppEvent_Watcher* OH_HiAppEvent_CreateWatcher(const char* name);

/**
 * @brief Destroy the specified HiAppEvent_Watcher handle resource.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @since 12
 * @version 1.0
 */
void OH_HiAppEvent_DestroyWatcher(HiAppEvent_Watcher* watcher);

/**
 * @brief Set the TriggerCondition to trigger the onTrigger callback.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @param row The row of write events that trigger the onTrigger callback.
 * @param size The size of write events that trigger the onTrigger callback.
 * @param timeOut The interval for trigger the onTrigger callback.
 * @return Returns {@code 0} if set TriggerCondition is successful, and returns a
 * negative integer if set fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_SetTriggerCondition(HiAppEvent_Watcher* watcher, uint32_t row, uint32_t size, uint32_t timeOut);

/**
 * @brief Set the AppEventFilter, which is used to filter events monitored by the watcher.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @param domain The name of the event domain to be monitored by the watcher..
 * @param eventTypes The types of the events to be monitored by the watcher.0x08 means BEHAVIOR,0x04 means
 * SECURITY, 0x02 means STATISTIC,0x01 means FAULT, 0xff and 0x00 means all.
 * @param names The names of the events to be monitored by the watcher.
 * @param namesLen The length of names array.
 * @return Returns {@code 0} if set AppEventFilter is successful, and returns a
 * negative integer if set fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_SetAppEventFilter(HiAppEvent_Watcher* watcher, const char* domain, uint8_t eventTypes,
                                    const char* const *names, int namesLen);

/**
 * @brief Set the onTrigger callback.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @param onTrigger The callback of the watcher.
 * @return Returns {@code 0} if set OnTrigger is successful, and returns a
 * negative integer if set fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_SetWatcherOnTrigger(HiAppEvent_Watcher* watcher, OH_HiAppEvent_OnTrigger onTrigger);

/**
 * @brief Set the OnReceive callback.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @param onReceive The callback of the watcher.
 * @return Returns {@code 0} if set OnReceive is successful, and returns a
 * negative integer if set fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_SetWatcherOnReceive(HiAppEvent_Watcher* watcher, OH_HiAppEvent_OnReceive onReceive);

/**
 * @brief Set the AppEventFilter, which is used to filter events monitored by the watcher.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @param size The the threshold size per read.
 * @param onTake The callback of the watcher.
 * @return Returns {@code 0} if remove watcher is successful, and returns a
 * negative integer if remove fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_TakeWatcherData(HiAppEvent_Watcher* watcher, uint32_t size, OH_HiAppEvent_OnTake onTake);

/**
 * @brief Add watcher to receive the event.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance which receive the event.
 * @return Returns {@code 0} if add watcher is successful, and returns a
 * negative integer if add fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_AddWatcher(HiAppEvent_Watcher* watcher);

/**
 * @brief Remove watcher.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @param watcher The pointer to the HiAppEvent_Watcher instance.
 * @return Returns {@code 0} if remove watcher is successful, and returns a
 * negative integer if remove fail.
 * @since 12
 * @version 1.0
 */
int OH_HiAppEvent_RemoveWatcher(HiAppEvent_Watcher* watcher);

/**
 * @brief Clear all local logging data of the application.
 *
 * @SystemCapability.HiviewDFX.HiAppEvent
 * @since 12
 * @version 1.0
 */
void OH_HiAppEvent_ClearData();
#ifdef __cplusplus
}
#endif
/** @} */
#endif // HIVIEWDFX_HIAPPEVENT_H