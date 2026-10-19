#define FUZZ_PROJECT_NAME "hiappeventinnerprocessor_fuzzer"

#include "hiappeventinnerprocessor_fuzzer.h"
#include <cstring>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "app_event_processor.h"
#include "app_event_processor_mgr.h"
#include "base_type.h"

namespace {
constexpr int MAX_STR_LEN = 64;
constexpr int64_t INVALID_PROCESSOR_SEQ = -1;
constexpr int64_t ZERO_PROCESSOR_SEQ = 0;

class FuzzProcessor : public OHOS::HiviewDFX::HiAppEvent::AppEventProcessor {
public:
    int OnReport(int64_t processorSeq,
                 const std::vector<OHOS::HiviewDFX::HiAppEvent::UserId>& userIds,
                 const std::vector<OHOS::HiviewDFX::HiAppEvent::UserProperty>& userProperties,
                 const std::vector<OHOS::HiviewDFX::HiAppEvent::AppEventInfo>& events) override
    {
        return 0;
    }
    int ValidateUserId(const OHOS::HiviewDFX::HiAppEvent::UserId& userId) override { return 0; }
    int ValidateUserProperty(const OHOS::HiviewDFX::HiAppEvent::UserProperty& userProperty) override { return 0; }
    int ValidateEvent(const OHOS::HiviewDFX::HiAppEvent::AppEventInfo& event) override { return 0; }
};

struct FuzzData {
    char processorName[MAX_STR_LEN];
    char domain[MAX_STR_LEN];
    char eventName[MAX_STR_LEN];
    char appId[MAX_STR_LEN];
    char routeInfo[MAX_STR_LEN];
    char configName[MAX_STR_LEN];
    char userIdName[MAX_STR_LEN];
    char userPropName[MAX_STR_LEN];
    char customKey[MAX_STR_LEN];
    char customValue[MAX_STR_LEN];
    int row;
    int size;
    int timeout;
    int configId;
    bool onStartup;
    bool onBackground;
    bool isRealTime;
    bool debugMode;
};

void FuzzProcessorMgrOps(const FuzzData& data)
{
    using namespace OHOS::HiviewDFX::HiAppEvent;
    ReportConfig config;
    config.name = data.processorName;
    config.debugMode = data.debugMode;
    config.routeInfo = data.routeInfo;
    config.appId = data.appId;
    config.triggerCond.row = data.row;
    config.triggerCond.size = data.size;
    config.triggerCond.timeout = data.timeout;
    config.triggerCond.onStartup = data.onStartup;
    config.triggerCond.onBackground = data.onBackground;
    config.configId = data.configId;
    config.configName = data.configName;
    config.userIdNames.insert(data.userIdName);
    config.userPropertyNames.insert(data.userPropName);
    config.customConfigs[data.customKey] = data.customValue;

    EventConfig eventConfig;
    eventConfig.domain = data.domain;
    eventConfig.name = data.eventName;
    eventConfig.isRealTime = data.isRealTime;
    config.eventConfigs.push_back(eventConfig);

    int64_t processorSeq = AppEventProcessorMgr::AddProcessor(config);
    if (processorSeq > 0) {
        ReportConfig getConfig;
        AppEventProcessorMgr::GetProcessorConfig(processorSeq, getConfig);
        AppEventProcessorMgr::SetProcessorConfig(processorSeq, config);
        std::vector<int64_t> seqs;
        AppEventProcessorMgr::GetProcessorSeqs(data.processorName, seqs);
        AppEventProcessorMgr::RemoveProcessor(processorSeq);
    }
    AppEventProcessorMgr::RemoveProcessor(INVALID_PROCESSOR_SEQ);
    AppEventProcessorMgr::RemoveProcessor(ZERO_PROCESSOR_SEQ);
}

void FuzzRegisterProcessor(const FuzzData& data)
{
    using namespace OHOS::HiviewDFX::HiAppEvent;
    auto processor = std::make_shared<FuzzProcessor>();
    AppEventProcessorMgr::RegisterProcessor(data.processorName, processor);
    AppEventProcessorMgr::RegisterProcessor(data.processorName, processor);
    AppEventProcessorMgr::UnregisterProcessor(data.processorName);
    AppEventProcessorMgr::UnregisterProcessor(data.processorName);
    AppEventProcessorMgr::RegisterProcessor("", nullptr);
    AppEventProcessorMgr::UnregisterProcessor("");
}

void FuzzProcessorMgrAsync(const FuzzData& data)
{
    using namespace OHOS::HiviewDFX::HiAppEvent;
    ReportConfig config;
    config.name = data.processorName;
    AppEventProcessorMgr::AddProcessorAsync(config, [](int64_t seq) {});
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(FuzzData)) {
        return 0;
    }
    FuzzData fuzzData = *reinterpret_cast<const FuzzData*>(data);
    FuzzProcessorMgrOps(fuzzData);
    FuzzRegisterProcessor(fuzzData);
    FuzzProcessorMgrAsync(fuzzData);
    return 0;
}
