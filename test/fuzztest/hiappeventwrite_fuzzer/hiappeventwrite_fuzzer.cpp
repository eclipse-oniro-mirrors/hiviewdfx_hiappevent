#define FUZZ_PROJECT_NAME "hiappeventwrite_fuzzer"

#include "hiappeventwrite_fuzzer.h"
#include <cstring>
#include "hiappevent/hiappevent.h"

namespace {
constexpr int MAX_STR_LEN = 64;
constexpr int MAX_ARR_LEN = 10;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_BASE = 1;
constexpr int LOCAL_ARR_LEN = 2;
constexpr int DEFAULT_ARR_LEN = 1;

struct FuzzData {
    int8_t int8Val;
    int16_t int16Val;
    int32_t int32Val;
    int64_t int64Val;
    float floatVal;
    double doubleVal;
    bool boolVal;
    int8_t int8Arr[MAX_ARR_LEN];
    int16_t int16Arr[MAX_ARR_LEN];
    int32_t int32Arr[MAX_ARR_LEN];
    int64_t int64Arr[MAX_ARR_LEN];
    float floatArr[MAX_ARR_LEN];
    double doubleArr[MAX_ARR_LEN];
    bool boolArr[MAX_ARR_LEN];
    char domainStr[MAX_STR_LEN];
    char nameStr[MAX_STR_LEN];
    char strVal[MAX_STR_LEN];
    char arrSize;
    char eventType;
};

void FuzzWriteWithParams(const FuzzData& data)
{
    ParamList list = OH_HiAppEvent_CreateParamList();
    if (list == nullptr) {
        return;
    }
    OH_HiAppEvent_AddBoolParam(list, "bool_key", data.boolVal);
    OH_HiAppEvent_AddInt8Param(list, "int8_key", data.int8Val);
    OH_HiAppEvent_AddInt16Param(list, "int16_key", data.int16Val);
    OH_HiAppEvent_AddInt32Param(list, "int32_key", data.int32Val);
    OH_HiAppEvent_AddInt64Param(list, "int64_key", data.int64Val);
    OH_HiAppEvent_AddFloatParam(list, "float_key", data.floatVal);
    OH_HiAppEvent_AddDoubleParam(list, "double_key", data.doubleVal);
    OH_HiAppEvent_AddStringParam(list, "str_key", data.strVal);

    int arrSize = data.arrSize % (MAX_ARR_LEN + 1);
    OH_HiAppEvent_AddBoolArrayParam(list, "bool_arr_key", data.boolArr, arrSize);
    OH_HiAppEvent_AddInt8ArrayParam(list, "int8_arr_key", data.int8Arr, arrSize);
    OH_HiAppEvent_AddInt16ArrayParam(list, "int16_arr_key", data.int16Arr, arrSize);
    OH_HiAppEvent_AddInt32ArrayParam(list, "int32_arr_key", data.int32Arr, arrSize);
    OH_HiAppEvent_AddInt64ArrayParam(list, "int64_arr_key", data.int64Arr, arrSize);
    OH_HiAppEvent_AddFloatArrayParam(list, "float_arr_key", data.floatArr, arrSize);
    OH_HiAppEvent_AddDoubleArrayParam(list, "double_arr_key", data.doubleArr, arrSize);

    const char* strArr[] = {data.strVal, data.domainStr};
    OH_HiAppEvent_AddStringArrayParam(list, "str_arr_key", strArr, arrSize % (LOCAL_ARR_LEN + 1));

    int type = (data.eventType % EVENT_TYPE_COUNT) + EVENT_TYPE_BASE;
    OH_HiAppEvent_Write(data.domainStr, data.nameStr, static_cast<enum EventType>(type), list);
    OH_HiAppEvent_DestroyParamList(list);
}

void FuzzWriteNullParams(const FuzzData& data)
{
    int type = (data.eventType % EVENT_TYPE_COUNT) + EVENT_TYPE_BASE;
    OH_HiAppEvent_Write(data.domainStr, data.nameStr, static_cast<enum EventType>(type), nullptr);
    OH_HiAppEvent_Write(nullptr, data.nameStr, static_cast<enum EventType>(type), nullptr);
    OH_HiAppEvent_Write(data.domainStr, nullptr, static_cast<enum EventType>(type), nullptr);
    OH_HiAppEvent_Write(nullptr, nullptr, static_cast<enum EventType>(type), nullptr);
}

void FuzzParamListOps(const FuzzData& data)
{
    ParamList list = OH_HiAppEvent_CreateParamList();
    OH_HiAppEvent_AddBoolParam(nullptr, "key", data.boolVal);
    OH_HiAppEvent_AddInt32Param(nullptr, "key", data.int32Val);
    OH_HiAppEvent_AddStringParam(nullptr, "key", data.strVal);
    OH_HiAppEvent_AddBoolArrayParam(nullptr, "key", data.boolArr, DEFAULT_ARR_LEN);
    OH_HiAppEvent_DestroyParamList(list);
    OH_HiAppEvent_DestroyParamList(nullptr);
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzWriteWithParams(fuzzData);
    FuzzWriteNullParams(fuzzData);
    FuzzParamListOps(fuzzData);
    return 0;
}
