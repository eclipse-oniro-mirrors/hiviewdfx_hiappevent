/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "napi_hiappevent_userinfo.h"

#include <map>
#include <string>

#include "hiappevent_base.h"
#include "hiappevent_userinfo.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"
#include "napi_error.h"
#include "napi_util.h"

namespace OHOS {
namespace HiviewDFX {
namespace NapiHiAppEventUserInfo {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "Napi_HiAppEvent_UserInfo" };

static constexpr size_t MAX_STRING_LEN = 8 * 1024 + 2;

bool IsStringEmptyOrNull(const napi_env env, const napi_value name)
{
    napi_valuetype napiType = NapiUtil::GetType(env, name);
    if (napiType == napi_null) {
        return true;
    }
    if (napiType != napi_string) {
        return false;
    }
    std::string strName = NapiUtil::GetString(env, name, MAX_STRING_LEN);
    return strName.empty();
}
}

bool SetUserId(const napi_env env, const napi_value name, const napi_value userId)
{
    if (!NapiUtil::IsString(env, name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("name", "string"));
        return false;
    }
    std::string strName = NapiUtil::GetString(env, name, MAX_STRING_LEN);
    if (!IsValidUserIdName(strName)) {
        HiLog::Warn(LABEL, "Parameter error. The name parameter is invalid.");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    if (IsStringEmptyOrNull(env, userId)) {
        if (HiAppEvent::UserInfo::GetInstance().RemoveUserId(strName) != 0) {
            HiLog::Error(LABEL, "failed to remove userId");
            return false;
        }
        return true;
    }
    if (!NapiUtil::IsString(env, userId)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("userId", "string"));
        return false;
    }
    std::string strUserId = NapiUtil::GetString(env, userId, MAX_STRING_LEN);
    if (!IsValidUserIdValue(strUserId)) {
        HiLog::Warn(LABEL, "Parameter error. The value parameter is invalid.");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Parameter error. The value parameter is invalid.");
        return false;
    }
    if (HiAppEvent::UserInfo::GetInstance().SetUserId(strName, strUserId) != 0) {
        HiLog::Error(LABEL, "failed to set userId");
        return false;
    }
    return true;
}

bool GetUserId(const napi_env env, const napi_value name, napi_value& out)
{
    if (!NapiUtil::IsString(env, name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("name", "string"));
        return false;
    }
    std::string strName = NapiUtil::GetString(env, name, MAX_STRING_LEN);
    if (!IsValidUserIdName(strName)) {
        HiLog::Warn(LABEL, "Parameter error. The name parameter is invalid.");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    std::string strUserId;
    if (HiAppEvent::UserInfo::GetInstance().GetUserId(strName, strUserId) != 0) {
        HiLog::Error(LABEL, "failed to get userId");
        return false;
    }
    out = NapiUtil::CreateString(env, strUserId);
    return true;
}

bool SetUserProperty(const napi_env env, const napi_value name, const napi_value userProperty)
{
    if (!NapiUtil::IsString(env, name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("name", "string"));
        return false;
    }
    std::string strName = NapiUtil::GetString(env, name, MAX_STRING_LEN);
    if (!IsValidUserPropName(strName)) {
        HiLog::Warn(LABEL, "Parameter error. The name parameter is invalid.");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    if (IsStringEmptyOrNull(env, userProperty)) {
        if (HiAppEvent::UserInfo::GetInstance().RemoveUserProperty(strName) != 0) {
            HiLog::Error(LABEL, "failed to remove user property");
            return false;
        }
        return true;
    }
    if (!NapiUtil::IsString(env, userProperty)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("user property", "string"));
        return false;
    }
    std::string strUserProperty = NapiUtil::GetString(env, userProperty, MAX_STRING_LEN);
    if (!IsValidUserPropValue(strUserProperty)) {
        HiLog::Warn(LABEL, "Parameter error. The value parameter is invalid.");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Parameter error. The value parameter is invalid.");
        return false;
    }
    if (HiAppEvent::UserInfo::GetInstance().SetUserProperty(strName, strUserProperty) != 0) {
        HiLog::Error(LABEL, "failed to set user property");
        return false;
    }
    return true;
}

bool GetUserProperty(const napi_env env, const napi_value name, napi_value& out)
{
    if (!NapiUtil::IsString(env, name)) {
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, NapiUtil::CreateErrMsg("name", "string"));
        return false;
    }
    std::string strName = NapiUtil::GetString(env, name, MAX_STRING_LEN);
    if (!IsValidUserPropName(strName)) {
        HiLog::Warn(LABEL, "Parameter error. The name parameter is invalid.");
        NapiUtil::ThrowError(env, NapiError::ERR_PARAM, "Parameter error. The name parameter is invalid.");
        return false;
    }
    std::string strUserProperty;
    if (HiAppEvent::UserInfo::GetInstance().GetUserProperty(strName, strUserProperty) != 0) {
        HiLog::Error(LABEL, "failed to get user property");
        return false;
    }
    out = NapiUtil::CreateString(env, strUserProperty);
    return true;
}
} // namespace NapiHiAppEventUserInfo
} // namespace HiviewDFX
} // namespace OHOS
