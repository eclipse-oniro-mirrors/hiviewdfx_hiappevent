/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hiappevent_userinfo.h"

#include <mutex>
#include <string>

#include "app_event_store.h"
#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventUserInfo"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

namespace {
constexpr int DB_FAILED = -1;

std::mutex g_mutex;
}

UserInfo& UserInfo::GetInstance()
{
    static UserInfo userInfo;
    return userInfo;
}

UserInfo::UserInfo() : userIdVersion_(0), userPropertyVersion_(0)
{
    InitUserIds();
    InitUserProperties();
}

int UserInfo::SetUserId(const std::string& name, const std::string& value)
{
    HILOG_DEBUG(LOG_CORE, "start to set userId, name=%{public}s, value=%{public}s.", name.c_str(), value.c_str());

    std::string out;
    if (AppEventStore::GetInstance().QueryUserId(name, out) == DB_FAILED) {
        HILOG_WARN(LOG_CORE, "failed to insert user id, name=%{public}s.", name.c_str());
        return -1;
    }
    if (out.empty()) {
        if (AppEventStore::GetInstance().InsertUserId(name, value) == DB_FAILED) {
            HILOG_WARN(LOG_CORE, "failed to insert user id, name=%{public}s.", name.c_str());
            return -1;
        }
    } else {
        if (AppEventStore::GetInstance().UpdateUserId(name, value) == DB_FAILED) {
            HILOG_WARN(LOG_CORE, "failed to update user id, name=%{public}s.", name.c_str());
            return -1;
        }
    }
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userIds_[name] = value;
    userIdVersion_++;

    return 0;
}

int UserInfo::GetUserId(const std::string& name, std::string& out)
{
    HILOG_DEBUG(LOG_CORE, "start to get userId, name=%{public}s.", name.c_str());

    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (userIds_.find(name) == userIds_.end()) {
        HILOG_INFO(LOG_CORE, "no userid, name=%{public}s.", name.c_str());
        return 0;
    }
    out = userIds_.at(name);

    return 0;
}

int UserInfo::RemoveUserId(const std::string& name)
{
    HILOG_DEBUG(LOG_CORE, "start to remove userId, name=%{public}s.", name.c_str());

    if (AppEventStore::GetInstance().DeleteUserId(name) == DB_FAILED) {
        HILOG_WARN(LOG_CORE, "failed to remove userid, name=%{public}s.", name.c_str());
        return -1;
    }
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (userIds_.find(name) != userIds_.end()) {
        userIds_.erase(name);
        userIdVersion_++;
    }

    return 0;
}

int UserInfo::SetUserProperty(const std::string& name, const std::string& value)
{
    HILOG_DEBUG(LOG_CORE, "start to set userProperty, name=%{public}s, value=%{public}s.", name.c_str(), value.c_str());

    std::string out;
    if (AppEventStore::GetInstance().QueryUserProperty(name, out) == DB_FAILED) {
        HILOG_WARN(LOG_CORE, "failed to insert user property, name=%{public}s.", name.c_str());
        return -1;
    }
    if (out.empty()) {
        if (AppEventStore::GetInstance().InsertUserProperty(name, value) == DB_FAILED) {
            HILOG_WARN(LOG_CORE, "failed to insert user property, name=%{public}s.", name.c_str());
            return -1;
        }
    } else {
        if (AppEventStore::GetInstance().UpdateUserProperty(name, value) == DB_FAILED) {
            HILOG_WARN(LOG_CORE, "failed to update user property, name=%{public}s.", name.c_str());
            return -1;
        }
    }
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userProperties_[name] = value;
    userPropertyVersion_++;

    return 0;
}

int UserInfo::GetUserProperty(const std::string& name, std::string& out)
{
    HILOG_DEBUG(LOG_CORE, "start to get userProperty, name=%{public}s.", name.c_str());

    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (userProperties_.find(name) == userProperties_.end()) {
        HILOG_INFO(LOG_CORE, "no user property, name=%{public}s.", name.c_str());
        return 0;
    }
    out = userProperties_.at(name);

    return 0;
}

int UserInfo::RemoveUserProperty(const std::string& name)
{
    HILOG_DEBUG(LOG_CORE, "start to remove userProperty, name=%{public}s.", name.c_str());

    if (AppEventStore::GetInstance().DeleteUserProperty(name) == DB_FAILED) {
        HILOG_WARN(LOG_CORE, "failed to remove user property, name=%{public}s.", name.c_str());
        return -1;
    }
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (userProperties_.find(name) != userProperties_.end()) {
        userProperties_.erase(name);
        userPropertyVersion_++;
    }

    return 0;
}

void UserInfo::InitUserIds()
{
    userIds_.clear();
    if (AppEventStore::GetInstance().QueryUserIds(userIds_) == DB_FAILED) {
        HILOG_WARN(LOG_CORE, "failed to get user ids.");
        return;
    }
}

void UserInfo::InitUserProperties()
{
    userProperties_.clear();
    if (AppEventStore::GetInstance().QueryUserProperties(userProperties_) == DB_FAILED) {
        HILOG_WARN(LOG_CORE, "failed to get user properties.");
        return;
    }
}

std::vector<HiAppEvent::UserId> UserInfo::GetUserIds()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    std::vector<HiAppEvent::UserId> userIds;
    for (auto it = userIds_.begin(); it != userIds_.end(); it++) {
        HiAppEvent::UserId userId;
        userId.name = it->first;
        userId.value = it->second;
        userIds.emplace_back(userId);
    }
    return userIds;
}

std::vector<HiAppEvent::UserProperty> UserInfo::GetUserProperties()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    std::vector<HiAppEvent::UserProperty> userProperties;
    for (auto it = userProperties_.begin(); it != userProperties_.end(); it++) {
        HiAppEvent::UserProperty userProperty;
        userProperty.name = it->first;
        userProperty.value = it->second;
        userProperties.emplace_back(userProperty);
    }
    return userProperties;
}

int64_t UserInfo::GetUserIdVersion()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return userIdVersion_;
}

int64_t UserInfo::GetUserPropertyVersion()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return userPropertyVersion_;
}

void UserInfo::ClearData()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userIdVersion_ = 0;
    userPropertyVersion_ = 0;
    userIds_.clear();
    userProperties_.clear();
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
