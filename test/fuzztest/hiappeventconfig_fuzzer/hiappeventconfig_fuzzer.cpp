#define FUZZ_PROJECT_NAME "hiappeventconfig_fuzzer"

#include "hiappeventconfig_fuzzer.h"
#include <cstring>
#include "hiappevent/hiappevent.h"
#include "hiappevent/hiappevent_event.h"

namespace {
constexpr int MAX_STR_LEN = 64;
constexpr int EVENT_NAME_COUNT = 6;
constexpr const char* EVENT_NAMES[] = {
    "APP_CRASH", "APP_FREEZE", "RESOURCE_OVERLIMIT", "ADDRESS_SANITIZER", "CPU_USAGE_HIGH", "MAIN_THREAD_JANK"
};

struct FuzzData {
    char itemName[MAX_STR_LEN];
    char itemValue[MAX_STR_LEN];
    char eventName[MAX_STR_LEN];
    uint8_t nameIndex;
};

void FuzzConfigOps(const FuzzData& data)
{
    HiAppEvent_Config* config = OH_HiAppEvent_CreateConfig();
    if (config == nullptr) {
        return;
    }
    OH_HiAppEvent_SetConfigItem(config, data.itemName, data.itemValue);
    OH_HiAppEvent_SetConfigItem(config, "disable", data.itemValue);
    OH_HiAppEvent_SetConfigItem(config, "max_storage", "100M");
    OH_HiAppEvent_SetConfigItem(config, data.itemName, "true");
    OH_HiAppEvent_SetConfigItem(config, data.itemName, "false");
    OH_HiAppEvent_SetConfigItem(config, data.itemName, nullptr);

    const char* eventName = EVENT_NAMES[data.nameIndex % EVENT_NAME_COUNT];
    OH_HiAppEvent_SetEventConfig(eventName, config);
    OH_HiAppEvent_SetEventConfig(data.eventName, config);
    OH_HiAppEvent_SetEventConfig(nullptr, config);
    OH_HiAppEvent_DestroyConfig(config);
}

void FuzzConfigNullOps()
{
    auto config = OH_HiAppEvent_CreateConfig();
    OH_HiAppEvent_SetConfigItem(nullptr, "key", "value");
    OH_HiAppEvent_SetConfigItem(nullptr, nullptr, nullptr);
    OH_HiAppEvent_SetEventConfig("APP_CRASH", nullptr);
    OH_HiAppEvent_SetEventConfig(nullptr, nullptr);
    OH_HiAppEvent_DestroyConfig(nullptr);
    if (config != nullptr) {
        OH_HiAppEvent_DestroyConfig(config);
    }
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzConfigOps(fuzzData);
    FuzzConfigNullOps();
    return 0;
}
