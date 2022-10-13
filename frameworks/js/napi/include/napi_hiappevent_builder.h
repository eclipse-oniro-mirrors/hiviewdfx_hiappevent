/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_BUILDER_H
#define HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_BUILDER_H

#include <memory>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

class NapiHiAppEventBuilder {
public:
    NapiHiAppEventBuilder() : isV9_(false), result_(0), callback_(nullptr), appEventPack_(nullptr) {}
    ~NapiHiAppEventBuilder() {}
    int GetResult() const;
    napi_ref GetCallback() const;
    std::shared_ptr<AppEventPack> Build(const napi_env env, const napi_value params[], size_t len);
    std::shared_ptr<AppEventPack> BuildV9(const napi_env env, const napi_value params[], size_t len);

private:
    bool IsValidEventDomain(const napi_env env, const napi_value domain);
    bool IsValidEventName(const napi_env env, const napi_value name);
    bool IsValidEventType(const napi_env env, const napi_value type);
    bool IsValidEventParam(const napi_env env, const napi_value param);
    bool IsValidEventInfo(const napi_env env, const napi_value eventInfo);
    bool IsOldWriteParams(const napi_env env, const napi_value params[], size_t len);
    bool IsNewWriteParams(const napi_env env, const napi_value params[], size_t len);
    void AddArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arr);
    void AddParam2EventPack(napi_env env, const std::string &key, const napi_value value);
    void AddParams2EventPack(napi_env env, const napi_value paramObj);
    void BuildEventPack(napi_env env, const napi_value params[]);
    void BuildEventPack(napi_env env, const napi_value eventInfo);
    void BuildCallback(const napi_env env, const napi_value callback);

private:
    bool isV9_;
    int result_;
    napi_ref callback_;
    std::shared_ptr<AppEventPack> appEventPack_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_HIAPPEVENT_BUILDER_H
