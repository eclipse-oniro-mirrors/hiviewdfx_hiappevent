/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_HIAPPEVENT_CONFIG_H
#define HIVIEWDFX_HIAPPEVENT_CONFIG_H

/**
 * @file hiappevent_cfg.h
 *
 * @brief Defines the names of all the configuration items of the event logging configuration function.
 *
 * If you want to configure the event logging function, you can directly use the configuration item constants.
 *
 * Sample code:
 * <pre>
 *     bool res = HiAppEventConfigure(MAX_STORAGE, "100M");
 * </pre>
 *
 * @since 8
 * @version 1.0
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event logging switch.
 *
 * @since 8
 * @version 1.0
 */
#define DISABLE "disable"

/**
 * @brief Event file directory storage quota size.
 *
 * @since 8
 * @version 1.0
 */
#define MAX_STORAGE "max_storage"

#ifdef __cplusplus
}
#endif
#endif // HIVIEWDFX_HIAPPEVENT_CONFIG_H