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

#ifndef HIAPPEVENT_ANI_PARAMETER_NAME_H
#define HIAPPEVENT_ANI_PARAMETER_NAME_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
const std::string PROCESSOR_NAME = "name";
const std::string DEBUG_MODE = "debugMode";
const std::string ROUTE_INFO = "routeInfo";
const std::string APP_ID = "appId";
const std::string START_REPORT = "onStartReport";
const std::string BACKGROUND_REPORT = "onBackgroundReport";
const std::string PERIOD_REPORT = "periodReport";
const std::string BATCH_REPORT = "batchReport";
const std::string USER_IDS = "userIds";
const std::string USER_PROPERTIES = "userProperties";
const std::string EVENT_CONFIGS = "eventConfigs";
const std::string EVENT_CONFIG_DOMAIN = "domain";
const std::string EVENT_CONFIG_NAME = "name";
const std::string EVENT_CONFIG_REALTIME = "isRealTime";
const std::string CONFIG_ID = "configId";
const std::string CUSTOM_CONFIG = "customConfigs";

constexpr char CLASS_NAME_APPEVENT[] = "L@ohos/hiviewdfx/hiAppEvent/hiAppEvent;";
constexpr char CLASS_NAME_RESULTS[] = "L@ohos/hiviewdfx/hiAppEvent/ResultsInner;";
constexpr char CLASS_NAME_INT[] = "Lstd/core/Int;";
constexpr char CLASS_NAME_BOOLEAN[] = "Lstd/core/Boolean;";
constexpr char CLASS_NAME_DOUBLE[] = "Lstd/core/Double;";
constexpr char CLASS_NAME_STRING[] = "Lstd/core/String;";
constexpr char CLASS_NAME_ARRAY[] = "Lescompat/Array;";
constexpr char FUNC_NAME_GETLONG[] = "getLong";
constexpr char CLASS_NAME_ITERATOR[] = "Lescompat/Iterator;";
constexpr char CLASS_NAME_RECORD[] = "Lescompat/Record;";
constexpr char FUNC_NAME_UNBOXED[] = "unboxed";
constexpr char FUNC_NAME_NEXT[] = "next";
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_ANI_PARAMETER_NAME_H
