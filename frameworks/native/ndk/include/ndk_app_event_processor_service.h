/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef HIAPPEVENT_NDK_APP_EVENT_PROCESSOR_SERVICE_H
#define HIAPPEVENT_NDK_APP_EVENT_PROCESSOR_SERVICE_H

#include <stdint.h>

#include "hiappevent/hiappevent.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HiAppEvent_Processor* CreateProcessor(const char* name);
int SetReportRoute(struct HiAppEvent_Processor* processor, const char* appId, const char* routeInfo);
int SetReportPolicy(struct HiAppEvent_Processor* processor, int periodReport, int batchReport, bool onStartReport,
    bool onBackgroundReport);
int SetReportEvent(struct HiAppEvent_Processor* processor, const char* domain, const char* name, bool isRealTime);
int SetCustomConfig(struct HiAppEvent_Processor* processor, const char* key, const char* value);
int SetConfigId(struct HiAppEvent_Processor* processor, int configId);
int SetConfigName(struct HiAppEvent_Processor* processor, const char* configName);
int SetReportUserId(struct HiAppEvent_Processor* processor, const char* const * userIdNames, int size);
int SetReportUserProperty(struct HiAppEvent_Processor* processor, const char* const * userPropertyNames, int size);
int64_t AddProcessor(struct HiAppEvent_Processor* processor);
void DestroyProcessor(struct HiAppEvent_Processor* processor);
int RemoveProcessor(int64_t processorId);
#ifdef __cplusplus
}
#endif

#endif // HIAPPEVENT_NDK_APP_EVENT_PROCESSOR_SERVICE_H
