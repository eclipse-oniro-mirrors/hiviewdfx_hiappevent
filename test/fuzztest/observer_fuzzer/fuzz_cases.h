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

#ifndef OBSERVER_FUZZER_FUZZ_CASES_H
#define OBSERVER_FUZZER_FUZZ_CASES_H

#include <cstddef>
#include <cstdint>

void FuzzAppEventObserver(const uint8_t* data, size_t size);
void FuzzAppEventObserverMgr(const uint8_t* data, size_t size);
void FuzzAppEventProcessorProxy(const uint8_t* data, size_t size);
void FuzzAppEventWatcher(const uint8_t* data, size_t size);
void FuzzAppEventExternalLogManager(const uint8_t* data, size_t size);
void FuzzOsEventListener(const uint8_t* data, size_t size);
void FuzzAppStateCallback(const uint8_t* data, size_t size);

#endif // OBSERVER_FUZZER_FUZZ_CASES_H
