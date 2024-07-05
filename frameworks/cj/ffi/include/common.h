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

#ifndef OHOS_HIAPPEVENT_COMMON_H
#define OHOS_HIAPPEVENT_COMMON_H

#include "cj_ffi/cj_common_ffi.h"

extern "C" {
    struct CConfigOption {
        bool disable = false;
        const char* maxStorage = nullptr;
    };

    struct CParameters {
        uint8_t valueType = 0;
        char* key = nullptr;
        void* value = nullptr;
        int64_t size = 0;
    };

    struct CArrParameters {
        CParameters* head = nullptr;
        int64_t size = 0;
    };

    struct CAppEventInfo {
        const char* domain = nullptr;
        const char* name = nullptr;
        int event = 0;
        CArrParameters cArrParamters;
    };

    struct CAppEventReportConfig {
        const char* domain = nullptr;
        const char* name = nullptr;
        bool isRealTime = false;
    };

    struct CArrAppEventReportConfig {
        CAppEventReportConfig* head = nullptr;
        int64_t size = 0;
    };

    struct CProcessor {
        const char* name = nullptr;
        bool debugMode = false;
        const char* routeInfo = nullptr;
        const char* appId = nullptr;
        bool onStartReport = false;
        bool onBackgroundReport = false;
        int64_t periodReport = 0;
        int64_t batchReport = 0;
        CArrString userIds;
        CArrString userProperties;
        CArrAppEventReportConfig eventConfigs;
    };

    struct CAppEventFilter {
        char* domain = nullptr;
        CArrUI32 eventTypes;
        CArrString names;
    };

    struct CArrAppEventFilter {
        CAppEventFilter* head = nullptr;
        int64_t size = 0;
    };

    struct CTriggerCondition {
        int row = 0;
        int size = 0;
        int timeOut = 0;
    };

    struct CArrAppEventInfo {
        CAppEventInfo* head = nullptr;
        int64_t size = 0;
    };

    struct RetAppEventGroup {
        char* name = nullptr;
        CArrAppEventInfo appEventInfos;
    };

    struct CArrRetAppEventGroup {
        RetAppEventGroup* head = nullptr;
        int64_t size = 0;
    };

    struct RetAppEventPackage {
        int packageId;
        int row;
        int size;
        CArrString events;
    };

    struct ReTakeNext {
        int status;
        RetAppEventPackage event;
    };

    struct CWatcher {
        char* name = nullptr;
        CTriggerCondition triggerCondition;
        CArrAppEventFilter appEventFilters;
        void (*callbackOnTriggerRef)(int, int, int64_t);
        void (*callbackOnReceiveRef)(char*, CArrRetAppEventGroup);
    };
}

#endif