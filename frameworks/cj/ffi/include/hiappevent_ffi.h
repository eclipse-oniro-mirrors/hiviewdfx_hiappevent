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

#ifndef HIAPPEVENT_FFI_H
#define HIAPPEVENT_FFI_H

#include <cstdint>

#include "cj_ffi/cj_common_ffi.h"
#include "common.h"

extern "C" {
    FFI_EXPORT int FfiOHOSHiAppEventConfigure(CConfigOption config);
    FFI_EXPORT int FfiOHOSHiAppEventWrite(CAppEventInfo info);
    FFI_EXPORT RetDataBool FfiOHOSHiAppEventAddProcessor(CProcessor processor);
    FFI_EXPORT int FfiOHOSHiAppEventSetUserId(const char* name, const char* value);
    FFI_EXPORT RetDataCString FfiOHOSHiAppEventGetUserId(const char* name);
    FFI_EXPORT int FfiOHOSHiAppEventSetUserProperty(const char* name, const char* value);
    FFI_EXPORT RetDataCString FfiOHOSHiAppEventgetUserProperty(const char* name);
    FFI_EXPORT int64_t FfiOHOSHiAppEventConstructor(char* cWatcherName);
    FFI_EXPORT int FfiOHOSHiAppEventSetSize(int64_t id, int size);
    FFI_EXPORT ReTakeNext FfiOHOSHiAppEventTakeNext(int64_t id);
    FFI_EXPORT RetDataI64 FfiOHOSHiAppEventAddWatcher(CWatcher watcher);
    FFI_EXPORT int FfiOHOSHiAppEventRemoveWatcher(CWatcher watcher);
}

#endif