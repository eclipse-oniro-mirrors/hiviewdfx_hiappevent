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

#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_TYPES_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_TYPES_H

#include <map>
#include <string>
#include <vector>

#include "base_type.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

class ApiDescriptor {
public:
    ApiDescriptor(const std::string& kitName, const std::string& apiName) : kitName_(kitName), apiName_(apiName) {}
    
    std::string KitName() const
    {
        return std::string(kitName_);
    }
    
    std::string ApiName() const
    {
        return std::string(apiName_);
    }

    std::string Description() const
    {
        return kitName_ + ':' + apiName_;
    }

    struct ApiDescriptorComparator {
        bool operator() (const ApiDescriptor& lApi, const ApiDescriptor& rApi) const
        {
            return lApi.Description() < rApi.Description();
        }
    };
    
private:
    std::string kitName_;
    std::string apiName_;
};

using ApiMetricsMap = std::map<ApiDescriptor, std::vector<ApiMetric>, ApiDescriptor::ApiDescriptorComparator>;

} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_STAT_INCLUDE_API_STATS_TYPES_H