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

#ifndef HIAPPEVENT_TEST_FUZZTEST_INIT_CACHE_TEST_H
#define HIAPPEVENT_TEST_FUZZTEST_INIT_CACHE_TEST_H

namespace OHOS {
namespace HiviewDFX {
class AppEventStoreInit {
public:
    static AppEventStoreInit& GetInstance();

private:
    AppEventStoreInit();
    AppEventStoreInit(const AppEventStoreInit&) = delete;
    AppEventStoreInit& operator=(const AppEventStoreInit&) = delete;
};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_TEST_FUZZTEST_INIT_CACHE_TEST_H
