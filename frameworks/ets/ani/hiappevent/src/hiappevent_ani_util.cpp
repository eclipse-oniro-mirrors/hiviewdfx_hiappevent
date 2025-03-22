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

using namespace OHOS::HiviewDFX;

bool HiAppEventAniUtil::IsArray(ani_env *env, ani_object object)
{
    ani_boolean IsArray = ANI_FALSE;
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_ARRAY, &cls)) {
        return false;
    }
    ani_static_method static_method {};
    if (ANI_OK != env->Class_FindStaticMethod(cls, "isArray", nullptr, &static_method)) {
        return false;
    }
    if (ANI_OK != env->Class_CallStaticMethod_Boolean(cls, static_method, &IsArray, object)) {
        return false;
    }
    return static_cast<bool>(IsArray);
}

bool HiAppEventAniUtil::IsRefUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    env->Reference_IsUndefined(ref, &isUndefined);
    return isUndefined;
}

static void GetKeyValueFormIterArray(ani_env *env, ani_ref param, std::string &key, ani_ref &value)
{
    ani_size keyIndex = 0;
    ani_size valueIndex = keyIndex + 1;
    ani_ref keyRef {};
    if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(param), keyIndex, &keyRef)) {
        HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}zu Failed", keyIndex);
    }
    key = HiAppEventAniUtil::ParseStringValue(env, keyRef);
    if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(param), valueIndex, &value)) {
        HILOG_ERROR(LOG_CORE, "Array_Get_Ref %{public}zu Failed", valueIndex);
    }
}

static void ParseIterValue(ani_env *env, ani_ref iterValue,
    std::map<std::string, ani_ref>& recordResult)
{
    if (HiAppEventAniUtil::IsArray(env, static_cast<ani_object>(iterValue))) {
        std::string key = "";
        ani_ref value {};
        GetKeyValueFormIterArray(env, iterValue, key, value);
        recordResult[key] = value;
    }
}

static void ParseIterator(ani_env *env, ani_ref iteratorRef, std::map<std::string, ani_ref>& recordResult)
{
    ani_class IteratorCls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_ITERATOR, &IteratorCls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_ITERATOR);
    }
    ani_method nextMethod {};
    if (ANI_OK != env->Class_FindMethod(IteratorCls, FUNC_NAME_NEXT, nullptr, &nextMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_NEXT);
    }
    ani_object IteratorObj = static_cast<ani_object>(iteratorRef);
    ani_ref iteratorResult {};
    ani_boolean done = false;
    while (!done) {
        if (ANI_OK != env->Object_CallMethod_Ref(IteratorObj, nextMethod, &iteratorResult)) {
            HILOG_ERROR(LOG_CORE, "Object_CallMethod_Ref next Failed");
        }
        ani_object iterResultObj = static_cast<ani_object>(iteratorResult);
        if (ANI_OK != env->Object_GetPropertyByName_Boolean(iterResultObj, "done", &done)) {
            HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Boolean done Failed");
        }
        if (!done) {
            ani_ref value {};
            if (ANI_OK != env->Object_GetPropertyByName_Ref(iterResultObj, "value", &value)) {
                HILOG_ERROR(LOG_CORE, "Object_GetPropertyByName_Ref value Failed");
            }
            ParseIterValue(env, value, recordResult);
        }
    }
}

void HiAppEventAniUtil::ParseRecord(ani_env *env, ani_ref recordRef, std::map<std::string, ani_ref>& recordResult)
{
    ani_class recordCls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_RECORD, &recordCls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_RECORD);
        return;
    }

    ani_method entriesMethod {};
    if (ANI_OK != env->Class_FindMethod(recordCls, "entries", nullptr, &entriesMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod entries Failed");
        return;
    }
    ani_ref iteratorRef {};
    if (ANI_OK != env->Object_CallMethod_Ref(static_cast<ani_object>(recordRef), entriesMethod, &iteratorRef)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Ref entries Failed");
        return;
    }
    ParseIterator(env, iteratorRef, recordResult);
}

