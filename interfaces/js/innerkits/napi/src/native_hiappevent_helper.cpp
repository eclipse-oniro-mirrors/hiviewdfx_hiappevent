/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "native_hiappevent_helper.h"

#include <string>
#include <vector>

#include "ability.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_pack.h"
#include "hiappevent_write.h"
#include "hilog/log.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::ErrorCode;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_NAPI" };
constexpr static int CALLBACK_FUNC_PARAM_NUM = 2;
constexpr static int NAPI_VALUE_STRING_LEN = 10240;
bool g_isSetDirFlag = false;

void AddBoolParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    bool value = true;
    napi_status status = napi_get_value_bool(env, param, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddBoolParam2EventPack：napi_get_value_bool failed.");
        return;
    }

    AddEventParam(appEventPack, key, value);
}

void AddNumberParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    double value = 0;
    napi_status status = napi_get_value_double(env, param, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddNumberParam2EventPack：napi_get_value_double failed.");
        return;
    }

    AddEventParam(appEventPack, key, value);
}

void AddStringParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    char value[NAPI_VALUE_STRING_LEN] = {0};
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, param, value, NAPI_VALUE_STRING_LEN - 1, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddStringParam2EventPack：napi_get_value_string_utf8 failed.");
        return;
    }

    std::string valueStr = value;
    AddEventParam(appEventPack, key, valueStr);
}

int AddBoolArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddBoolArrayParam2EventPack：napi_get_array_length failed.");
        return ERROR_UNKNOWN;
    }

    std::vector<bool> bools;
    napi_value element;
    napi_valuetype type;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrParam, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddBoolArrayParam2EventPack：napi_get_element failed.");
            return ERROR_UNKNOWN;
        }

        status = napi_typeof(env, element, &type);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddBoolArrayParam2EventPack：napi get element type failed.");
            return ERROR_UNKNOWN;
        }

        if (type != napi_valuetype::napi_boolean) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the array elements are not all boolean types.",
                key.c_str());
            return ERROR_INVALID_LIST_PARAM_TYPE;
        }

        bool value = true;
        status = napi_get_value_bool(env, element, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddBoolArrayParam2EventPack：napi_get_value_bool failed.");
            return ERROR_UNKNOWN;
        }
        bools.push_back(value);
    }
    AddEventParam(appEventPack, key, bools);
    return SUCCESS_FLAG;
}

int AddNumberArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddNumberArrayParam2EventPack：napi_get_array_length failed.");
        return ERROR_UNKNOWN;
    }

    std::vector<double> doubles;
    napi_value element;
    napi_valuetype type;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrParam, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddNumberArrayParam2EventPack：napi_get_element failed.");
            return ERROR_UNKNOWN;
        }

        status = napi_typeof(env, element, &type);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddNumberArrayParam2EventPack：napi get element type failed.");
            return ERROR_UNKNOWN;
        }

        if (type != napi_valuetype::napi_number) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the array elements are not all number types.",
                key.c_str());
            return ERROR_INVALID_LIST_PARAM_TYPE;
        }

        double value = 0;
        status = napi_get_value_double(env, element, &value);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddNumberArrayParam2EventPack：napi_get_value_double failed.");
            return ERROR_UNKNOWN;
        }
        doubles.push_back(value);
    }
    AddEventParam(appEventPack, key, doubles);
    return SUCCESS_FLAG;
}

int AddStringArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddStringArrayParam2EventPack：napi_get_array_length failed.");
        return ERROR_UNKNOWN;
    }

    std::vector<const std::string> strs;
    napi_value element;
    napi_valuetype type;
    for (uint32_t i = 0; i < len; i++) {
        status = napi_get_element(env, arrParam, i, &element);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddStringArrayParam2EventPack：napi_get_element failed.");
            return ERROR_UNKNOWN;
        }

        status = napi_typeof(env, element, &type);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddStringArrayParam2EventPack：napi get element type failed.");
            return ERROR_UNKNOWN;
        }

        if (type != napi_valuetype::napi_string) {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the array elements are not all string types.",
                key.c_str());
            return ERROR_INVALID_LIST_PARAM_TYPE;
        }

        char value[NAPI_VALUE_STRING_LEN] = {0};
        size_t valueLen = 0;
        status = napi_get_value_string_utf8(env, element, value, NAPI_VALUE_STRING_LEN - 1, &valueLen);
        if (status != napi_ok) {
            HiLog::Error(LABEL, "AddStringArrayParam2EventPack：napi_get_value_string_utf8 failed.");
            return ERROR_UNKNOWN;
        }
        strs.push_back(value);
    }
    AddEventParam(appEventPack, key, strs);
    return SUCCESS_FLAG;
}

