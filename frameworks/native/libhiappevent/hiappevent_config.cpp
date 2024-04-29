/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <mutex>
#include <regex>
#include <sstream>
#include <string>

#include "app_event_observer_mgr.h"
#include "application_context.h"
#include "context.h"
#include "hiappevent_base.h"
#include "hiappevent_read.h"
#include "hilog/log.h"
#include "module_loader.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventConfig"

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
    HILOG_DEBUG(LOG_CORE, "start to configure, name=%{public}s, value=%{public}s.", name.c_str(), value.c_str());

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

    auto len = value.length();
    std::string::size_type numEndIndex = 0;
    uint64_t numValue = std::stoull(value, &numEndIndex, DECIMAL_UNIT);
    if (numEndIndex == len) {
        SetMaxStorageSize(numValue);
        return true;
    }

    uint32_t unitLen = (numEndIndex == (len - 1)) ? 1 : 2; // 1 2, means the length of the storage unit
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
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        this->disable = disable;
    }
}

void HiAppEventConfig::SetMaxStorageSize(uint64_t size)
{
    {
        std::lock_guard<std::mutex> lockGuard(g_mutex);
        this->maxStorageSize = size;
    }
}

void HiAppEventConfig::SetStorageDir(const std::string& dir)
{
    this->storageDir = dir;
    LogAssistant::Instance().UpdateHiAppEventLogDir(dir);
}

bool HiAppEventConfig::GetDisable()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return this->disable;
}

uint64_t HiAppEventConfig::GetMaxStorageSize()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return this->maxStorageSize;
}

std::string HiAppEventConfig::GetStorageDir()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (!this->storageDir.empty()) {
        return this->storageDir;
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
    return this->storageDir;
}
} // namespace HiviewDFX
} // namespace OHOS
