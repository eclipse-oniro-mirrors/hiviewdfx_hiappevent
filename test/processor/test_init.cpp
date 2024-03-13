/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "test_processor.h"

#include <iostream>

#include "app_event_processor_mgr.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "TestProcessor"

using namespace OHOS::HiviewDFX;
using namespace OHOS::HiviewDFX::HiAppEvent;

void __attribute__((constructor)) XInit(void)
{
    auto processor = std::make_shared<TestProcessor>();
    int ret = AppEventProcessorMgr::RegisterProcessor("test_processor", processor);
    HILOG_INFO(LOG_CORE, "register observer ret=%{public}d", ret);
}