int AddArrayParam2EventPack(napi_env env, const std::string &key, const napi_value arrParam,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, arrParam, &len);
    if (status != napi_ok) {
        HiLog::Warn(LABEL, "param=%{public}s is discarded because the value is not an array.", key.c_str());
        return ERROR_INVALID_PARAM_VALUE_TYPE;
    }

    if (len == 0) {
        HiLog::Warn(LABEL, "param=%{public}s array value is empty.", key.c_str());
        AddEventParam(appEventPack, key);
        return SUCCESS_FLAG;
    }

    napi_value value;
    status = napi_get_element(env, arrParam, 0, &value);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "napi failed to get array element.");
        return ERROR_UNKNOWN;
    }

    napi_valuetype type;
    status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "napi failed to get the element type.");
        return ERROR_UNKNOWN;
    }

    int res = SUCCESS_FLAG;
    if (type == napi_valuetype::napi_boolean) {
        res = AddBoolArrayParam2EventPack(env, key, arrParam, appEventPack);
    } else if (type == napi_valuetype::napi_number) {
        res = AddNumberArrayParam2EventPack(env, key, arrParam, appEventPack);
    } else if (type == napi_valuetype::napi_string) {
        res = AddStringArrayParam2EventPack(env, key, arrParam, appEventPack);
    } else {
        HiLog::Warn(LABEL, "param=%{public}s is discarded because the list value type is invalid.", key.c_str());
        return ERROR_INVALID_LIST_PARAM_TYPE;
    }

    return res;
}

int AddParam2EventPack(napi_env env, const std::string &key, const napi_value param,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    napi_valuetype type;
    napi_status status = napi_typeof(env, param, &type);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "AddParam2EventPack：napi_typeof failed.");
        return ERROR_UNKNOWN;
    }

    if (type == napi_valuetype::napi_boolean) {
        AddBoolParam2EventPack(env, key, param, appEventPack);
    } else if (type == napi_valuetype::napi_number) {
        AddNumberParam2EventPack(env, key, param, appEventPack);
    } else if (type == napi_valuetype::napi_string) {
        AddStringParam2EventPack(env, key, param, appEventPack);
    } else if (type == napi_valuetype::napi_object) {
        return AddArrayParam2EventPack(env, key, param, appEventPack);
    } else {
        HiLog::Warn(LABEL, "param=%{public}s is discarded because the value type is invalid.", key.c_str());
        return ERROR_INVALID_PARAM_VALUE_TYPE;
    }

    return SUCCESS_FLAG;
}

bool CheckConfigureParamsType(napi_env env, const napi_value name, const napi_value value)
{
    napi_valuetype valueType;
    napi_typeof(env, name, &valueType);
    if (valueType != napi_valuetype::napi_string) {
        HiLog::Error(LABEL, "the configuration item name must be of type string.");
        return false;
    }

    napi_typeof(env, value, &valueType);
    if (valueType != napi_valuetype::napi_string) {
        HiLog::Error(LABEL, "the Configuration item value must be of type string.");
        return false;
    }

    return true;
}
}

