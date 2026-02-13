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

#include "hiappevent_c.h"

#include <memory>
#include <vector>

#include "event_policy_mgr.h"
#include "hiappevent_base.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hiappevent_verify.h"
#include "hiappevent_write.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AppEventC"

using namespace OHOS::HiviewDFX;

namespace {
constexpr int MAX_SIZE_OF_LIST_PARAM = 100;
constexpr size_t MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE = 1024;

template<typename T>
ParamList AddParamValue(ParamList list, const char* name, T value)
{
    if (list == nullptr || name == nullptr) {
        HILOG_WARN(LOG_CORE, "ParamList is nullptr or name is nullptr.");
        return list;
    }
    auto ndkAppEventPackPtr = reinterpret_cast<AppEventPack *>(list);
    ndkAppEventPackPtr->AddParam(name, value);
    return reinterpret_cast<ParamList>(ndkAppEventPackPtr);
}

template<typename T>
ParamList AddParamArrayValue(ParamList list, const char* name, const T* arr, int len)
{
    if (list == nullptr || name == nullptr || arr == nullptr) {
        HILOG_WARN(LOG_CORE, "ParamList is nullptr or name is nullptr or param array is nullptr.");
        return list;
    }
    std::vector<T> params(arr, (len > MAX_SIZE_OF_LIST_PARAM) ? (arr + MAX_SIZE_OF_LIST_PARAM + 1) : (arr + len));
    auto ndkAppEventPackPtr = reinterpret_cast<AppEventPack *>(list);
    ndkAppEventPackPtr->AddParam(name, params);
    return reinterpret_cast<ParamList>(ndkAppEventPackPtr);
}
}

ParamList HiAppEventCreateParamList()
{
    auto ndkAppEventPackPtr = new (std::nothrow) AppEventPack();
    if (ndkAppEventPackPtr == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to new ParamList.");
    }
    return reinterpret_cast<ParamList>(ndkAppEventPackPtr);
}

void HiAppEventDestroyParamList(ParamList list)
{
    if (list != nullptr) {
        delete reinterpret_cast<AppEventPack *>(list);
        list = nullptr;
    }
}

ParamList AddBoolParamValue(ParamList list, const char* name, bool boolean)
{
    return AddParamValue(list, name, boolean);
}

ParamList AddBoolArrayParamValue(ParamList list, const char* name, const bool* booleans, int arrSize)
{
    return AddParamArrayValue(list, name, booleans, arrSize);
}

ParamList AddInt8ParamValue(ParamList list, const char* name, int8_t num)
{
    return AddParamValue(list, name, num);
}

ParamList AddInt8ArrayParamValue(ParamList list, const char* name, const int8_t* nums, int arrSize)
{
    return AddParamArrayValue(list, name, nums, arrSize);
}

ParamList AddInt16ParamValue(ParamList list, const char* name, int16_t num)
{
    return AddParamValue(list, name, num);
}

ParamList AddInt16ArrayParamValue(ParamList list, const char* name, const int16_t* nums, int arrSize)
{
    return AddParamArrayValue(list, name, nums, arrSize);
}

ParamList AddInt32ParamValue(ParamList list, const char* name, int32_t num)
{
    return AddParamValue(list, name, num);
}

ParamList AddInt32ArrayParamValue(ParamList list, const char* name, const int32_t* nums, int arrSize)
{
    return AddParamArrayValue(list, name, nums, arrSize);
}

ParamList AddInt64ParamValue(ParamList list, const char* name, int64_t num)
{
    return AddParamValue(list, name, num);
}

ParamList AddInt64ArrayParamValue(ParamList list, const char* name, const int64_t* nums, int arrSize)
{
    return AddParamArrayValue(list, name, nums, arrSize);
}

ParamList AddFloatParamValue(ParamList list, const char* name, float num)
{
    return AddParamValue(list, name, num);
}

ParamList AddFloatArrayParamValue(ParamList list, const char* name, const float* nums, int arrSize)
{
    return AddParamArrayValue(list, name, nums, arrSize);
}

ParamList AddDoubleParamValue(ParamList list, const char* name, double num)
{
    return AddParamValue(list, name, num);
}

