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

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, 0xD002D07, "HiAppEvent_LogCleaner" };
}
uint64_t AppEventLogCleaner::GetFilesSize()
{
    return FileUtil::GetDirSize(path_);
}

uint64_t AppEventLogCleaner::ClearSpace(uint64_t curSize, uint64_t maxSize)
{
    HiLog::Info(LABEL, "start to clear the space occupied by log files");
    std::vector<std::string> files;
    FileUtil::GetDirFiles(path_, files);

    // delete log files one by one based on the timestamp order of the file name
    sort(files.begin(), files.end());

    uint64_t nowSize = curSize;
    while (!files.empty() && nowSize > maxSize) {
        std::string delFile = files[0];
        files.erase(files.begin());
        if (!FileUtil::IsFileExists(delFile)) {
            HiLog::Error(LABEL, "failed to access the log file, errno=%{public}d", errno);
            continue;
        }
        uint64_t delFileSize = FileUtil::GetFileSize(delFile);
        if (!FileUtil::RemoveFile(delFile)) {
            HiLog::Error(LABEL, "failed to remove the log file, errno=%{public}d", errno);
            continue;
        }
        nowSize -= std::min(delFileSize, nowSize);
    }
    return nowSize;
}

void AppEventLogCleaner::ClearData()
{
    HiLog::Info(LABEL, "start to clear the log data");
    if (FileUtil::IsFileExists(path_) && !FileUtil::ForceRemoveDirectory(path_)) {
        HiLog::Error(LABEL, "failed to clear the log data, errno=%{public}d", errno);
    }
}
} // namespace HiviewDFX
} // namespace OHOS
