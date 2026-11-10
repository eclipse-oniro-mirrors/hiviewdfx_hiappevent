/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef NDK_FUZZER_FUZZ_CASES_H
#define NDK_FUZZER_FUZZ_CASES_H

#include <cstddef>
#include <cstdint>

void FuzzHiappeventNdk(const uint8_t* data, size_t size);
void FuzzNdkAppEventProcessor(const uint8_t* data, size_t size);
void FuzzNdkAppEventProcessorService(const uint8_t* data, size_t size);
void FuzzNdkAppEventWatcher(const uint8_t* data, size_t size);
void FuzzNdkAppEventWatcherProxy(const uint8_t* data, size_t size);
void FuzzNdkAppEventWatcherService(const uint8_t* data, size_t size);
void FuzzNdkExternalLogCallback(const uint8_t* data, size_t size);
void FuzzNdkExternalLogService(const uint8_t* data, size_t size);

#endif // NDK_FUZZER_FUZZ_CASES_H