std::string HiAppEventAniUtil::ParseStringValue(ani_env *env, ani_ref aniStrRef)
{
    ani_size strSize = 0;
    if (ANI_OK != env->String_GetUTF8Size(static_cast<ani_string>(aniStrRef), &strSize)) {
        HILOG_ERROR(LOG_CORE, "String_GetUTF8Size Failed");
        return "";
    }
    std::vector<char> buffer(strSize + 1);
    char* utf8Buffer = buffer.data();
    ani_size bytesWritten = 0;
    if (ANI_OK != env->String_GetUTF8(static_cast<ani_string>(aniStrRef), utf8Buffer, strSize + 1, &bytesWritten)) {
        HILOG_ERROR(LOG_CORE, "String_GetUTF8 Failed");
        return "";
    }
    utf8Buffer[bytesWritten] = '\0';
    std::string content = std::string(utf8Buffer);
    return content;
}

int32_t HiAppEventAniUtil::ParseIntValue(ani_env *env, ani_ref elementRef)
{
    ani_int intVal = 0;
    ani_class cls {};
    ani_method unboxedMethod {};
    if (env->FindClass(CLASS_NAME_INT, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_INT);
        return static_cast<int32_t>(intVal);
    }
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":I", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED);
        return static_cast<int32_t>(intVal);
    }
    if (ANI_OK != env->Object_CallMethod_Int(static_cast<ani_object>(elementRef), unboxedMethod, &intVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Boolean %{public}s Failed", FUNC_NAME_UNBOXED);
        return static_cast<int32_t>(intVal);
    }
    return static_cast<int32_t>(intVal);
}

bool HiAppEventAniUtil::ParseBoolValue(ani_env *env, ani_ref elementRef)
{
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BOOLEAN, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_BOOLEAN);
        return false;
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":Z", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED);
        return false;
    }
    ani_boolean booleanVal = static_cast<ani_boolean>(false);
    if (ANI_OK != env->Object_CallMethod_Boolean(static_cast<ani_object>(elementRef), unboxedMethod, &booleanVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Boolean %{public}s Failed", FUNC_NAME_UNBOXED);
        return false;
    }
    return static_cast<bool>(booleanVal);
}

double HiAppEventAniUtil::ParseNumberValue(ani_env *env, ani_ref elementRef)
{
    ani_double doubleVal = 0;
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_DOUBLE, &cls)) {
        HILOG_ERROR(LOG_CORE, "FindClass %{public}s Failed", CLASS_NAME_DOUBLE);
        return static_cast<double>(doubleVal);
    }
    ani_method unboxedMethod {};
    if (ANI_OK != env->Class_FindMethod(cls, FUNC_NAME_UNBOXED, ":D", &unboxedMethod)) {
        HILOG_ERROR(LOG_CORE, "Class_FindMethod %{public}s Failed", FUNC_NAME_UNBOXED);
    }
    if (ANI_OK != env->Object_CallMethod_Double(static_cast<ani_object>(elementRef), unboxedMethod, &doubleVal)) {
        HILOG_ERROR(LOG_CORE, "Object_CallMethod_Double %{public}s Failed", FUNC_NAME_UNBOXED);
    }
    return static_cast<double>(doubleVal);
}

void HiAppEventAniUtil::ParseArrayStringValue(ani_env *env, ani_ref Ref, std::vector<std::string> &arr)
{
    ani_size length = 0;
    if (ANI_OK != env->Array_GetLength(static_cast<ani_array_ref>(Ref), &length)) {
        HILOG_ERROR(LOG_CORE, "Array_GetLength length Failed");
    }
    for (ani_size i = 0; i < length; i++) {
        ani_ref value {};
        if (ANI_OK != env->Array_Get_Ref(static_cast<ani_array_ref>(Ref), i, &value)) {
            HILOG_ERROR(LOG_CORE, "Array_GetLength length Failed");
        }
        std::string valueStr = HiAppEventAniUtil::ParseStringValue(env, static_cast<ani_string>(value));
        arr.push_back(valueStr);
    }
}
