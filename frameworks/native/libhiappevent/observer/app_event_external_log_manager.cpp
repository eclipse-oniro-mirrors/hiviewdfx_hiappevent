/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "app_event_external_log_manager.h"

#include <cinttypes>
#include <cstddef>
#include <sys/stat.h>
#include <unordered_map>

#include "file_util.h"
#include "hiappevent_common.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07
#undef LOG_TAG
#define LOG_TAG "ExternalLogManager"

namespace OHOS {
namespace HiviewDFX {
namespace {
std::unordered_map<std::string, uint64_t> externalLogSize = {
    { "/data/storage/el2/log/hiappevent", 4 * 1024 * 1024 },
    { "/data/storage/el2/log/resourcelimit", 1500 * 1024 * 1024 },
};
constexpr size_t MIN_TIMESTAMP_LEN = 10;
constexpr size_t MAX_TIMESTAMP_LEN = 13;
}

AppEventExternalLogManager& AppEventExternalLogManager::GetInstance()
{
    static AppEventExternalLogManager instance;
    return instance;
}

bool AppEventExternalLogManager::RegisterCallback(std::shared_ptr<ExternalLogManagerCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback_ != nullptr) {
        return false;
    }
    callback_ = callback;
    return true;
}

bool AppEventExternalLogManager::IsRegistered()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return callback_ != nullptr;
}

void AppEventExternalLogManager::CheckCapacity()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callback_ == nullptr) {
        return;
    }

    for (const auto& [dir, thresholdSize] : externalLogSize) {
        if (!FileUtil::IsDirectory(dir)) {
            continue;
        }
        uint64_t curSize = FileUtil::GetDirSize(dir);
        if (curSize < thresholdSize) {
            continue;
        }

        std::vector<ExternalLogWrapperInfo> logInfos;
        ScanLogFiles(dir, logInfos);
        if (!logInfos.empty()) {
            callback_->OnCapacityReached(logInfos);
        }
    }
}

void AppEventExternalLogManager::ScanLogFiles(const std::string& dir,
    std::vector<ExternalLogWrapperInfo>& logInfos)
{
    std::vector<std::string> files;
    FileUtil::GetDirFiles(dir, files);
    for (const auto& file : files) {
        logInfos.emplace_back(ParseLogFileInfo(file));
    }
}

bool IsTimestampSegment(const std::string& fileName, size_t start, size_t end)
{
    for (size_t i = start; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(fileName[i]))) {
            return false;
        }
    }
    return true;
}

ExternalLogWrapperInfo AppEventExternalLogManager::ParseLogFileInfo(const std::string& filePath) const
{
    ExternalLogWrapperInfo info;
    info.filePath = filePath;

    struct stat statBuf = {};
    if (stat(filePath.c_str(), &statBuf) == 0) {
        info.sizeInKb = static_cast<int64_t>(statBuf.st_size / 1024); // 1KB = 1024B
    }

    // Parse event name and timestamp from filename: EVENTNAME_TIMESTAMP_PID.ext
    std::string fileName = filePath.substr(filePath.rfind('/') + 1);
    size_t start = 0;
    size_t end = 0;
    while ((end = fileName.find('_', start)) != std::string::npos) {
        size_t nextUnderscore = fileName.find('_', end + 1);
        if (nextUnderscore == std::string::npos) {
            break;
        }
        size_t len = nextUnderscore - (end + 1);
        if (len >= MIN_TIMESTAMP_LEN && len <= MAX_TIMESTAMP_LEN &&
            IsTimestampSegment(fileName, end + 1, nextUnderscore)) {
            info.sysEvent = fileName.substr(0, end);
            info.generationTime = std::stoll(fileName.substr(end + 1, len));
            break;
        }
        start = end + 1;
    }
    return info;
}

} // namespace HiviewDFX
} // namespace OHOS