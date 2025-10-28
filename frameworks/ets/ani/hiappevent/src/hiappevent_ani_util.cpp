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

#include "hiappevent_ani_util.h"

#include "json/json.h"

#include <ani_signature_builder.h>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_ANI_UTIL"

using namespace OHOS::HiviewDFX;
using namespace arkts::ani_signature;
static const std::pair<const char*, AniArgsType> OBJECT_TYPE[] = {
    {CLASS_NAME_INT, AniArgsType::ANI_INT},
    {CLASS_NAME_LONG, AniArgsType::ANI_LONG},
    {CLASS_NAME_BOOLEAN, AniArgsType::ANI_BOOLEAN},
    {CLASS_NAME_DOUBLE, AniArgsType::ANI_DOUBLE},
    {CLASS_NAME_STRING, AniArgsType::ANI_STRING},
};

static const std::map<EventTypeAni, int32_t> ANI_EVENTTYPE_INDEX_MAP = {
    {EventTypeAni::FAULT, 0},
    {EventTypeAni::STATISTIC, 1},
    {EventTypeAni::SECURITY, 2},
    {EventTypeAni::BEHAVIOR, 3},
};

std::string HiAppEventAniUtil::CreateErrMsg(const std::string& name)
{
    return "Parameter error. The " + name + " parameter is mandatory.";
}

std::string HiAppEventAniUtil::CreateErrMsg(const std::string &name, const std::string &type)
{
    return "Parameter error. The type of " + name + " must be " + type + ".";
}

bool HiAppEventAniUtil::IsArray(ani_env *env, ani_object object)
{
    ani_boolean IsArray = ANI_FALSE;
    ani_class cls {};
    if (env == nullptr) {
        return false;
    }
    if (env->FindClass(CLASS_NAME_ARRAY, &cls) != ANI_OK) {
        return false;
    }
    ani_static_method static_method {};
    if (env->Class_FindStaticMethod(cls, "isArray", nullptr, &static_method) != ANI_OK) {
        return false;
    }
    if (env->Class_CallStaticMethod_Boolean(cls, static_method, &IsArray, object) != ANI_OK) {
        return false;
    }
    return static_cast<bool>(IsArray);
}

bool HiAppEventAniUtil::IsRefUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    if (env != nullptr) {
        env->Reference_IsUndefined(ref, &isUndefined);
    }
    return isUndefined;
}

void HiAppEventAniUtil::ParseRecord(ani_env *env, ani_ref recordRef, std::map<std::string, ani_ref>& recordResult)
{
    if (env == nullptr) {
        return;
    }
    ani_ref keys {};
    if (env->Object_CallMethodByName_Ref(static_cast<ani_object>(recordRef), "keys",
        ":C{std.core.IterableIterator}", &keys) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method keys() failed.");
    }
    ani_boolean done = ANI_FALSE;
    while (!done) {
        ani_ref next {};
        if (env->Object_CallMethodByName_Ref(static_cast<ani_object>(keys), "next", nullptr, &next) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "call method next() failed.");
            break;
        }
        if (env->Object_GetFieldByName_Boolean(static_cast<ani_object>(next), "done", &done) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get field done failed.");
            break;
        }
        if (done) {
            break;
        }
        ani_ref keyRef {};
        if (env->Object_GetFieldByName_Ref(static_cast<ani_object>(next), "value", &keyRef) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get field value failed.");
            break;
        }
        ani_ref valueRef {};
        if (env->Object_CallMethodByName_Ref(static_cast<ani_object>(recordRef),
            "$_get", nullptr, &valueRef, keyRef) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "call method $_get failed.");
            break;
        }
        std::string keyStr = ParseStringValue(env, keyRef);
        recordResult[keyStr] = valueRef;
    }
}

