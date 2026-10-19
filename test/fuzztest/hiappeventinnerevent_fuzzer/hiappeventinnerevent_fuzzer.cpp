#define FUZZ_PROJECT_NAME "hiappeventinnerevent_fuzzer"

#include "hiappeventinnerevent_fuzzer.h"
#include <cstring>
#include <string>
#include <vector>
#include "app_event.h"

namespace {
constexpr int MAX_STR_LEN = 64;
constexpr int MAX_ARR_LEN = 10;
constexpr int EVENT_TYPE_COUNT = 4;
constexpr int EVENT_TYPE_BASE = 1;

struct FuzzData {
    char domainStr[MAX_STR_LEN];
    char nameStr[MAX_STR_LEN];
    char keyStr[MAX_STR_LEN];
    char valStr[MAX_STR_LEN];
    bool boolVal;
    int32_t int32Val;
    int64_t int64Val;
    double doubleVal;
    bool boolArr[MAX_ARR_LEN];
    int32_t int32Arr[MAX_ARR_LEN];
    int64_t int64Arr[MAX_ARR_LEN];
    double doubleArr[MAX_ARR_LEN];
    char arrSize;
    char eventType;
};

void FuzzEventWrite(const FuzzData& data)
{
    using namespace OHOS::HiviewDFX::HiAppEvent;
    int type = (data.eventType % EVENT_TYPE_COUNT) + EVENT_TYPE_BASE;
    Event event(data.domainStr, data.nameStr, static_cast<EventType>(type));

    event.AddParam(data.keyStr, data.boolVal);
    event.AddParam(data.keyStr, data.int32Val);
    event.AddParam(data.keyStr, data.int64Val);
    event.AddParam(data.keyStr, data.doubleVal);
    event.AddParam(data.keyStr, std::string(data.valStr));

    int arrSize = data.arrSize % (MAX_ARR_LEN + 1);
    std::vector<bool> boolVec(data.boolArr, data.boolArr + arrSize);
    std::vector<int32_t> int32Vec(data.int32Arr, data.int32Arr + arrSize);
    std::vector<int64_t> int64Vec(data.int64Arr, data.int64Arr + arrSize);
    std::vector<double> doubleVec(data.doubleArr, data.doubleArr + arrSize);
    std::vector<std::string> strVec = {std::string(data.valStr), std::string(data.domainStr)};
    event.AddParam(data.keyStr, boolVec);
    event.AddParam(data.keyStr, int32Vec);
    event.AddParam(data.keyStr, int64Vec);
    event.AddParam(data.keyStr, doubleVec);
    event.AddParam(data.keyStr, strVec);

    Write(event);
}

void FuzzEventEmptyWrite(const FuzzData& data)
{
    using namespace OHOS::HiviewDFX::HiAppEvent;
    int type = (data.eventType % EVENT_TYPE_COUNT) + EVENT_TYPE_BASE;
    Event event("", "", static_cast<EventType>(type));
    Write(event);
}

void FuzzEventNullDomainName()
{
    using namespace OHOS::HiviewDFX::HiAppEvent;
    Event event(std::string(""), std::string(""), EventType::FAULT);
    event.AddParam(std::string(""), true);
    event.AddParam(std::string(""), int32_t(0));
    Write(event);
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzEventWrite(fuzzData);
    FuzzEventEmptyWrite(fuzzData);
    FuzzEventNullDomainName();
    return 0;
}
