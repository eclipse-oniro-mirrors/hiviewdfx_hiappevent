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

#ifndef ANI_HIAPPEVENT_HOLDER_H
#define ANI_HIAPPEVENT_HOLDER_H

#include <ani.h>
#include <memory>
#include <string>
#include <vector>

#include "hiappevent_base.h"

namespace OHOS {
namespace HiviewDFX {
struct AppEventPackage {
    AppEventPackage() : packageId(0), row(0), size(0), data(0), events(0) {}
    ~AppEventPackage() {}
    int packageId;
    int row;
    int size;
    std::vector<std::string> data;
    std::vector<std::shared_ptr<AppEventPack>> events;
};

class AniAppEventHolder {
public:
    AniAppEventHolder(const std::string& name, int64_t observerSeq = -1);
    ~AniAppEventHolder() {}
    static void AniConstructor(ani_env *env, ani_object aniObject, ani_string watcherName);
    static void AniFinalize(ani_env *env, ani_object object, ani_long nativeHolder);
    static void AniSetRow(ani_env *env, ani_object object, ani_int size);
    static void AniSetSize(ani_env *env, ani_object object, ani_int size);
    static ani_object AniTakeNext(ani_env *env, ani_object object);

    void SetRow(int row);
    void SetSize(int size);
    std::shared_ptr<AppEventPackage> TakeNext();

private:
    std::string name_;
    int takeRow_;
    int takeSize_;
    int packageId_;
    int64_t observerSeq_;
    bool hasSetRow_;
    bool hasSetSize_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // ANI_HIAPPEVENT_HOLDER_H
