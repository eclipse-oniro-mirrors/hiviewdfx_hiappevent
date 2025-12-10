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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_MGR_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_MGR_H

#include <memory>

#include "app_event_processor.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class AppEventProcessorMgr {
public:
/**
 * @brief Adds the configuration information of the data processor.
 *
 * @param config configuration information of a data processor, including the name of the data processor.
 * @return ID of the data processor of the reported event, which uniquely identifies the data processor and can be
 *         used to remove the data processor.
 *         Returns a positive integer if the operation is successful; otherwise, returns a negative integer.
 * @warning This is a synchronous interface and involves time-consuming operations. To ensure performance, you are
 *          advised to use it in a child thread.
*/
    static int64_t AddProcessor(const ReportConfig& config);
/**
 * @brief Removes the data processor of a reported event.
 *
 * @param processorId ID of a data processor. The value must be greater than 0. The value is obtained by calling the
 *         'AddProcessor' interface.
 * @return Returns 0 if the operation is successful or processorId does not exist; otherwise, returns a negative
 *         integer.
 * @warning This is a synchronous interface and involves time-consuming operations. To ensure performance, you are
 *          advised to use it in a child thread.
*/
    static int RemoveProcessor(int64_t processorId);

/**
 * @brief Registers an AppEventProcessor object.
 *
 * @param name specifies the name of an AppEventProcessor object.
 * @param processor smart pointer to the AppEventProcessor object.
 * @return Returns 0 if the registration is successful; otherwise, returns a negative integer(returns -1 if the name
 *         is already registered).
*/
    static int RegisterProcessor(const std::string& name, std::shared_ptr<AppEventProcessor> processor);
/**
 * @brief Unregisters an AppEventProcessor object.
 *
 * @param name name of the AppEventProcessor object, which is registered via the 'RegisterProcessor' interface.
 * @return Returns 0 if the unregistration is successful; otherwise, returns a negative integer(returns -1 if the name
 *         is not found).
 * @warning This is a synchronous interface and involves time-consuming operations. To ensure performance, you are
 *          advised to use it in a child thread.
*/
    static int UnregisterProcessor(const std::string& name);

/**
 * @brief Sets configuration information of a data processor.
 *
 * @param processorSeq ID of the data processor of the reported event. This ID is obtained from the return value of the
 *        'AddProcessor' interface.
 * @param config configuration information of a data processor, including the name of the data processor.
 * @return Returns 0 if the setting is successful; otherwise, returns a negative integer(returns -1 if 'processorSeq'
 *         does not exist).
*/
    static int SetProcessorConfig(int64_t processorSeq, const ReportConfig& config);
/**
 * @brief Retrieves the configuration information of a data processor.
 *
 * @param processorSeq ID of the data processor of the reported event. This ID is obtained from the return value of the
 *        'AddProcessor' interface.
 * @param config a reference to the configuration information of a data processor, which will be filled with the
 *        retrieved data.
 * @return Returns 0 if the configuration information is successfully retrieved; otherwise, returns a negative integer
 *         (returns -1 if 'processorSeq' does not exist).
*/
    static int GetProcessorConfig(int64_t processorSeq, ReportConfig& config);

/**
 * @brief Gets the IDs of data processors with a specific name.
 *
 * @param name the name of the data processors, which is specified in the 'RegisterProcessor' interface.
 * @param [out] processorSeqs a reference to a vector that will receive the IDs of the data processors.
 * @return Returns 0 if the IDs of the data processors are successfully obtained; otherwise, returns a negative integer.
 * @warning This is a synchronous interface and involves time-consuming operations. To ensure performance, you are
 *          advised to use it in a child thread.
*/
    static int GetProcessorSeqs(const std::string& name, std::vector<int64_t>& processorSeqs);
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_PROCESSOR_MGR_H
