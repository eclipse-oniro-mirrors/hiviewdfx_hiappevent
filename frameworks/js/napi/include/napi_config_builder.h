/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_CONFIG_BUILDER_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_CONFIG_BUILDER_H

#include <map>
#include <string>

#include "napi/native_api.h"

namespace OHOS {
namespace HiviewDFX {
struct EventConfigPack {
    std::map<std::string, std::string> configStringMap;
    std::map<uint8_t, uint32_t> configUintMap;
    std::string eventName;
    bool isValid {false};
};

class NapiConfigBuilder {
public:
    NapiConfigBuilder() : eventConfigPack_(std::make_unique<EventConfigPack>()) {}
    std::unique_ptr<EventConfigPack> BuildEventConfig(const napi_env env, const napi_value params[], size_t len);

private:
    void GetAppCrashConfig(const napi_env env, const napi_value params);
    void GetCommonConfig(const napi_env env, const napi_value params);

private:
    std::unique_ptr<EventConfigPack> eventConfigPack_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_CONFIG_BUILDER_H