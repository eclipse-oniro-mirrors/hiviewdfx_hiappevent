/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "app_event_log_cleaner.h"

#include <algorithm>
#include <cerrno>
#include <cmath>

#include "file_util.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventLogCleaner"

namespace OHOS {
namespace HiviewDFX {
uint64_t AppEventLogCleaner::GetFilesSize()
{
    return FileUtil::GetDirSize(path_);
}

uint64_t AppEventLogCleaner::ClearSpace(uint64_t curSize, uint64_t maxSize)
{
    HILOG_INFO(LOG_CORE, "start to clear the space occupied by log files");
    std::vector<std::string> files;
    FileUtil::GetDirFiles(path_, files);

    // delete log files one by one based on the timestamp order of the file name
    sort(files.begin(), files.end());

    uint64_t nowSize = curSize;
    while (!files.empty() && nowSize > maxSize) {
        std::string delFile = files[0];
        files.erase(files.begin());
        if (!FileUtil::IsFileExists(delFile)) {
            HILOG_ERROR(LOG_CORE, "failed to access the log file, errno=%{public}d", errno);
            continue;
        }
        uint64_t delFileSize = FileUtil::GetFileSize(delFile);
        if (!FileUtil::RemoveFile(delFile)) {
            HILOG_ERROR(LOG_CORE, "failed to remove the log file, errno=%{public}d", errno);
            continue;
        }
        nowSize -= std::min(delFileSize, nowSize);
    }
    return nowSize;
}

void AppEventLogCleaner::ClearData()
{
    HILOG_INFO(LOG_CORE, "start to clear the log data");
    std::vector<std::string> files;
    FileUtil::GetDirFiles(path_, files);
    for (const auto& file : files) {
        if (!FileUtil::RemoveFile(file)) {
            HILOG_WARN(LOG_CORE, "failed to remove the log file=%{public}s", file.c_str());
        } else {
            HILOG_INFO(LOG_CORE, "succ to remove the log file=%{public}s", file.c_str());
        }
    }
}
} // namespace HiviewDFX
} // namespace OHOS
