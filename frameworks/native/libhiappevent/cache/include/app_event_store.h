/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H

#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "app_event_dao.h"
#include "app_event_mapping_dao.h"
#include "app_event_observer_dao.h"
#include "custom_event_param_dao.h"
#include "nocopyable.h"
#include "rdb_store.h"
#include "singleton.h"
#include "user_id_dao.h"
#include "user_property_dao.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

class AppEventStore : public NoCopyable {
public:
    static AppEventStore& GetInstance();

    int InitDbStore();
    int DestroyDbStore();
    int64_t InsertEvent(std::shared_ptr<AppEventPack> event);
    int64_t InsertObserver(const AppEventCacheCommon::Observer& observer);
    int InsertEventMapping(const std::vector<AppEventCacheCommon::EventObserverInfo>& eventObservers);
    int InsertUserId(const std::string& name, const std::string& value);
    int InsertUserProperty(const std::string& name, const std::string& value);
    int InsertCustomEventParams(std::shared_ptr<AppEventPack> event);
    int UpdateUserId(const std::string& name, const std::string& value);
    int UpdateUserProperty(const std::string& name, const std::string& value);
    int UpdateObserver(int64_t seq, const std::string& filters);
    int TakeEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t eventSize = 0);
    int QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t eventSize = 0);
    int64_t QueryObserverSeq(const std::string& name, int64_t hashCode = 0);
    int64_t QueryObserverSeqAndFilters(const std::string& name, int64_t hashCode, std::string& filters);
    int QueryObserverSeqs(const std::string& name, std::vector<int64_t>& observerSeqs);
    int QueryWatchers(std::vector<AppEventCacheCommon::Observer>& observers);
    int QueryUserIds(std::unordered_map<std::string, std::string>& out);
    int QueryUserId(const std::string& name, std::string& out);
    int QueryUserProperties(std::unordered_map<std::string, std::string>& out);
    int QueryUserProperty(const std::string& name, std::string& out);
    int QueryCustomParamsAdd2EventPack(std::shared_ptr<AppEventPack> event);
    int DeleteObserver(int64_t observerSeq);
    int DeleteEventMapping(int64_t observerSeq = 0, const std::vector<int64_t>& eventSeqs = {});
    int DeleteUserId(const std::string& name = "");
    int DeleteUserProperty(const std::string& name = "");
    int DeleteEvent(int64_t eventSeq = 0);
    int DeleteCustomEventParams();
    int DeleteEvent(const std::vector<int64_t>& eventSeqs);
    int DeleteUnusedParamsExceptCurId(const std::string& curRunningId);
    int DeleteUnusedEventMapping();
    int DeleteHistoryEvent(int reservedNum, int reservedNumOs);
    bool DeleteData(int64_t observerSeq, const std::vector<int64_t>& eventSeqs);

private:
    AppEventStore();
    ~AppEventStore();
    bool InitDbStoreDir();
    void CheckAndRepairDbStore(int errCode);
    int ExecuteDbOperation(const std::function<int()>& func);
    int ExecuteReadOperation(const std::function<int()>& func, bool& isExecuted);
    int ExecuteWriteOperation(const std::function<int()>& func, const bool& isExecuted, int& OperationRes);

private:
    std::shared_ptr<NativeRdb::RdbStore> dbStore_;
    std::string dirPath_;
    std::shared_mutex dbMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H