ParamList AddDoubleArrayParamValue(ParamList list, const char* name, const double* nums, int arrSize)
{
    return AddParamArrayValue(list, name, nums, arrSize);
}

ParamList AddStringParamValue(ParamList list, const char* name, const char* str)
{
    if (str == nullptr) {
        HILOG_WARN(LOG_CORE, "the str is nullptr.");
        return list;
    }
    return AddParamValue(list, name, str);
}

ParamList AddStringArrayParamValue(ParamList list, const char* name, const char* const *strs, int arrSize)
{
    return AddParamArrayValue(list, name, strs, arrSize);
}

bool HiAppEventInnerConfigure(const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to configure, because the input params contain a null pointer.");
        return false;
    }
    return HiAppEventConfig::GetInstance().SetConfigurationItem(name, value);
}

int HiAppEventInnerWrite(const char* domain, const char* name, EventType type, const ParamList list)
{
    if (domain == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to write event, domain is null");
        return ErrorCode::ERROR_INVALID_EVENT_DOMAIN;
    }
    if (name == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to write event, name is null");
        return ErrorCode::ERROR_INVALID_EVENT_NAME;
    }

    std::shared_ptr<AppEventPack> appEventPack = std::make_shared<AppEventPack>(domain, name, type);
    if (list != nullptr) {
        auto ndkAppEventPackPtr = reinterpret_cast<AppEventPack *>(list);
        appEventPack->SetBaseParams(ndkAppEventPackPtr->GetBaseParams());
    }
    int res = VerifyAppEvent(appEventPack);
    if (res >= 0) {
        SubmitWritingTask(appEventPack, "app_c_event");
    }
    return res;
}

void ClearData()
{
    HiAppEventClean::ClearData(HiAppEventConfig::GetInstance().GetStorageDir());
}

HiAppEvent_Config* HiAppEventCreateConfig()
{
    auto ndkConfigMapPtr = new (std::nothrow) std::map<std::string, std::string>;
    if (ndkConfigMapPtr == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to new HiAppEvent_Config.");
    }
    return reinterpret_cast<HiAppEvent_Config *>(ndkConfigMapPtr);
}

int HiAppEventSetConfigItem(HiAppEvent_Config* config, const char* itemName, const char* itemValue)
{
    if (config == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to Set Config Item, the event config is null.");
        return ErrorCode::ERROR_EVENT_CONFIG_IS_NULL;
    }

    if (itemName == nullptr || std::strlen(itemName) > MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE) {
        HILOG_ERROR(LOG_CORE, "Failed to Set Config Item, the itemName is nullptr or length is more than %{public}zu.",
            MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE);
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }

    std::string itemNameStr = itemName;
    std::string itemValueStr = "";
    if (itemValue != nullptr) {
        if (std::strlen(itemValue) > MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE) {
            HILOG_ERROR(LOG_CORE, "Failed to Set Config Item, the itemValue length is more than %{public}zu.",
                MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE);
            return ErrorCode::ERROR_INVALID_PARAM_VALUE;
        }
        itemValueStr = itemValue;
    }
    auto ndkConfigMapPtr = reinterpret_cast<std::map<std::string, std::string> *>(config);
    (*ndkConfigMapPtr)[itemNameStr] = itemValueStr;
    return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
}

int HiAppEventSetEventConfig(const char* name, HiAppEvent_Config* config)
{
    if (config == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to set event config, the event config is null.");
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    if (name == nullptr || std::strlen(name) > MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE) {
        HILOG_ERROR(LOG_CORE, "Failed to set event config, the name is nullptr or length more than %{public}zu.",
            MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE);
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }

    auto configMap = reinterpret_cast<std::map<std::string, std::string> *>(config);
    int res = EventPolicyMgr::GetInstance().SetEventPolicy(name, *configMap);
    if (res != ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL) {
        return ErrorCode::ERROR_INVALID_PARAM_VALUE;
    }
    return res;
}

void HiAppEventDestroyConfig(HiAppEvent_Config* config)
{
    if (config != nullptr) {
        delete reinterpret_cast<std::map<std::string, std::string> *>(config);
        config = nullptr;
    }
}