std::string HiAppEventAniUtil::ParseStringValue(ani_env *env, ani_ref aniStrRef)
{
    if (env == nullptr) {
        return "";
    }
    ani_size strSize = 0;
    if (env->String_GetUTF8Size(static_cast<ani_string>(aniStrRef), &strSize) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get ani string size failed");
        return "";
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytesWritten = 0;
    if (env->String_GetUTF8(static_cast<ani_string>(aniStrRef), utf8Buffer, strSize + 1, &bytesWritten) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get ani string failed");
        return "";
    }
    utf8Buffer[bytesWritten] = '\0';
    std::string content = std::string(utf8Buffer);
    return content;
}

int32_t HiAppEventAniUtil::ParseIntValue(ani_env *env, ani_ref elementRef)
{
    ani_int intVal = 0;
    if (env == nullptr) {
        return static_cast<int32_t>(intVal);
    }
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_INT, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_INT);
        return static_cast<int32_t>(intVal);
    }
    ani_method unboxedMethod {};
    if (env->Class_FindMethod(cls, FUNC_NAME_TOINT, ":i", &unboxedMethod) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_TOINT);
        return static_cast<int32_t>(intVal);
    }
    if (env->Object_CallMethod_Int(static_cast<ani_object>(elementRef), unboxedMethod, &intVal) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_TOINT);
    }
    return static_cast<int32_t>(intVal);
}

int64_t HiAppEventAniUtil::ParseLongValue(ani_env *env, ani_ref elementRef)
{
    ani_long longVal = 0;
    if (env == nullptr) {
        return static_cast<int64_t>(longVal);
    }
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_LONG, &cls)) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_LONG);
        return static_cast<int64_t>(longVal);
    }
    ani_method unboxedMethod {};
    if (env->Class_FindMethod(cls, FUNC_NAME_TOLONG, ":l", &unboxedMethod) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_TOLONG);
        return static_cast<int64_t>(longVal);
    }
    if (env->Object_CallMethod_Long(static_cast<ani_object>(elementRef), unboxedMethod, &longVal) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_TOLONG);
    }
    return static_cast<int64_t>(longVal);
}

bool HiAppEventAniUtil::ParseBoolValue(ani_env *env, ani_ref elementRef)
{
    if (env == nullptr) {
        return false;
    }
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_BOOLEAN, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BOOLEAN);
        return false;
    }
    ani_method unboxedMethod {};
    if (env->Class_FindMethod(cls, FUNC_NAME_TOBOOLEAN, ":z", &unboxedMethod) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_TOBOOLEAN);
        return false;
    }
    ani_boolean booleanVal = static_cast<ani_boolean>(false);
    if (env->Object_CallMethod_Boolean(static_cast<ani_object>(elementRef), unboxedMethod, &booleanVal) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_TOBOOLEAN);
        return false;
    }
    return static_cast<bool>(booleanVal);
}

double HiAppEventAniUtil::ParseDoubleValue(ani_env *env, ani_ref elementRef)
{
    ani_double doubleVal = 0;
    if (env == nullptr) {
        return static_cast<double>(doubleVal);
    }
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_DOUBLE, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return static_cast<double>(doubleVal);
    }
    ani_method unboxedMethod {};
    if (env->Class_FindMethod(cls, FUNC_NAME_TODOUBLE, ":d", &unboxedMethod) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method %{public}s failed", FUNC_NAME_TODOUBLE);
        return static_cast<double>(doubleVal);
    }
    if (env->Object_CallMethod_Double(static_cast<ani_object>(elementRef), unboxedMethod, &doubleVal) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method %{public}s failed", FUNC_NAME_TODOUBLE);
    }
    return static_cast<double>(doubleVal);
}

void HiAppEventAniUtil::GetStringsToSet(ani_env *env, ani_ref Ref, std::unordered_set<std::string> &arr)
{
    if (env == nullptr) {
        return;
    }
    ani_size length = 0;
    if (env->Array_GetLength(static_cast<ani_array>(Ref), &length) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get array length failed");
    }
    for (ani_size i = 0; i < length; i++) {
        ani_ref value {};
        if (env->Array_Get(static_cast<ani_array>(Ref), i, &value) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get array element failed");
            continue;
        }
        arr.insert(ParseStringValue(env, static_cast<ani_string>(value)));
    }
}

void HiAppEventAniUtil::GetIntValueToVector(ani_env *env, ani_ref Ref, std::vector<int> &arr)
{
    if (env == nullptr) {
        return;
    }
    ani_size length = 0;
    if (env->Array_GetLength(static_cast<ani_array>(Ref), &length) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get array length failed");
        return;
    }
    for (ani_size i = 0; i < length; i++) {
        ani_ref value {};
        if (env->Array_Get(static_cast<ani_array>(Ref), i, &value) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "get array element failed");
            continue;
        }
        ani_int aniInt = 0;
        if (env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(value), &aniInt) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "enumItem get int value failed");
            return;
        }
        arr.emplace_back(static_cast<int32_t>(aniInt));
    }
}

