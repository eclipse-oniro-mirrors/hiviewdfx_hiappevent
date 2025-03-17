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
#ifndef HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_FFRT_H
#define HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_FFRT_H

#include "ffrt.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
FFRT_C_API ffrt_error_t ffrt_submit_base_nb(ffrt_function_header_t* f, const ffrt_deps_t* in_deps,
    const ffrt_deps_t* out_deps, const ffrt_task_attr_t* attr);

static inline ffrt_error_t Submit(std::function<void()>&& func, std::initializer_list<ffrt::dependence> in_deps,
    std::initializer_list<ffrt::dependence> out_deps, const ffrt::task_attr& attr = {})
{
    ffrt_deps_t in{static_cast<uint32_t>(in_deps.size()), in_deps.begin()};
    ffrt_deps_t out{static_cast<uint32_t>(out_deps.size()), out_deps.begin()};
    return ffrt_submit_base_nb(ffrt::create_function_wrapper(std::move(func)), &in, &out, &attr);
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_NATIVE_LIB_HIAPPEVENT_INCLUDE_HIAPPEVENT_FFRT_H
