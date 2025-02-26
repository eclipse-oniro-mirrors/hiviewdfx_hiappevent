/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "hiappevent_config.h"

#include <algorithm>
#include <cinttypes>
#include <cstdlib>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>

#include "app_event_observer_mgr.h"
#include "application_context.h"
#include "context.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "iservice_registry.h"
#include "storage_manager_proxy.h"
#include "system_ability_definition.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Config"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DISABLE = "disable";
const std::string MAX_STORAGE = "max_storage";
const std::string DEFAULT_STORAGE_DIR = "";
const std::string APP_EVENT_DIR = "/hiappevent/";
constexpr uint64_t STORAGE_UNIT_KB = 1024;
constexpr uint64_t STORAGE_UNIT_MB = STORAGE_UNIT_KB * 1024;
constexpr uint64_t STORAGE_UNIT_GB = STORAGE_UNIT_MB * 1024;
constexpr uint64_t STORAGE_UNIT_TB = STORAGE_UNIT_GB * 1024;
constexpr int DECIMAL_UNIT = 10;
constexpr int64_t FREE_SIZE_LIMIT = STORAGE_UNIT_MB * 300;

std::mutex g_mutex;

std::string TransUpperToUnderscoreAndLower(const std::string& str)
{
    if (str.empty()) {
        return "";
    }

    std::stringstream ss;
    for (size_t i = 0; i < str.size(); i++) {
        char tmp = str[i];
        if (tmp < 'A' || tmp > 'Z') {
            ss << tmp;
            continue;
        }
        if (i != 0) { // prevent string from starting with an underscore
            ss << "_";
        }
        tmp += 32; // 32 means upper case to lower case
        ss << tmp;
    }

    return ss.str();
}

sptr<OHOS::StorageManager::IStorageManager> GetStorageMgr()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        HILOG_WARN(LOG_CORE, "systemAbilityManager is null.");
        return nullptr;
    }
    sptr<IRemoteObject> storageMgrSa = systemAbilityManager->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (storageMgrSa == nullptr) {
        HILOG_WARN(LOG_CORE, "storageMgrSa is null.");
        return nullptr;
    }
    return iface_cast<OHOS::StorageManager::IStorageManager>(storageMgrSa);
}
}

HiAppEventConfig& HiAppEventConfig::GetInstance()
{
    static HiAppEventConfig instance;
    return instance;
}

bool HiAppEventConfig::SetConfigurationItem(std::string name, std::string value)
{
    // trans uppercase to underscore and lowercase
    name = TransUpperToUnderscoreAndLower(name);
    HILOG_DEBUG(LOG_CORE, "start to configure.");

    if (name == "") {
        HILOG_ERROR(LOG_CORE, "item name can not be empty.");
        return false;
    }
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if (value == "") {
        HILOG_ERROR(LOG_CORE, "item value can not be empty.");
        return false;
    }
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    if (name == DISABLE) {
        return SetDisableItem(value);
    } else if (name == MAX_STORAGE) {
        return SetMaxStorageSizeItem(value);
    } else {
        HILOG_ERROR(LOG_CORE, "unrecognized configuration item name.");
        return false;
    }
}

bool HiAppEventConfig::SetDisableItem(const std::string& value)
{
    if (value == "true") {
        SetDisable(true);
    } else if (value == "false") {
        SetDisable(false);
    } else {
        HILOG_ERROR(LOG_CORE, "invalid bool value=%{public}s of the application dotting switch.", value.c_str());
        return false;
    }
    return true;
}

bool HiAppEventConfig::SetMaxStorageSizeItem(const std::string& value)
{
    if (!std::regex_match(value, std::regex("[0-9]+[k|m|g|t]?[b]?"))) {
        HILOG_ERROR(LOG_CORE, "invalid value=%{public}s of the event file dir storage quota size.", value.c_str());
        return false;
    }

    char* numEndIndex = nullptr;
    uint64_t numValue = std::strtoull(value.c_str(), &numEndIndex, DECIMAL_UNIT);
    if (*numEndIndex == '\0') {
        SetMaxStorageSize(numValue);
        return true;
    }

    uint32_t unitLen = std::strlen(numEndIndex);
    auto len = value.length();
    char unitChr = value[len - unitLen];
    uint64_t maxStoSize = 0;
    switch (unitChr) {
        case 'b':
            maxStoSize = numValue;
            break;
        case 'k':
            maxStoSize = numValue * STORAGE_UNIT_KB;
            break;
        case 'm':
            maxStoSize = numValue * STORAGE_UNIT_MB;
            break;
        case 'g':
            maxStoSize = numValue * STORAGE_UNIT_GB;
            break;
        case 't':
            maxStoSize = numValue * STORAGE_UNIT_TB;
            break;
        default:
            HILOG_ERROR(LOG_CORE, "invalid storage unit value=%{public}c.", unitChr);
            return false;
    }

    SetMaxStorageSize(maxStoSize);
    return true;
}

void HiAppEventConfig::SetDisable(bool disable)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    disable_ = disable;
}

void HiAppEventConfig::SetMaxStorageSize(uint64_t size)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    maxStorageSize_ = size;
}

void HiAppEventConfig::SetStorageDir(const std::string& dir)
{
    storageDir_ = dir;
}

bool HiAppEventConfig::GetDisable()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return disable_;
}

uint64_t HiAppEventConfig::GetMaxStorageSize()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return maxStorageSize_;
}

std::string HiAppEventConfig::GetStorageDir()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (!storageDir_.empty()) {
        return storageDir_;
    }
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "Context is null.");
        return DEFAULT_STORAGE_DIR;
    }
    if (context->GetFilesDir().empty()) {
        HILOG_ERROR(LOG_CORE, "The files dir obtained from context is empty.");
        return DEFAULT_STORAGE_DIR;
    }
    std::string dir = context->GetFilesDir() + APP_EVENT_DIR;
    SetStorageDir(dir);
    return storageDir_;
}

std::string HiAppEventConfig::GetRunningId()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (!runningId_.empty()) {
        return runningId_;
    }
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "Context is null.");
        return "";
    }
    runningId_ = context->GetAppRunningUniqueId();
    if (runningId_.empty()) {
        HILOG_ERROR(LOG_CORE, "The running id from context is empty.");
    }
    return runningId_;
}

bool HiAppEventConfig::IsFreeSizeOverLimit()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (!isInitFreeSize_) {
        isInitFreeSize_ = true;
        auto storageMgr = GetStorageMgr();
        int64_t freeSize = -1;
        if (storageMgr == nullptr || storageMgr->GetFreeSize(freeSize) != 0) {
            HILOG_WARN(LOG_CORE, "Failed to get free size.");
            return false;
        }
        HILOG_INFO(LOG_CORE, "get free size=%{public}" PRId64, freeSize);
        freeSize_ = freeSize;
    }
    return freeSize_ >= 0 && freeSize_ < FREE_SIZE_LIMIT;
}

void HiAppEventConfig::RefreshFreeSize()
{
    auto storageMgr = GetStorageMgr();
    int64_t freeSize = -1;
    if (storageMgr == nullptr || storageMgr->GetFreeSize(freeSize) != 0) {
        HILOG_WARN(LOG_CORE, "Failed to refresh free size.");
        return;
    }
    HILOG_INFO(LOG_CORE, "refresh free size=%{public}" PRId64, freeSize);
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    freeSize_ = freeSize;
}
} // namespace HiviewDFX
} // namespace OHOS