ani_ref HiAppEventAniUtil::GetProperty(ani_env *env, ani_object object, const std::string &name)
{
    ani_ref value = nullptr;
    if (env == nullptr) {
        return value;
    }
    if (env->Object_GetPropertyByName_Ref(object, name.c_str(), &value) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get property %{public}s from object", name.c_str());
    }
    return value;
}

void HiAppEventAniUtil::ThrowAniError(ani_env *env, int32_t code, const std::string &message)
{
    if (env == nullptr) {
        return;
    }
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_BUSINESSERROR, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return;
    }
    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method BusinessError constructor failed");
        return;
    }
    ani_object error {};
    if (env->Object_New(cls, ctor, &error) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "new object %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return;
    }
    if (env->Object_SetPropertyByName_Int(error, "code", static_cast<ani_int>(code)) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "set property BusinessError.code failed");
        return;
    }
    ani_string messageRef {};
    if (env->String_NewUTF8(message.c_str(), message.size(), &messageRef) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "new message string failed");
        return;
    }
    if (env->Object_SetPropertyByName_Ref(error, "message", static_cast<ani_ref>(messageRef)) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "set property BusinessError.message failed");
        return;
    }
    if (env->ThrowError(static_cast<ani_error>(error)) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "throwError ani_error object failed");
    }
}

