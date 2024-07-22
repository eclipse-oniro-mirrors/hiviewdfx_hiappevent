/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef APPEVENT_PACKAGEHOLDER_IMPL_H
#define APPEVENT_PACKAGEHOLDER_IMPL_H

#include "app_event_store.h"
#include "cj_ffi/cj_common_ffi.h"
#include "common.h"
#include "hiappevent_base.h"
#include "native/ffi_remote_data.h"

namespace OHOS {
namespace CJSystemapi {
namespace HiAppEvent {
int64_t GetObserverSeqByName(const std::string& name);
char* MallocCString(const std::string& origin);
class AppEventPackageHolderImpl : public OHOS::FFI::FFIData {
public:
    OHOS::FFI::RuntimeType* GetRuntimeType() override
    {
        return GetClassType();
    }
    AppEventPackageHolderImpl(const std::string& name, int64_t observerSeq);
    void SetSize(int size);
    std::tuple<int32_t, RetAppEventPackage> TakeNext();

private:
    std::string name_;
    int64_t takeSize_ = 0;
    int packageId_ = 0;
    int64_t observerSeq_ = 0;
    friend class OHOS::FFI::RuntimeType;
    friend class OHOS::FFI::TypeBase;
    static OHOS::FFI::RuntimeType* GetClassType()
    {
        static OHOS::FFI::RuntimeType runtimeType =
            OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("AppEventPackageHolderImpl");
        return &runtimeType;
    }
};
} // HiAppEvent
} // CJSystemapi
} // OHOS

#endif
