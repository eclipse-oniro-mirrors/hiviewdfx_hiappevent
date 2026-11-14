/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "napi_hiappevent_builder.h"

#include "hiappevent_base.h"
#include "hiappevent_facade.h"
#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiEventBuilder"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr const char* DOMAIN_PROPERTY = "domain";
constexpr const char* NAME_PROPERTY = "name";
constexpr const char* TYPE_PROPERTY = "eventType";
constexpr const char* PARAM_PROPERTY = "params";
constexpr size_t MAX_LENGTH_OF_PARAM_NAME = 32;
constexpr const char* PARAM_VALUE_TYPE = "boolean|number|string|array[boolean|number|string]";
constexpr size_t INDEX_OF_INFO_PARAM = 0;
constexpr const char* TYPE_NAME_OF_STRING = "string";
constexpr const char* TYPE_NAME_OF_OBJECT = "object";
constexpr const char* TYPE_NAME_OF_EVENT_TYPE = "EventType";

std::string CreateRequiredErrMsg(size_t paramIndex, const std::string& propName)
{
    return propName.empty() ? NapiUtil::CreateParamRequiredErrMsg(paramIndex)
        : NapiUtil::CreatePropertyRequiredErrMsg(paramIndex, propName);
}

std::string CreateTypeMismatchErrMsg(size_t paramIndex, const std::string& propName, const std::string& expectedType,
    const std::string& actualType)
{
    return propName.empty() ? NapiUtil::CreateParamTypeErrMsg(paramIndex, expectedType, actualType)
        : NapiUtil::CreatePropertyTypeErrMsg(paramIndex, propName, expectedType, actualType);
}
}
using namespace OHOS::HiviewDFX::ErrorCode;

bool NapiHiAppEventBuilder::IsValidEventDomain(const napi_env env, const napi_value domain, size_t paramIndex,
    const std::string& propName, bool isRequired)
{
    if (domain == nullptr) {
        if (!isRequired) {
            return true;
        }
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    napi_valuetype type = NapiUtil::GetType(env, domain);
    if ((type == napi_null || type == napi_undefined) && isRequired) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    if (!NapiUtil::IsString(env, domain)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM,
            CreateTypeMismatchErrMsg(paramIndex, propName, TYPE_NAME_OF_STRING, NapiUtil::GetTypeName(type)), isV9_);
        return false;
    }
    return true;
}

bool NapiHiAppEventBuilder::IsValidEventName(const napi_env env, const napi_value name, size_t paramIndex,
    const std::string& propName, bool isRequired)
{
    if (name == nullptr) {
        if (!isRequired) {
            return true;
        }
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    napi_valuetype type = NapiUtil::GetType(env, name);
    if ((type == napi_null || type == napi_undefined) && isRequired) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    if (!NapiUtil::IsString(env, name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM,
            CreateTypeMismatchErrMsg(paramIndex, propName, TYPE_NAME_OF_STRING, NapiUtil::GetTypeName(type)), isV9_);
        return false;
    }
    return true;
}

bool NapiHiAppEventBuilder::IsValidEventType(const napi_env env, const napi_value type, size_t paramIndex,
    const std::string& propName, bool isRequired)
{
    if (type == nullptr) {
        if (!isRequired) {
            return true;
        }
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    napi_valuetype valueType = NapiUtil::GetType(env, type);
    if ((valueType == napi_null || valueType == napi_undefined) && isRequired) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    if (!NapiUtil::IsNumber(env, type)
        || !AppEventVerifyFacade::VerifyIsValidEventType(NapiUtil::GetInt32(env, type))) {
        std::string errMsg = CreateTypeMismatchErrMsg(paramIndex, propName, TYPE_NAME_OF_EVENT_TYPE,
            NapiUtil::GetTypeName(valueType));
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isV9_);
        return false;
    }
    return true;
}

bool NapiHiAppEventBuilder::IsValidEventParam(const napi_env env, const napi_value param, size_t paramIndex,
    const std::string& propName, bool isRequired)
{
    if (param == nullptr) {
        if (!isRequired) {
            return true;
        }
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    napi_valuetype type = NapiUtil::GetType(env, param);
    if ((type == napi_null || type == napi_undefined) && isRequired) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, CreateRequiredErrMsg(paramIndex, propName), isV9_);
        return false;
    }
    if (!NapiUtil::IsObject(env, param)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM,
            CreateTypeMismatchErrMsg(paramIndex, propName, TYPE_NAME_OF_OBJECT, NapiUtil::GetTypeName(type)), isV9_);
        return false;
    }
    return true;
}

