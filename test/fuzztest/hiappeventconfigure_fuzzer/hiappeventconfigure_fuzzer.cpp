#define FUZZ_PROJECT_NAME "hiappeventconfigure_fuzzer"

#include "hiappeventconfigure_fuzzer.h"
#include <cstring>
#include "hiappevent/hiappevent.h"
#include "hiappevent/hiappevent_cfg.h"

namespace {
constexpr int MAX_STR_LEN = 64;

struct FuzzData {
    char nameStr[MAX_STR_LEN];
    char valueStr[MAX_STR_LEN];
};

void FuzzConfigure(const FuzzData& data)
{
    OH_HiAppEvent_Configure(data.nameStr, data.valueStr);
    OH_HiAppEvent_Configure(DISABLE, data.valueStr);
    OH_HiAppEvent_Configure(MAX_STORAGE, data.valueStr);
    OH_HiAppEvent_Configure(data.nameStr, "true");
    OH_HiAppEvent_Configure(data.nameStr, "false");
    OH_HiAppEvent_Configure(data.nameStr, "100M");
}

void FuzzConfigureNull()
{
    OH_HiAppEvent_Configure(nullptr, "true");
    OH_HiAppEvent_Configure("disable", nullptr);
    OH_HiAppEvent_Configure(nullptr, nullptr);
}

void FuzzClearData()
{
    OH_HiAppEvent_ClearData();
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzConfigure(fuzzData);
    FuzzConfigureNull();
    FuzzClearData();
    return 0;
}
