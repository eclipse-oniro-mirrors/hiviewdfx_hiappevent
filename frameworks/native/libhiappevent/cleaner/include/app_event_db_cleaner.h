/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CLEANER_APP_EVENT_DB_CLEANER_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CLEANER_APP_EVENT_DB_CLEANER_H

#include "app_event_cleaner.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventDbCleaner : public AppEventCleaner {
public:
    AppEventDbCleaner(const std::string& path) : AppEventCleaner(path) {}
    ~AppEventDbCleaner() {}
    uint64_t GetFilesSize() override;
    uint64_t ClearSpace(uint64_t curSize, uint64_t maxSize) override;
    void ClearData() override;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CLEANER_APP_EVENT_DB_CLEANER_H