bool NapiHiAppEventBuilder::IsValidEventInfo(const napi_env env, const napi_value eventInfo)
{
    napi_valuetype type = NapiUtil::GetType(env, eventInfo);
    if (type == napi_null || type == napi_undefined) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM,
            NapiUtil::CreateParamRequiredErrMsg(INDEX_OF_INFO_PARAM), isV9_);
        return false;
    }
    if (!NapiUtil::IsObject(env, eventInfo)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM,
            NapiUtil::CreateParamTypeErrMsg(INDEX_OF_INFO_PARAM, TYPE_NAME_OF_OBJECT, NapiUtil::GetTypeName(type)),
            isV9_);
        return false;
    }
    return IsValidEventDomain(env, NapiUtil::GetProperty(env, eventInfo, DOMAIN_PROPERTY), INDEX_OF_INFO_PARAM,
        DOMAIN_PROPERTY)
        && IsValidEventName(env, NapiUtil::GetProperty(env, eventInfo, NAME_PROPERTY), INDEX_OF_INFO_PARAM,
        NAME_PROPERTY)
        && IsValidEventType(env, NapiUtil::GetProperty(env, eventInfo, TYPE_PROPERTY), INDEX_OF_INFO_PARAM,
        TYPE_PROPERTY)
        && IsValidEventParam(env, NapiUtil::GetProperty(env, eventInfo, PARAM_PROPERTY), INDEX_OF_INFO_PARAM,
        PARAM_PROPERTY);
}

bool NapiHiAppEventBuilder::IsOldWriteParams(const napi_env env, const napi_value params[], size_t len)
{
    return IsValidEventName(env, params[0], 0) // 0 means the index of event name
        && IsValidEventType(env, params[1], 1) // 1 means the index of event type
        && IsValidEventParam(env, params[2], 2); // 2 means the index of event param
}

bool NapiHiAppEventBuilder::IsNewWriteParams(const napi_env env, const napi_value params[], size_t len)
{
    return IsValidEventInfo(env, params[0]);
}

bool NapiHiAppEventBuilder::AddArrayParam2EventPack(napi_env env, const std::string &key,
    const napi_value arr)
{
    napi_valuetype type = NapiUtil::GetArrayType(env, arr);
    switch (type) {
        case napi_boolean: {
            std::vector<bool> bools;
            NapiUtil::GetBooleans(env, arr, bools);
            appEventPack_->AddParam(key, bools);
            break;
        }
        case napi_number: {
            std::vector<double> doubles;
            NapiUtil::GetDoubles(env, arr, doubles);
            appEventPack_->AddParam(key, doubles);
            break;
        }
        case napi_string: {
            std::vector<std::string> strs;
            NapiUtil::GetStrings(env, arr, strs);
            appEventPack_->AddParam(key, strs);
            break;
        }
        case napi_null: {
            appEventPack_->AddParam(key);
            break;
        }
        default: {
            HILOG_ERROR(LOG_CORE, "array param value type is invalid");
            result_ = ERROR_INVALID_LIST_PARAM_TYPE;
            std::string errMsg = NapiUtil::CreatePropertyTypeErrMsg(INDEX_OF_INFO_PARAM,
                std::string(PARAM_PROPERTY) + "." + key, PARAM_VALUE_TYPE, NapiUtil::GetTypeName(env, arr));
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isV9_);
            return false;
        }
    }
    return true;
}

bool NapiHiAppEventBuilder::AddParam2EventPack(napi_env env, const std::string &key,
    const napi_value value)
{
    napi_valuetype type = NapiUtil::GetType(env, value);
    switch (type) {
        case napi_boolean:
            appEventPack_->AddParam(key, NapiUtil::GetBoolean(env, value));
            break;
        case napi_number:
            appEventPack_->AddParam(key, NapiUtil::GetDouble(env, value));
            break;
        case napi_string:
            appEventPack_->AddParam(key, NapiUtil::GetString(env, value));
            break;
        case napi_object:
            if (NapiUtil::IsArray(env, value)) {
                return AddArrayParam2EventPack(env, key, value);
            }
            [[fallthrough]];
        default:
            HILOG_ERROR(LOG_CORE, "param value type is invalid");
            result_ = ERROR_INVALID_PARAM_VALUE_TYPE;
            std::string errMsg = NapiUtil::CreatePropertyTypeErrMsg(INDEX_OF_INFO_PARAM,
                std::string(PARAM_PROPERTY) + "." + key, PARAM_VALUE_TYPE, NapiUtil::GetTypeName(type));
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isV9_);
            return false;
    }
    return true;
}

