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
#include "file_util.h"

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace OHOS {
namespace HiviewDFX {
namespace FileUtil {
namespace {
const char PATH_DELIMITER = '/';
}
bool IsFileExists(const std::string& file)
{
    return access(file.c_str(), F_OK) == 0;
}

bool IsFile(const std::string& file)
{
    struct stat statBuf {};
    return lstat(file.c_str(), &statBuf) == 0 ? S_ISREG(statBuf.st_mode) : false;
}

bool IsDirectory(const std::string& dir)
{
    struct stat statBuf {};
    return lstat(dir.c_str(), &statBuf) == 0 ? S_ISDIR(statBuf.st_mode) : false;
}

bool RemoveFile(const std::string& file)
{
    return !IsFileExists(file) || (remove(file.c_str()) == 0);
}

bool RemoveDirectory(const std::string& dir)
{
    return !IsFileExists(dir) || (rmdir(dir.c_str()) == 0);
}

bool ForceCreateDirectory(const std::string& dir)
{
    std::string::size_type index = 0;
    do {
        std::string subPath;
        index = dir.find('/', index + 1); // (index + 1) means the next char traversed
        if (index == std::string::npos) {
            subPath = dir;
        } else {
            subPath = dir.substr(0, index);
        }

        if (!IsFileExists(subPath) && mkdir(subPath.c_str(), S_IRWXU) != 0) {
            return false;
        }
    } while (index != std::string::npos);
    return IsFileExists(dir);
}

bool ForceRemoveDirectory(const std::string& dir, bool isDeleteSelf)
{
    if (IsFile(dir)) {
        return RemoveFile(dir);
    } else if (IsDirectory(dir)) {
        DIR* dirPtr = opendir(dir.c_str());
        if (dirPtr == nullptr) {
            return false;
        }
        struct dirent* dirInfo = nullptr;
        while ((dirInfo = readdir(dirPtr)) != nullptr) {
            // do not process the special dir
            if (strcmp(dirInfo->d_name, ".") == 0 || strcmp(dirInfo->d_name, "..") == 0) {
                continue;
            }
            std::string filePath = GetFilePathByDir(dir, dirInfo->d_name);
            if (!ForceRemoveDirectory(filePath)) {
                closedir(dirPtr);
                return false;
            }
        }
        closedir(dirPtr);
        if (isDeleteSelf && !RemoveDirectory(dir)) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

void GetDirFiles(const std::string& dir, std::vector<std::string>& files)
{
    DIR* dirPtr = opendir(dir.c_str());
    if (dirPtr == nullptr) {
        return;
    }

    struct dirent* ent = nullptr;
    while ((ent = readdir(dirPtr)) != nullptr) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
            continue;
        }
        std::string filePath = GetFilePathByDir(dir, ent->d_name);
        if (IsFile(filePath)) { // do not process subdirectory
            files.push_back(filePath);
        }
    }
    closedir(dirPtr);
}

uint64_t GetDirSize(const std::string& dir)
{
    std::vector<std::string> files;
    GetDirFiles(dir, files);
    uint64_t totalSize = 0;
    struct stat statBuf {};
    for (auto& file : files) {
        if (stat(file.c_str(), &statBuf) == 0) {
            totalSize += static_cast<uint64_t>(statBuf.st_size);
        }
    }
    return totalSize;
}

uint64_t GetFileSize(const std::string& file)
{
    struct stat statBuf {};
    return stat(file.c_str(), &statBuf) == 0 ? static_cast<uint64_t>(statBuf.st_size) : 0;
}

bool SaveStringToFile(const std::string& file, const std::string& content, bool isTrunc)
{
    if (content.empty()) {
        return true;
    }

    std::ofstream os;
    if (isTrunc) {
        os.open(file.c_str(), std::ios::out | std::ios::trunc);
    } else {
        os.open(file.c_str(), std::ios::out | std::ios::app);
    }
    if (!os.is_open()) {
        return false;
    }

    os.write(content.c_str(), content.length());
    if (os.fail()) {
        os.close();
        return false;
    }
    os.close();
    return true;
}

std::string GetFilePathByDir(const std::string& dir, const std::string& fileName)
{
    if (dir.empty()) {
        return fileName;
    }
    std::string filePath = dir;
    if (filePath.back() != '/') {
        filePath.push_back(PATH_DELIMITER);
    }
    filePath.append(fileName);
    return filePath;
}
} // namespace FileUtil
} // namespace HiviewDFX
} // namespace OHOS