int BuildAppEventPack(napi_env env, const napi_value params[], const int paramEndIndex,
    std::shared_ptr<AppEventPack>& appEventPack)
{
    const int valuePost = 1;
    const int kvNumber = 2;
    const int paramStartIndex = 2;
    std::string keyStr;
    int buildRes = SUCCESS_FLAG;
    for (int index = paramStartIndex; index < paramEndIndex; index++) {
        if (index % kvNumber == valuePost) {
            if (keyStr.empty()) {
                HiLog::Warn(LABEL, "param is discarded because the param name cannot be empty.");
                buildRes = ERROR_INVALID_PARAM_NAME;
                continue;
            }

            int addParamsRes = AddParam2EventPack(env, keyStr, params[index], appEventPack);
            buildRes = (addParamsRes == SUCCESS_FLAG) ? buildRes : addParamsRes;
            keyStr.clear();
        } else {
            napi_valuetype type;
            napi_typeof(env, params[index], &type);

            if (type == napi_valuetype::napi_string) {
                char key[NAPI_VALUE_STRING_LEN] = {0};
                size_t len = 0;
                napi_get_value_string_utf8(env, params[index], key, NAPI_VALUE_STRING_LEN - 1, &len);
                keyStr = key;
            } else {
                HiLog::Warn(LABEL, "param is discarded because the key type of the event params must be String.");
                buildRes = ERROR_INVALID_PARAM_KEY_TYPE;
                index++;
                continue;
            }
        }
    }

    return buildRes;
}

int BuildAppEventPackFromObject(napi_env env, const napi_value object, std::shared_ptr<AppEventPack>& appEventPack)
{
    napi_value keyArr = nullptr;
    napi_status status = napi_get_property_names(env, object, &keyArr);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "napi_get_property_names failed.");
        return ERROR_UNKNOWN;
    }

    uint32_t len = 0;
    status = napi_get_array_length(env, keyArr, &len);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "napi_get_array_length failed.");
        return ERROR_UNKNOWN;
    }

    int buildRes = SUCCESS_FLAG;
    for (uint32_t i = 0; i < len; i++) {
        napi_value keyNapiValue = nullptr;
        napi_get_element(env, keyArr, i, &keyNapiValue);

        napi_valuetype valueType;
        napi_typeof(env, keyNapiValue, &valueType);
        if (valueType != napi_valuetype::napi_string) {
            HiLog::Warn(LABEL, "param is discarded because the key type of the event params must be String.");
            buildRes = ERROR_INVALID_PARAM_KEY_TYPE;
            continue;
        }

        char key[NAPI_VALUE_STRING_LEN] = {0};
        size_t cValueLength = 0;
        napi_get_value_string_utf8(env, keyNapiValue, key, NAPI_VALUE_STRING_LEN - 1, &cValueLength);

        napi_value value = nullptr;
        napi_get_named_property(env, object, key, &value);

        std::string keyStr = key;
        napi_typeof(env, value, &valueType);
        if (valueType == napi_valuetype::napi_boolean) {
            AddBoolParam2EventPack(env, keyStr, value, appEventPack);
        } else if (valueType == napi_valuetype::napi_number) {
            AddNumberParam2EventPack(env, keyStr, value, appEventPack);
        } else if (valueType == napi_valuetype::napi_string) {
            AddStringParam2EventPack(env, keyStr, value, appEventPack);
        } else if (valueType == napi_valuetype::napi_object) {
            int addArrParamsRes = AddArrayParam2EventPack(env, keyStr, value, appEventPack);
            buildRes = (addArrParamsRes == SUCCESS_FLAG) ? buildRes : addArrParamsRes;
        } else {
            HiLog::Warn(LABEL, "param=%{public}s is discarded because the value type is invalid.", key);
            buildRes = ERROR_INVALID_PARAM_VALUE_TYPE;
            continue;
        }
    }

    return buildRes;
}