ani_object HiAppEventAniUtil::Result(ani_env *env, std::pair<int32_t, std::string> result)
{
    ani_object results_obj {};
    if (env == nullptr) {
        return results_obj;
    }

    ani_class cls {};
    if (env->FindClass(CLASS_NAME_RESULTS, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to find class %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get method %{public}s <ctor> failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    if (env->Object_New(cls, ctor, &results_obj) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "create object %{public}s failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method codeSetter {};
    if (env->Class_FindMethod(cls, Builder::BuildSetterName("code").c_str(), nullptr, &codeSetter) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get method codeSetter %{public}s failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    if (env->Object_CallMethod_Void(results_obj, codeSetter, result.first) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method codeSetter %{public}s failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_method messageSetter {};
    if (env->Class_FindMethod(cls, Builder::BuildSetterName("message").c_str(), nullptr, &messageSetter) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find method messageSetter %{public}s failed", CLASS_NAME_RESULTS);
        return results_obj;
    }

    ani_string message_string = HiAppEventAniUtil::CreateAniString(env, result.second);
    if (env->Object_CallMethod_Void(results_obj, messageSetter, message_string) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "call method messageSetter Fail %{public}s", CLASS_NAME_RESULTS);
        return results_obj;
    }

    return results_obj;
}

std::pair<int32_t, std::string> HiAppEventAniUtil::BuildErrorByResult(int32_t result)
{
    const std::map<int32_t, std::pair<int32_t, std::string>> codeMap = {
        { ERR_CODE_SUCC,
            { ERR_CODE_SUCC, "Success." } },
        { ErrorCode::ERROR_INVALID_EVENT_NAME,
            { ERR_INVALID_NAME, "Invalid event name." } },
        { ErrorCode::ERROR_INVALID_EVENT_DOMAIN,
            { ERR_INVALID_DOMAIN, "Invalid event domain." } },
        { ErrorCode::ERROR_HIAPPEVENT_DISABLE,
            { ERR_DISABLE, "Function disabled." } },
        { ErrorCode::ERROR_INVALID_PARAM_NAME,
            { ERR_INVALID_KEY, "Invalid event parameter name." } },
        { ErrorCode::ERROR_INVALID_PARAM_VALUE_LENGTH,
            { ERR_INVALID_STR_LEN, "Invalid string length of the event parameter." } },
        { ErrorCode::ERROR_INVALID_PARAM_NUM,
            { ERR_INVALID_PARAM_NUM, "Invalid number of event parameters." } },
        { ErrorCode::ERROR_INVALID_LIST_PARAM_SIZE,
            { ERR_INVALID_ARR_LEN, "Invalid array length of the event parameter." } },
        { ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM,
            { ERR_INVALID_CUSTOM_PARAM_NUM, "The number of parameter keys exceeds the limit." }},
    };
    return codeMap.at(result);
}

AniArgsType HiAppEventAniUtil::GetArgType(ani_env *env, ani_object elementObj)
{
    if (env == nullptr) {
        return AniArgsType::ANI_UNKNOWN;
    }
    if (HiAppEventAniUtil::IsRefUndefined(env, static_cast<ani_ref>(elementObj))) {
        return AniArgsType::ANI_UNDEFINED;
    }
    for (const auto &objType : OBJECT_TYPE) {
        ani_class cls {};
        if (env->FindClass(objType.first, &cls) != ANI_OK) {
            continue;
        }
        ani_boolean isInstance = ANI_FALSE;
        if (env->Object_InstanceOf(elementObj, cls, &isInstance) != ANI_OK) {
            continue;
        }
        if (static_cast<bool>(isInstance)) {
            return objType.second;
        }
    }
    return AniArgsType::ANI_UNKNOWN;
}

AniArgsType HiAppEventAniUtil::GetArrayType(ani_env *env, ani_ref arrayRef)
{
    if (env == nullptr) {
        return AniArgsType::ANI_UNKNOWN;
    }
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array>(arrayRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return AniArgsType::ANI_UNDEFINED;
    }
    AniArgsType type = AniArgsType::ANI_NULL; // note: empty array returns null type
    for (ani_size i = 0; i < len; ++i) {
        ani_ref element = nullptr;
        if (env->Array_Get(static_cast<ani_array>(arrayRef), i, &element) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return AniArgsType::ANI_UNDEFINED;
        }
        if (i == 0) {
            type = GetArgType(env, static_cast<ani_object>(element));
            continue;
        }
        if (type != GetArgType(env, static_cast<ani_object>(element))) {
            HILOG_ERROR(LOG_CORE, "array has different element types");
            return AniArgsType::ANI_UNDEFINED;
        }
    }
    return type;
}

std::string HiAppEventAniUtil::ConvertToString(ani_env *env, ani_ref valueRef)
{
    if (env == nullptr) {
        return "";
    }
    AniArgsType type = GetArgType(env, static_cast<ani_object>(valueRef));
    std::string result = "";
    switch (type) {
        case AniArgsType::ANI_BOOLEAN:
            result = HiAppEventAniUtil::ParseBoolValue(env, valueRef) ? "true" : "false";
            break;
        case AniArgsType::ANI_INT:
            result = std::to_string(ParseIntValue(env, valueRef));
            break;
        case AniArgsType::ANI_LONG:
            result = std::to_string(ParseLongValue(env, valueRef));
            break;
        case AniArgsType::ANI_DOUBLE:
            result = std::to_string(ParseDoubleValue(env, valueRef));
            break;
        case AniArgsType::ANI_STRING:
            result = HiAppEventAniUtil::ParseStringValue(env, valueRef);
            break;
        default:
            break;
    }
    return result;
}

ani_ref HiAppEventAniUtil::CreateGlobalReference(ani_env *env, ani_ref func)
{
    ani_ref objectGRef {};
    if (env == nullptr) {
        return objectGRef;
    }
    if (env->GlobalReference_Create(func, &objectGRef) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to create global reference");
        return objectGRef;
    }
    return objectGRef;
}

ani_object HiAppEventAniUtil::CreateInt(ani_env *env, int32_t num)
{
    ani_class cls {};
    ani_object obj {};
    if (env == nullptr) {
        return obj;
    }
    if (env->FindClass(CLASS_NAME_INT, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_INT);
        return obj;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", "i:", &ctor);
    env->Object_New(cls, ctor, &obj, static_cast<ani_int>(num));
    return obj;
}

ani_object HiAppEventAniUtil::CreateDouble(ani_env *env, int32_t num)
{
    ani_class cls {};
    ani_object obj {};
    if (env == nullptr) {
        return obj;
    }
    if (env->FindClass(CLASS_NAME_DOUBLE, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_DOUBLE);
        return obj;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", "d:", &ctor);
    env->Object_New(cls, ctor, &obj, static_cast<ani_double>(num));
    return obj;
}

static ani_enum_item ToAniEnum(ani_env *env, EventTypeAni value)
{
    ani_enum_item aniEnumItem {};
    if (env == nullptr) {
        return aniEnumItem;
    }
    auto it = ANI_EVENTTYPE_INDEX_MAP.find(value);
    if (it == ANI_EVENTTYPE_INDEX_MAP.end()) {
        HILOG_ERROR(LOG_CORE, "Unsupport enum: %{public}d", value);
        return aniEnumItem;
    }
    ani_int enumIndex = static_cast<ani_int>(it->second);

    ani_enum aniEnum {};
    if (env->FindEnum(ENUM_NAME_EVENT_TYPE, &aniEnum) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find enum %{public}s failed", ENUM_NAME_EVENT_TYPE);
        return aniEnumItem;
    }
    if (env->Enum_GetEnumItemByIndex(aniEnum, enumIndex, &aniEnumItem)) {
        HILOG_ERROR(LOG_CORE, "get enum eventType value failed");
        return aniEnumItem;
    }
    return aniEnumItem;
}

ani_object HiAppEventAniUtil::CreateBool(ani_env *env, bool boolValue)
{
    ani_class cls {};
    ani_object obj {};
    if (env == nullptr) {
        return obj;
    }
    if (env->FindClass(CLASS_NAME_BOOLEAN, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "find class %{public}s failed", CLASS_NAME_BOOLEAN);
        return obj;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", "l:", &ctor);
    env->Object_New(cls, ctor, &obj, static_cast<ani_boolean>(boolValue));
    return obj;
}

ani_string HiAppEventAniUtil::CreateAniString(ani_env *env, const std::string &str)
{
    ani_string aniString {};
    if (env != nullptr) {
        env->String_NewUTF8(str.c_str(), str.size(), &aniString);
    }
    return aniString;
}

static ani_ref CreateArray(ani_env *env, size_t length)
{
    ani_class cls {};
    ani_object array {};
    if (env == nullptr) {
        return array;
    }
    if (env->FindClass(CLASS_NAME_ARRAY, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_ARRAY);
        return array;
    }
    ani_method method {};
    if (env->Class_FindMethod(cls, "<ctor>", "i:", &method) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "%{public}s Find Method ctor Failed", CLASS_NAME_ARRAY);
        return array;
    }
    if (env->Object_New(cls, method, &array, length) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "%{public}s Call Method ctor Failed", CLASS_NAME_ARRAY);
        return array;
    }
    return array;
}

static ani_ref CreateArray(ani_env *env, const std::string &name, ani_size length)
{
    ani_array array {};
    if (env == nullptr) {
        return array;
    }
    if (env->Array_New(length, nullptr, &array) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "New %{public}s Array Ref Failed.", name.c_str());
        return array;
    }
    return array;
}

static ani_method FindArrayMethodSet(ani_env *env)
{
    ani_class cls {};
    ani_method setMethod {};
    if (env == nullptr) {
        return setMethod;
    }
    if (env->FindClass(CLASS_NAME_ARRAY, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_ARRAY);
        return setMethod;
    }
    if (env->Class_FindMethod(cls, "$_set", "iY:", &setMethod) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindMethod $_set %{public}s Failed", CLASS_NAME_ARRAY);
        return setMethod;
    }
    return setMethod;
}

ani_ref HiAppEventAniUtil::CreateStrings(ani_env *env, const std::vector<std::string>& strs)
{
    ani_ref arr = CreateArray(env, CLASS_NAME_STRING, strs.size());
    for (size_t i = 0; i < strs.size(); ++i) {
        env->Array_Set(static_cast<ani_array>(arr),
            static_cast<ani_size>(i), HiAppEventAniUtil::CreateAniString(env, strs[i]));
    }
    return arr;
}

ani_object HiAppEventAniUtil::CreateObject(ani_env *env, const std::string &name)
{
    if (env == nullptr) {
        return nullptr;
    }

    ani_class cls {};
    if (env->FindClass(name.c_str(), &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", name.c_str());
    }

    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get %{public}s ctor Failed", name.c_str());
    }

    ani_object obj {};
    if (env->Object_New(cls, ctor, &obj) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "Create Object Failed: %{public}s", name.c_str());
    }
    return obj;
}

static ani_ref CreateBaseValueByJson(ani_env *env, const Json::Value& jsonValue)
{
    if (env == nullptr) {
        return nullptr;
    }
    if (jsonValue.isBool()) {
        return HiAppEventAniUtil::CreateBool(env, jsonValue.asBool());
    }
    if (jsonValue.isDouble()) {
        return HiAppEventAniUtil::CreateDouble(env, jsonValue.asDouble());
    }
    if (jsonValue.isString()) {
        return HiAppEventAniUtil::CreateAniString(env, jsonValue.asString());
    }
    return nullptr;
}

static ani_method GetRecordSetMethod(ani_env *env)
{
    if (env == nullptr) {
        return nullptr;
    }

    ani_class cls {};
    if (env->FindClass(CLASS_NAME_RECORD, &cls) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_RECORD);
        return nullptr;
    }

    ani_method set {};
    if (env->Class_FindMethod(cls, "$_set", nullptr, &set) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "get %{public}s set Failed", CLASS_NAME_RECORD);
        return nullptr;
    }
    return set;
}

static ani_ref CreateValueByJson(ani_env *env, const Json::Value& jsonValue)
{
    if (env == nullptr) {
        return nullptr;
    }

    if (jsonValue.isArray()) {
        if (jsonValue[0].isBool()) {
            ani_ref boolArray = CreateArray(env, CLASS_NAME_BOOLEAN, jsonValue.size());
            for (Json::ArrayIndex i = 0; i < jsonValue.size(); ++i) {
                env->Array_Set(static_cast<ani_array>(boolArray), static_cast<ani_size>(i),
                    CreateValueByJson(env, jsonValue[static_cast<int>(i)]));
            }
            return boolArray;
        }
        if (jsonValue[0].isDouble()) {
            ani_ref doubleArray = CreateArray(env, CLASS_NAME_DOUBLE, jsonValue.size());
            for (Json::ArrayIndex i = 0; i < jsonValue.size(); ++i) {
                env->Array_Set(static_cast<ani_array>(doubleArray), static_cast<ani_size>(i),
                    CreateValueByJson(env, jsonValue[static_cast<int>(i)]));
            }
            return doubleArray;
        }
        if (jsonValue[0].isString()) {
            ani_ref stringArray = CreateArray(env, CLASS_NAME_STRING, jsonValue.size());
            for (Json::ArrayIndex i = 0; i < jsonValue.size(); ++i) {
                env->Array_Set(static_cast<ani_array>(stringArray), static_cast<ani_size>(i),
                    CreateValueByJson(env, jsonValue[static_cast<int>(i)]));
            }
            return stringArray;
        }
    }
    if (jsonValue.isObject()) {
        ani_object obj = HiAppEventAniUtil::CreateObject(env, CLASS_NAME_RECORD);
        ani_method set = GetRecordSetMethod(env);
        auto eventNameList = jsonValue.getMemberNames();
        for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); ++it) {
            auto propertyName = *it;
            if (env->Object_CallMethod_Void(obj, set, HiAppEventAniUtil::CreateAniString(env, propertyName),
                CreateValueByJson(env, jsonValue[propertyName])) != ANI_OK) {
                HILOG_ERROR(LOG_CORE, "set record params Fail: %{public}s", CLASS_NAME_RECORD);
                return obj;
            }
        }
        return obj;
    }
    return CreateBaseValueByJson(env, jsonValue);
}

