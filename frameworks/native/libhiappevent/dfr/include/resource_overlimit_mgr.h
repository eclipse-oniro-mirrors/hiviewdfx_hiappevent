/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_DFR_RESOURCE_OVERLIMIT_MGR_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_DFR_RESOURCE_OVERLIMIT_MGR_H

#include <map>
#include <string>
#include <nocopyable.h>

namespace OHOS {
namespace HiviewDFX {
inline constexpr const char *const RESOURCE_OVERLIMIT = "RESOURCE_OVERLIMIT";

class ResourceOverlimitMgr : public NoCopyable {
public:
    static ResourceOverlimitMgr& GetInstance();

    int SetEventConfig(const std::map<std::string, std::string>& configMap) const;
    void SetRunningId(const std::string& id);

private:
    ResourceOverlimitMgr() = default;
    ~ResourceOverlimitMgr() = default;

    bool IsValid(const std::string &key, const std::string &value) const;
    int UpdateProperty(const std::string& property, const std::string& value) const;
    std::string GetRawheapDir() const;

private:
    std::string runningId_;
};
}  // HiviewDFX
}  // OHOS
#endif  // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_DFR_RESOURCE_OVERLIMIT_MGR_H