int CheckWriteParamsType(napi_env env, const napi_value params[], const int paramNum)
{
    if (paramNum < WRITE_FUNC_MIN_PARAM_NUM || paramNum > WRITE_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "invalid number=%{public}d of params.", paramNum);
        return ERROR_INVALID_PARAM_NUM_JS;
    }

    napi_valuetype valueType;
    napi_typeof(env, params[EVENT_NAME_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        HiLog::Error(LABEL, "the first param must be of type string.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    napi_typeof(env, params[EVENT_TYPE_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        HiLog::Error(LABEL, "the second param must be of type number.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    return SUCCESS_FLAG;
}

int CheckWriteJsonParamsType(napi_env env, const napi_value params[], const int paramNum)
{
    if (paramNum < WRITE_JSON_FUNC_MIN_PARAM_NUM || paramNum > WRITE_JSON_FUNC_MAX_PARAM_NUM) {
        HiLog::Error(LABEL, "invalid number=%{public}d of params.", paramNum);
        return ERROR_INVALID_PARAM_NUM_JS;
    }

    napi_valuetype valueType;
    napi_typeof(env, params[EVENT_NAME_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_string) {
        HiLog::Error(LABEL, "the first param must be of type string.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    napi_typeof(env, params[EVENT_TYPE_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        HiLog::Error(LABEL, "the second param must be of type number.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    napi_typeof(env, params[JSON_OBJECT_INDEX], &valueType);
    if (valueType != napi_valuetype::napi_object) {
        HiLog::Error(LABEL, "the third param must be of type object.");
        return ERROR_INVALID_PARAM_TYPE_JS;
    }

    return SUCCESS_FLAG;
}

bool ConfigureFromNapiValue(napi_env env, const napi_value name, const napi_value value)
{
    if (!CheckConfigureParamsType(env, name, value)) {
        HiLog::Error(LABEL, "failed to check configuration params.");
        return false;
    }

    char nameChs[NAPI_VALUE_STRING_LEN] = {0};
    size_t cValueLength = 0;
    napi_get_value_string_utf8(env, name, nameChs, NAPI_VALUE_STRING_LEN - 1, &cValueLength);

    char valueChs[NAPI_VALUE_STRING_LEN] = {0};
    napi_get_value_string_utf8(env, value, valueChs, NAPI_VALUE_STRING_LEN - 1, &cValueLength);

    return HiAppEventConfig::GetInstance().SetConfigurationItem(nameChs, valueChs);
}

std::shared_ptr<AppEventPack> CreateEventPackFromNapiValue(napi_env env, napi_value nameValue, napi_value typeValue)
{
    char eventName[NAPI_VALUE_STRING_LEN] = {0};
    size_t cValueLength = 0;
    napi_get_value_string_utf8(env, nameValue, eventName, NAPI_VALUE_STRING_LEN - 1, &cValueLength);

    int32_t eventType = 0;
    napi_get_value_int32(env, typeValue, &eventType);

    return std::make_shared<AppEventPack>(eventName, eventType);
}

void AsyncWriteEvent(napi_env env, HiAppEventAsyncContext* asyncContext)
{
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSHiAppEventWrite", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            if (asyncContext->appEventPack != nullptr && asyncContext->result >= SUCCESS_FLAG) {
                WriterEvent(asyncContext->appEventPack);
            }
        },
        [](napi_env env, napi_status status, void* data) {
            HiAppEventAsyncContext* asyncContext = (HiAppEventAsyncContext*)data;
            napi_value result[CALLBACK_FUNC_PARAM_NUM] = {0};
            if (asyncContext->result == SUCCESS_FLAG) {
                napi_get_undefined(env, &result[0]);
                napi_create_int32(env, asyncContext->result, &result[1]);
            } else {
                napi_create_object(env, &result[0]);
                napi_value errCode = nullptr;
                napi_create_int32(env, asyncContext->result, &errCode);
                napi_set_named_property(env, result[0], "code", errCode);
                napi_get_undefined(env, &result[1]);
            }

            if (asyncContext->deferred) {
                if (asyncContext->result == SUCCESS_FLAG) {
                    napi_resolve_deferred(env, asyncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callback, &callback);
                napi_value retValue = nullptr;
                napi_call_function(env, nullptr, callback, CALLBACK_FUNC_PARAM_NUM, result, &retValue);
                napi_delete_reference(env, asyncContext->callback);
            }

            napi_delete_async_work(env, asyncContext->asyncWork);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->asyncWork);
    napi_queue_async_work(env, asyncContext->asyncWork);
}

void SetStorageDir(napi_env env, napi_callback_info info)
{
    if (g_isSetDirFlag) {
        return;
    }

    HiLog::Debug(LABEL, "start to init storage path.");

    napi_value global = nullptr;
    napi_get_global(env, &global);

    napi_value abilityObj = nullptr;
    napi_get_named_property(env, global, "ability", &abilityObj);

    AppExecFwk::Ability* ability = nullptr;
    napi_get_value_external(env, abilityObj, (void**)&ability);

    if (ability == nullptr) {
        HiLog::Error(LABEL, "ability is null, stop setting the storage dir.");
        return;
    }

    std::string dir = ability->GetFilesDir();
    HiAppEventConfig::GetInstance().SetStorageDir(dir);
    g_isSetDirFlag = true;
}
}
}