static ani_ref CreateValueByJsonStr(ani_env *env, const std::string& jsonStr)
{
    if (env == nullptr) {
        return nullptr;
    }
    Json::Value jsonValue;
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, jsonValue)) {
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json");
        return nullptr;
    }
    return CreateValueByJson(env, jsonValue);
}

static ani_object CreateEventInfo(ani_env *env, std::shared_ptr<AppEventPack> event)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_object obj = HiAppEventAniUtil::CreateObject(env, CLASS_NAME_EVENT_INFO);
    env->Object_SetPropertyByName_Ref(obj, EVENT_CONFIG_DOMAIN.c_str(),
        HiAppEventAniUtil::CreateAniString(env, event->GetDomain()));
    env->Object_SetPropertyByName_Ref(obj, EVENT_CONFIG_NAME.c_str(),
        HiAppEventAniUtil::CreateAniString(env, event->GetName()));
    env->Object_SetPropertyByName_Ref(obj, EVENT_INFO_EVENT_TYPE.c_str(),
        ToAniEnum(env, static_cast<EventTypeAni>(event->GetType())));
    env->Object_SetPropertyByName_Ref(obj, EVENT_INFO_PARAMS.c_str(),
        CreateValueByJsonStr(env, event->GetParamStr()));
    return obj;
}

