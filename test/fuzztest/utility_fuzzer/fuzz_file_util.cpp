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

#include <cstdint>
#include <string>
#include <vector>
#include "hiappevent_base.h"
#include "file_util.h"
#include "fuzz_cases.h"

namespace {
constexpr size_t MIN_FUZZ_DATA_SIZE = 1;
constexpr size_t FUZZ_KEY_MAX_LEN = 32;
constexpr size_t DEFAULT_STR_MAX_LEN = 64;
constexpr size_t FUZZ_FILE_MAX_LEN = 128;
constexpr int DEFAULT_DIR_MODE = 0755;
constexpr int DEFAULT_FILE_MODE = 0644;
}

static std::string FS(const uint8_t* d, size_t s, size_t m = DEFAULT_STR_MAX_LEN)
{
    return std::string(reinterpret_cast<const char*>(d), std::min(s, m));
}

static std::string SanitizeName(const std::string& input)
{
    std::string safe;
    for (char c : input) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            safe += c;
        }
    }
    return safe.empty() ? "fuzz" : safe;
}

void FuzzFileUtil(const uint8_t* data, size_t size)
{
    if (size < MIN_FUZZ_DATA_SIZE) {
        return;
    }
    std::string s = FS(data, size, FUZZ_FILE_MAX_LEN);
    std::string name = SanitizeName(FS(data, size, FUZZ_KEY_MAX_LEN));
    std::string base = "/data/test/fuzz/hiappevent";
    std::string p = base + "/" + name;

    std::string subDir = p + "/sub_" + name;
    OHOS::HiviewDFX::FileUtil::ForceCreateDirectory(subDir);
    OHOS::HiviewDFX::FileUtil::SaveStringToFile(subDir + "/f1.txt", "line1\nline2\nline3\n");
    OHOS::HiviewDFX::FileUtil::SaveStringToFile(subDir + "/f2.txt", s);

    std::vector<std::string> lines;
    OHOS::HiviewDFX::FileUtil::LoadLinesFromFile(subDir + "/f1.txt", lines);

    OHOS::HiviewDFX::FileUtil::SetDirXattr(subDir, "user.fuzz_attr", "attr_val");
    std::string xv;
    OHOS::HiviewDFX::FileUtil::GetDirXattr(subDir, "user.fuzz_attr", xv);

    OHOS::HiviewDFX::FileUtil::ForceRemoveDirectory(p);
    OHOS::HiviewDFX::FileUtil::ForceRemoveDirectory(p, false);
    OHOS::HiviewDFX::FileUtil::ForceRemoveDirectory(base + "/nonexist_" + name);

    OHOS::HiviewDFX::FileUtil::SaveStringToFile(p, s);
    OHOS::HiviewDFX::FileUtil::SaveStringToFile(p, s, true);
    OHOS::HiviewDFX::FileUtil::IsFileExists(p);
    OHOS::HiviewDFX::FileUtil::IsFile(p);
    OHOS::HiviewDFX::FileUtil::IsDirectory(p);
    OHOS::HiviewDFX::FileUtil::ForceCreateDirectory(p);
    OHOS::HiviewDFX::FileUtil::GetFilePathByDir(p, "sub");
    std::vector<std::string> fs;
    OHOS::HiviewDFX::FileUtil::GetDirFiles(base, fs);
    OHOS::HiviewDFX::FileUtil::GetDirSize(base);
    OHOS::HiviewDFX::FileUtil::GetFileSize(p);
    OHOS::HiviewDFX::FileUtil::ChangeMode(p, DEFAULT_DIR_MODE);
    OHOS::HiviewDFX::FileUtil::CreateFile(p, DEFAULT_FILE_MODE);
    OHOS::HiviewDFX::FileUtil::RemoveFile(p);
    OHOS::HiviewDFX::FileUtil::RemoveDirectory(p);
    OHOS::HiviewDFX::FileUtil::RemoveDirXattr(p, "attr");
    OHOS::HiviewDFX::FileUtil::RenameFile(p, p + "_r");
}
