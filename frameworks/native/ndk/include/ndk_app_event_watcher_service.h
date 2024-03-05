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

#ifndef HIAPPEVENT_NDK_APP_EVENT_WATCHER_SERVICE_H
#define HIAPPEVENT_NDK_APP_EVENT_WATCHER_SERVICE_H

#include <stdint.h>

#include "hiappevent/hiappevent.h"

#ifdef __cplusplus
extern "C" {
#endif

struct HiAppEvent_Watcher* CreateWatcher(const char *name);
int SetAppEventFilter(struct HiAppEvent_Watcher* watcher, const char* domain, uint8_t eventTypes,
                      const char* const *names, int namesLen);

int SetTriggerCondition(struct HiAppEvent_Watcher* watcher, int row, int size, int timeOut);

int SetWatcherOnTrigger(struct HiAppEvent_Watcher* watcher, OH_HiAppEvent_OnTrigger onTrigger);

int SetWatcherOnReceiver(struct HiAppEvent_Watcher* watcher, OH_HiAppEvent_OnReceive onReceiver);
int AddWatcher(struct HiAppEvent_Watcher* watcher);
int TakeWatcherData(struct HiAppEvent_Watcher* watcher, uint32_t size, OH_HiAppEvent_OnTake onTake);
int RemoveWatcher(struct HiAppEvent_Watcher *watcher);
void DestroyWatcher(struct HiAppEvent_Watcher* watcher);
#ifdef __cplusplus
}
#endif

#endif // HIAPPEVENT_NDK_APP_EVENT_WATCHER_SERVICE_H
