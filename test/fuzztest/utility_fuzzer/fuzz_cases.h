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

#ifndef UTILITY_FUZZER_FUZZ_CASES_H
#define UTILITY_FUZZER_FUZZ_CASES_H

#include <cstddef>
#include <cstdint>

void FuzzFileUtil(const uint8_t* data, size_t size);
void FuzzTimeUtil(const uint8_t* data, size_t size);
void FuzzAppEventStat(const uint8_t* data, size_t size);
void FuzzApiCallScopedStat(const uint8_t* data, size_t size);
void FuzzEventJsonUtil(const uint8_t* data, size_t size);
void FuzzSqlUtil(const uint8_t* data, size_t size);

#endif // UTILITY_FUZZER_FUZZ_CASES_H
