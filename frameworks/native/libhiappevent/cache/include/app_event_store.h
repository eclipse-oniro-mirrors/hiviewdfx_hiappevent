/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <mutex>
#include <string>
#include <vector>

#include "app_event_dao.h"
#include "app_event_mapping_dao.h"
#include "app_event_observer_dao.h"
#include "custom_event_param_dao.h"
#include "user_id_dao.h"
#include "user_property_dao.h"
#include "rdb_store.h"
#include "singleton.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;

class AppEventStore : public DelayedRefSingleton<AppEventStore>  {
public:
    AppEventStore();
    ~AppEventStore();
    int InitDbStore();
    int DestroyDbStore();
    int64_t InsertEvent(std::shared_ptr<AppEventPack> event);
    int64_t InsertObserver(const std::string& observer, int64_t hashCode = 0);
    int64_t InsertEventMapping(int64_t eventSeq, int64_t observerSeq);
    int64_t InsertUserId(const std::string& name, const std::string& value);
    int64_t InsertUserProperty(const std::string& name, const std::string& value);
    int64_t InsertCustomEventParams(std::shared_ptr<AppEventPack> event);
    int64_t UpdateUserId(const std::string& name, const std::string& value);
    int64_t UpdateUserProperty(const std::string& name, const std::string& value);
    int TakeEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t eventSize = 0);
    int QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t eventSize = 0);
    int64_t QueryObserverSeq(const std::string& observer, int64_t hashCode = 0);
    int QueryObserverSeqs(const std::string& observer, std::vector<int64_t>& observerSeqs);
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
    int DeleteCustomEventParams(const std::string& runningId);

private:
    bool InitDbStoreDir();

private:
    std::shared_ptr<NativeRdb::RdbStore> dbStore_;
    std::shared_ptr<AppEventDao> appEventDao_;
    std::shared_ptr<AppEventObserverDao> appEventObserverDao_;
    std::shared_ptr<AppEventMappingDao> appEventMappingDao_;
    std::shared_ptr<UserIdDao> userIdDao_;
    std::shared_ptr<UserPropertyDao> userPropertyDao_;
    std::shared_ptr<CustomEventParamDao> customEventParamDao_;
    std::string dirPath_;
    std::mutex dbMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_CACHE_APP_EVENT_STORE_H