bool NapiHiAppEventBuilder::AddParams2EventPack(napi_env env, const napi_value paramObj)
{
    std::vector<std::string> keys;
    NapiUtil::GetPropertyNames(env, paramObj, keys);
    for (auto key : keys) {
        if (key.length() > MAX_LENGTH_OF_PARAM_NAME) {
            result_ = ERROR_INVALID_PARAM_NAME;
            HILOG_INFO(LOG_CORE, "the length=%{public}zu of the param key is invalid", key.length());
            continue;
        }
        napi_value value = NapiUtil::GetProperty(env, paramObj, key);
        if (value == nullptr) {
            result_ = ERROR_INVALID_PARAM_VALUE_TYPE;
            std::string errMsg = NapiUtil::CreatePropertyTypeErrMsg(INDEX_OF_INFO_PARAM,
                std::string(PARAM_PROPERTY) + "." + key, PARAM_VALUE_TYPE, "unknown");
            NapiUtil::ThrowError(env, NapiError::ERR_PARAM, errMsg, isV9_);
            return false;
        }
        if (!AddParam2EventPack(env, key, value)) {
            return false;
        }
    }
    return true;
}

void NapiHiAppEventBuilder::BuildEventPack(napi_env env, const napi_value params[])
{
    int index = 0;
    std::string name = NapiUtil::GetString(env, params[index++]);
    int32_t type = NapiUtil::GetInt32(env, params[index++]);
    appEventPack_ = std::make_shared<AppEventPack>(name, type);
    AddParams2EventPack(env, params[index]);
}

bool NapiHiAppEventBuilder::BuildEventPack(napi_env env, const napi_value eventInfo)
{
    std::string domain = NapiUtil::GetString(env, NapiUtil::GetProperty(env, eventInfo, DOMAIN_PROPERTY));
    std::string name = NapiUtil::GetString(env, NapiUtil::GetProperty(env, eventInfo, NAME_PROPERTY));
    int32_t type = NapiUtil::GetInt32(env, NapiUtil::GetProperty(env, eventInfo, TYPE_PROPERTY));
    appEventPack_ = std::make_shared<AppEventPack>(domain, name, type);
    napi_value param = NapiUtil::GetProperty(env, eventInfo, PARAM_PROPERTY);
    return AddParams2EventPack(env, param);
}

void NapiHiAppEventBuilder::BuildCallback(const napi_env env, const napi_value callback)
{
    if (NapiUtil::IsFunction(env, callback)) {
        callback_ = NapiUtil::CreateReference(env, callback);
    }
}

std::shared_ptr<AppEventPack> NapiHiAppEventBuilder::Build(const napi_env env, const napi_value params[], size_t len)
{
    if (len < 3) { // the min number of params for write is 3
        result_ = ERROR_INVALID_PARAM_NUM_JS;
        return nullptr;
    }

    // 1. build AppEventPack
    if (IsOldWriteParams(env, params, len)) {
        BuildEventPack(env, params);
    } else {
        result_ = ERROR_INVALID_PARAM_TYPE_JS;
    }

    // 2. build callback if any
    BuildCallback(env, params[len - 1]); // (len - 1) means the last param
    return appEventPack_;
}

std::shared_ptr<AppEventPack> NapiHiAppEventBuilder::BuildV9(const napi_env env, const napi_value params[], size_t len)
{
    isV9_ = true;
    if (len < 1) { // the min number of params for writeV9 is 1
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateParamCntErrMsg(1, len), isV9_);
        return nullptr;
    }
    if (!IsNewWriteParams(env, params, len)) {
        return nullptr;
    }
    if (!BuildEventPack(env, params[0])) {
        return nullptr;
    }
    BuildCallback(env, params[len - 1]); // (len - 1) means the last param
    return appEventPack_;
}

int NapiHiAppEventBuilder::GetResult() const
{
    return result_;
}

napi_ref NapiHiAppEventBuilder::GetCallback() const
{
    return callback_;
}
} // namespace HiviewDFX
} // namespace OHOS
