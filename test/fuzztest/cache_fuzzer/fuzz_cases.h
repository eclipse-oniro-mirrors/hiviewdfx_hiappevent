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

#ifndef CACHE_FUZZER_FUZZ_CASES_H
#define CACHE_FUZZER_FUZZ_CASES_H

#include <cstddef>
#include <cstdint>

void FuzzAppEventStore(const uint8_t* data, size_t size);
void FuzzAppEventDao(const uint8_t* data, size_t size);
void FuzzAppEventObserverDao(const uint8_t* data, size_t size);
void FuzzAppEventMappingDao(const uint8_t* data, size_t size);
void FuzzUserIdDao(const uint8_t* data, size_t size);
void FuzzUserPropertyDao(const uint8_t* data, size_t size);
void FuzzCustomEventParamDao(const uint8_t* data, size_t size);
void FuzzApiStatsDao(const uint8_t* data, size_t size);

#endif // CACHE_FUZZER_FUZZ_CASES_H
