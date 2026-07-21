/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace HiviewDFX {
constexpr char PROCESSOR_NAME[] = "name";
constexpr char DEBUG_MODE[] = "debugMode";
constexpr char ROUTE_INFO[] = "routeInfo";
constexpr char APP_ID[] = "appId";
constexpr char START_REPORT[] = "onStartReport";
constexpr char BACKGROUND_REPORT[] = "onBackgroundReport";
constexpr char PERIOD_REPORT[] = "periodReport";
constexpr char BATCH_REPORT[] = "batchReport";
constexpr char USER_IDS[] = "userIds";
constexpr char USER_PROPERTIES[] = "userProperties";
constexpr char EVENT_CONFIGS[] = "eventConfigs";
constexpr char EVENT_CONFIG_DOMAIN[] = "domain";
constexpr char EVENT_CONFIG_NAME[] = "name";
constexpr char EVENT_CONFIG_REALTIME[] = "isRealTime";
constexpr char CONFIG_ID[] = "configId";
constexpr char CUSTOM_CONFIG[] = "customConfigs";
constexpr char WATCHER_NAME[] = "name";
constexpr char APPEVENT_FILTERS[] = "appEventFilters";
constexpr char FILTER_DOMAIN[] = "domain";
constexpr char FILTER_NAMES[] = "names";
constexpr char FILTER_TYPES[] = "eventTypes";
constexpr char TRIGGER_CONDITION[] = "triggerCondition";
constexpr char FUNCTION_ONTRIGGER[] = "onTrigger";
constexpr char FUNCTION_ONRECEIVE[] = "onReceive";
constexpr char EVENT_INFO_PARAMS[] = "params";
constexpr char EVENT_INFO_EVENT_TYPE[] = "eventType";
constexpr char EVENT_INFOS_PROPERTY[] = "appEventInfos";

constexpr char NAMESPACE_NAME_HIAPPEVENT[] = "@ohos.hiviewdfx.hiAppEvent.hiAppEvent";
constexpr char CLASS_NAME_RESULTS[] = "@ohos.hiviewdfx.hiAppEvent.ResultsInner";
constexpr char CLASS_NAME_BUSINESSERROR[] = "@ohos.base.BusinessError";
constexpr char CLASS_NAME_INT[] = "std.core.Int";
constexpr char CLASS_NAME_LONG[] = "std.core.Long";
constexpr char CLASS_NAME_BOOLEAN[] = "std.core.Boolean";
constexpr char CLASS_NAME_DOUBLE[] = "std.core.Double";
constexpr char CLASS_NAME_STRING[] = "std.core.String";
constexpr char CLASS_NAME_ARRAY[] = "std.core.Array";
constexpr char FUNC_NAME_GETLONG[] = "getLong";
constexpr char CLASS_NAME_ITERATOR[] = "std.core.Iterator";
constexpr char CLASS_NAME_RECORD[] = "std.core.Record";
constexpr char CLASS_NAME_EVENT_GROUP[] = "@ohos.hiviewdfx.hiAppEvent.AppEventGroupInner";
constexpr char CLASS_NAME_EVENT_INFO[] = "@ohos.hiviewdfx.hiAppEvent.AppEventInfoInner";
constexpr char CLASS_NAME_EVENT_PACKAGE[] = "@ohos.hiviewdfx.hiAppEvent.AppEventPackageInner";
constexpr char CLASS_NAME_EVENT_PACKAGE_HOLDER[] = "@ohos.hiviewdfx.hiAppEvent.hiAppEvent.AppEventPackageHolder";
constexpr char ENUM_NAME_EVENT_TYPE[] = "@ohos.hiviewdfx.hiAppEvent.hiAppEvent.EventType";

constexpr char FUNC_NAME_TOINT[] = "toInt";
constexpr char FUNC_NAME_TOLONG[] = "toLong";
constexpr char FUNC_NAME_TOBOOLEAN[] = "toBoolean";
constexpr char FUNC_NAME_TODOUBLE[] = "toDouble";
constexpr char FUNC_NAME_NEXT[] = "next";

constexpr int BIT_MASK{1};
constexpr unsigned int BIT_ALL_TYPES{0xff};
} // namespace HiviewDFX
} // namespace OHOS

#endif // HIAPPEVENT_ANI_PARAMETER_NAME_H
