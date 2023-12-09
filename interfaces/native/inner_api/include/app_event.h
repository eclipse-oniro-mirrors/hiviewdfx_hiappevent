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
#ifndef HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_H
#define HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_H

#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
namespace HiAppEvent {
enum EventType {
    FAULT = 1,
    STATISTIC = 2,
    SECURITY = 3,
    BEHAVIOR = 4
};

class Event {
public:
    Event(const std::string& domain, const std::string& name, EventType type);
    ~Event() = default;

    void AddParam(const std::string& key, bool value);
    void AddParam(const std::string& key, int32_t value);
    void AddParam(const std::string& key, int64_t value);
    void AddParam(const std::string& key, double value);
    /**
     * Note: To avoid implicit type conversion problems, do not support direct input
     * of string literal constants, for example:
     *     AddParam("str_key", "str_value") // Not supported
     */
    void AddParam(const std::string& key, const std::string& value);

    void AddParam(const std::string& key, const std::vector<bool>& value);
    void AddParam(const std::string& key, const std::vector<int32_t>& value);
    void AddParam(const std::string& key, const std::vector<int64_t>& value);
    void AddParam(const std::string& key, const std::vector<double>& value);
    /**
     * Note: To avoid implicit type conversion problems, do not support direct input of
     * initialization list and string literal constants, for example:
     *     AddParam("str_key", {"str1"}) // Not supported
     *     AddParam("str_key", {"str1", "str2"}) // Not supported
     *     AddParam("str_key", {str1}) // Not supported
     *     AddParam("str_key", {str1, str2}) // Supported
     */
    void AddParam(const std::string& key, const std::vector<std::string>& value);

    friend int Write(const Event& event);

private:
    std::shared_ptr<AppEventPack> eventPack_;
};

int Write(const Event& event);
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_INTERFACES_NATIVE_INNER_API_INCLUDE_APP_EVENT_H