ani_ref HiAppEventAniUtil::CreateEventInfoArray(ani_env *env, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    ani_ref arr = CreateArray(env, events.size());
    ani_method setMethod = FindArrayMethodSet(env);
    for (size_t i = 0; i < events.size(); ++i) {
        env->Object_CallMethod_Void(static_cast<ani_object>(arr), setMethod, i, CreateEventInfo(env, events[i]));
    }
    return arr;
}

ani_ref HiAppEventAniUtil::CreateEventGroups(ani_env *env, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::unordered_map<std::string, std::vector<std::shared_ptr<AppEventPack>>> eventMap;
    for (auto event : events) {
        eventMap[event->GetName()].emplace_back(event);
    }

    ani_ref eventGroups = CreateArray(env, eventMap.size());
    ani_method setMethod = FindArrayMethodSet(env);
    size_t index = 0;
    for (auto it = eventMap.begin(); it != eventMap.end(); ++it) {
        ani_ref eventInfos = CreateArray(env, it->second.size());
        for (size_t i = 0; i < it->second.size(); ++i) {
            env->Object_CallMethod_Void(static_cast<ani_object>(eventInfos),
                setMethod, i, CreateEventInfo(env, it->second[i]));
        }
        ani_object obj = HiAppEventAniUtil::CreateObject(env, CLASS_NAME_EVENT_GROUP);
        env->Object_SetPropertyByName_Ref(obj, EVENT_CONFIG_NAME.c_str(),
            HiAppEventAniUtil::CreateAniString(env, it->first));
        env->Object_SetPropertyByName_Ref(obj, EVENT_INFOS_PROPERTY.c_str(), eventInfos);
        env->Object_CallMethod_Void(static_cast<ani_object>(eventGroups), setMethod, index, obj);
        ++index;
    }
    return eventGroups;
}

