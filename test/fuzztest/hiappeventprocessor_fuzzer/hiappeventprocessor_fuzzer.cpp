#define FUZZ_PROJECT_NAME "hiappeventprocessor_fuzzer"

#include "hiappeventprocessor_fuzzer.h"
#include <cstring>
#include "hiappevent/hiappevent.h"

namespace {
constexpr int MAX_STR_LEN = 64;
constexpr int LOCAL_ARR_LEN = 2;
constexpr int PERIOD_REPORT_DEFAULT = 1;
constexpr int BATCH_REPORT_DEFAULT = 1;
constexpr int CONFIG_ID_DEFAULT = 1;
constexpr int64_t INVALID_PROCESSOR_SEQ = -1;
constexpr int64_t ZERO_PROCESSOR_SEQ = 0;

struct FuzzData {
    char processorName[MAX_STR_LEN];
    char appId[MAX_STR_LEN];
    char routeInfo[MAX_STR_LEN];
    char domain[MAX_STR_LEN];
    char eventName[MAX_STR_LEN];
    char customKey[MAX_STR_LEN];
    char customValue[MAX_STR_LEN];
    char configName[MAX_STR_LEN];
    int periodReport;
    int batchReport;
    int configId;
    bool onStartReport;
    bool onBackgroundReport;
    bool isRealTime;
    int userIdSize;
    int userPropSize;
};

void FuzzProcessorOps(const FuzzData& data)
{
    HiAppEvent_Processor* processor = OH_HiAppEvent_CreateProcessor(data.processorName);
    if (processor == nullptr) {
        return;
    }
    OH_HiAppEvent_SetReportRoute(processor, data.appId, data.routeInfo);
    OH_HiAppEvent_SetReportPolicy(processor, data.periodReport, data.batchReport,
        data.onStartReport, data.onBackgroundReport);
    OH_HiAppEvent_SetReportEvent(processor, data.domain, data.eventName, data.isRealTime);
    OH_HiAppEvent_SetCustomConfig(processor, data.customKey, data.customValue);
    OH_HiAppEvent_SetConfigId(processor, data.configId);
    OH_HiAppEvent_SetConfigName(processor, data.configName);
    const char* userIdNames[] = {data.appId, data.routeInfo};
    int userIdSize = data.userIdSize % (LOCAL_ARR_LEN + 1);
    OH_HiAppEvent_SetReportUserId(processor, userIdNames, userIdSize);
    const char* userPropNames[] = {data.customKey, data.customValue};
    int userPropSize = data.userPropSize % (LOCAL_ARR_LEN + 1);
    OH_HiAppEvent_SetReportUserProperty(processor, userPropNames, userPropSize);
    int64_t processorId = OH_HiAppEvent_AddProcessor(processor);
    OH_HiAppEvent_RemoveProcessor(processorId);
    OH_HiAppEvent_DestroyProcessor(processor);
}

void FuzzProcessorNullOps()
{
    OH_HiAppEvent_CreateProcessor(nullptr);
    OH_HiAppEvent_SetReportRoute(nullptr, "appId", "routeInfo");
    OH_HiAppEvent_SetReportPolicy(nullptr, PERIOD_REPORT_DEFAULT, BATCH_REPORT_DEFAULT, true, false);
    OH_HiAppEvent_SetReportEvent(nullptr, "domain", "name", true);
    OH_HiAppEvent_SetCustomConfig(nullptr, "key", "value");
    OH_HiAppEvent_SetConfigId(nullptr, CONFIG_ID_DEFAULT);
    OH_HiAppEvent_SetConfigName(nullptr, "configName");
    OH_HiAppEvent_SetReportUserId(nullptr, nullptr, 0);
    OH_HiAppEvent_SetReportUserProperty(nullptr, nullptr, 0);
    OH_HiAppEvent_AddProcessor(nullptr);
    OH_HiAppEvent_DestroyProcessor(nullptr);
    OH_HiAppEvent_RemoveProcessor(INVALID_PROCESSOR_SEQ);
    OH_HiAppEvent_RemoveProcessor(ZERO_PROCESSOR_SEQ);
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzProcessorOps(fuzzData);
    FuzzProcessorNullOps();
    return 0;
}
