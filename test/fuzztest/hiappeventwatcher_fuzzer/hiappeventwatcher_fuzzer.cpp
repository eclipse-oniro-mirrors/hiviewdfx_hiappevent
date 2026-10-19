#define FUZZ_PROJECT_NAME "hiappeventwatcher_fuzzer"

#include "hiappeventwatcher_fuzzer.h"
#include <cstring>
#include "hiappevent/hiappevent.h"

namespace {
constexpr int MAX_STR_LEN = 64;
constexpr int LOCAL_ARR_LEN = 2;
constexpr uint8_t ALL_EVENT_TYPES = 0xff;
constexpr int TRIGGER_ROW_DEFAULT = 1;
constexpr int TRIGGER_SIZE_DEFAULT = 1;
constexpr int TRIGGER_TIMEOUT_DEFAULT = 1;
constexpr int EVENT_NUM_DEFAULT = 1;

void FuzzOnTrigger(int row, int size) {}

void FuzzOnReceive(const char* domain, const struct HiAppEvent_AppEventGroup* appEventGroups, uint32_t groupLen) {}

void FuzzOnTake(const char* const *events, uint32_t eventLen) {}

struct FuzzData {
    char watcherName[MAX_STR_LEN];
    char domain[MAX_STR_LEN];
    char eventNames[2][MAX_STR_LEN];
    int row;
    int size;
    int timeOut;
    uint8_t eventTypes;
    int namesLen;
    uint32_t eventNum;
};

void FuzzWatcherOps(const FuzzData& data)
{
    HiAppEvent_Watcher* watcher = OH_HiAppEvent_CreateWatcher(data.watcherName);
    if (watcher == nullptr) {
        return;
    }
    OH_HiAppEvent_SetTriggerCondition(watcher, data.row, data.size, data.timeOut);
    const char* names[] = {data.eventNames[0], data.eventNames[1]};
    int namesLen = data.namesLen % (LOCAL_ARR_LEN + 1);
    OH_HiAppEvent_SetAppEventFilter(watcher, data.domain, data.eventTypes, names, namesLen);
    OH_HiAppEvent_SetWatcherOnTrigger(watcher, FuzzOnTrigger);
    OH_HiAppEvent_SetWatcherOnReceive(watcher, FuzzOnReceive);
    OH_HiAppEvent_AddWatcher(watcher);
    OH_HiAppEvent_TakeWatcherData(watcher, data.eventNum, FuzzOnTake);
    OH_HiAppEvent_RemoveWatcher(watcher);
    OH_HiAppEvent_DestroyWatcher(watcher);
}

void FuzzWatcherNullOps()
{
    OH_HiAppEvent_CreateWatcher(nullptr);
    OH_HiAppEvent_SetTriggerCondition(nullptr, TRIGGER_ROW_DEFAULT, TRIGGER_SIZE_DEFAULT, TRIGGER_TIMEOUT_DEFAULT);
    OH_HiAppEvent_SetAppEventFilter(nullptr, "domain", ALL_EVENT_TYPES, nullptr, 0);
    OH_HiAppEvent_SetWatcherOnTrigger(nullptr, nullptr);
    OH_HiAppEvent_SetWatcherOnReceive(nullptr, nullptr);
    OH_HiAppEvent_AddWatcher(nullptr);
    OH_HiAppEvent_TakeWatcherData(nullptr, EVENT_NUM_DEFAULT, nullptr);
    OH_HiAppEvent_RemoveWatcher(nullptr);
    OH_HiAppEvent_DestroyWatcher(nullptr);
}

void FuzzWatcherWithNullFilter(const FuzzData& data)
{
    HiAppEvent_Watcher* watcher = OH_HiAppEvent_CreateWatcher(data.watcherName);
    if (watcher == nullptr) {
        return;
    }
    OH_HiAppEvent_SetAppEventFilter(watcher, nullptr, ALL_EVENT_TYPES, nullptr, 0);
    OH_HiAppEvent_SetAppEventFilter(watcher, data.domain, 0, nullptr, 0);
    OH_HiAppEvent_SetWatcherOnTrigger(watcher, nullptr);
    OH_HiAppEvent_SetWatcherOnReceive(watcher, nullptr);
    OH_HiAppEvent_AddWatcher(watcher);
    OH_HiAppEvent_RemoveWatcher(watcher);
    OH_HiAppEvent_DestroyWatcher(watcher);
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzWatcherOps(fuzzData);
    FuzzWatcherNullOps();
    FuzzWatcherWithNullFilter(fuzzData);
    return 0;
}