std::vector<bool> HiAppEventAniUtil::GetBooleans(ani_env *env, ani_ref arrayRef)
{
    std::vector<bool> bools;
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array>(arrayRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return bools;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (env->Array_Get(static_cast<ani_array>(arrayRef), i, &element) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            break;
        }
        bools.emplace_back(HiAppEventAniUtil::ParseBoolValue(env, element));
    }
    return bools;
}

std::vector<double> HiAppEventAniUtil::GetDoubles(ani_env *env, ani_ref arrayRef)
{
    std::vector<double> doubles;
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array>(arrayRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return doubles;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (env->Array_Get(static_cast<ani_array>(arrayRef), i, &element) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            break;
        }
        doubles.emplace_back(HiAppEventAniUtil::ParseDoubleValue(env, element));
    }
    return doubles;
}

std::vector<std::string> HiAppEventAniUtil::GetStrings(ani_env *env, ani_ref arrayRef)
{
    std::vector<std::string> strs;
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array>(arrayRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return strs;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (env->Array_Get(static_cast<ani_array>(arrayRef), i, &element) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            break;
        }
        strs.emplace_back(HiAppEventAniUtil::ParseStringValue(env, element));
    }
    return strs;
}

std::vector<int> HiAppEventAniUtil::GetInts(ani_env *env, ani_ref arrayRef)
{
    std::vector<int> ints;
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array>(arrayRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return ints;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (env->Array_Get(static_cast<ani_array>(arrayRef), i, &element) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            break;
        }
        ints.emplace_back(HiAppEventAniUtil::ParseIntValue(env, element));
    }
    return ints;
}

std::vector<int64_t> HiAppEventAniUtil::GetLongs(ani_env *env, ani_ref arrayRef)
{
    std::vector<int64_t> longs;
    ani_size len = 0;
    if (env->Array_GetLength(static_cast<ani_array>(arrayRef), &len) != ANI_OK) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return longs;
    }
    for (ani_size i = 0; i < len; i++) {
        ani_ref element = nullptr;
        if (env->Array_Get(static_cast<ani_array>(arrayRef), i, &element) != ANI_OK) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            break;
        }
        longs.emplace_back(HiAppEventAniUtil::ParseLongValue(env, element));
    }
    return longs;
}
