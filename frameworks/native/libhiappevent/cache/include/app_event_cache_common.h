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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_CACHE_COMMON_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_CACHE_COMMON_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace AppEventCacheCommon {
constexpr int DB_SUCC = 0;
constexpr int DB_FAILED = -1;
const unsigned int HIAPPEVENT_DOMAIN = 0xD002D07;

namespace Blocks {
const std::string TABLE = "blocks";
const std::string FIELD_SEQ = "seq";
const std::string FIELD_NAME = "name";
} // namespace Blocks

namespace Block {
const std::string TABLE_PREFIX = "block_";
const std::string FIELD_SEQ = "seq";
const std::string FIELD_PACKAGE = "package";
const std::string FIELD_SIZE = "size";
} // namespace Block
} // namespace AppEventCacheCommon
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_CACHE_COMMON_H
