/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "hiappevent_base.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "hiappevent_config.h"
#include "hilog/log.h"
#include "hitrace/trace.h"
#include "time_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EventBase"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr const char* DEFAULT_DOMAIN = "default";
constexpr size_t MIN_PARAM_STR_LEN = 3; // 3: '{}\0'

std::string TrimRightZero(const std::string& str)
{
    auto endIndex = str.find_last_not_of("0");
    if (endIndex == std::string::npos) {
        return str;
    }

    return (str[endIndex] == '.') ? str.substr(0, endIndex) : str.substr(0, endIndex + 1);
}

template<typename T>
std::string GetValueStr(const T& value)
{
    return std::to_string(value);
}

std::string GetValueStr(bool value)
{
    return value ? "true" : "false";
}

std::string GetValueStr(char value)
{
    return "\"" + std::to_string(value) + "\"";
}

std::string GetValueStr(float value)
{
    return TrimRightZero(std::to_string(value));
}

std::string GetValueStr(double value)
{
    return TrimRightZero(std::to_string(value));
}

std::string GetValueStr(const std::string& value)
{
    return "\"" + value + "\"";
}

template<typename T>
std::string GetValuesStr(const std::vector<T>& values)
{
    std::string valuesStr;
    valuesStr.append("[");
    size_t valuesSize = values.size();
    for (size_t i = 0; i < valuesSize; ++i) {
        if constexpr (std::is_same_v<std::decay_t<T>, bool>) { // vector<bool> is stored as bit type
            bool bValue = values[i];
            valuesStr.append(GetValueStr(bValue));
        } else {
            valuesStr.append(GetValueStr(values[i]));
        }
        if (i != (valuesSize - 1)) { // -1 for last value
            valuesStr.append(",");
        }
    }
    valuesStr.append("]");
    return valuesStr;
}

template<typename T>
std::string GetParamValueStrByType(const AppEventParamValue& value)
{
    auto* ptr = std::get_if<T>(&value);
    if (ptr == nullptr) {
        HILOG_WARN(LOG_CORE, "the variant value type mismatch.");
        return "";
    }
    return GetValueStr(*ptr);
}

template<typename T>
std::string GetParamValuesStrByType(const AppEventParamValue& value)
{
    auto* ptr = std::get_if<T>(&value);
    if (ptr == nullptr) {
        HILOG_WARN(LOG_CORE, "the variant values type mismatch for vector type.");
        return "";
    }
    return GetValuesStr(*ptr);
}

std::string GetParamValueStr(const AppEventParam& param)
{
    switch (param.value.index()) {
        case AppEventParamType::EMPTY:
            return "";
        case AppEventParamType::BOOL:
            return GetParamValueStrByType<bool>(param.value);
        case AppEventParamType::CHAR:
            return GetParamValueStrByType<char>(param.value);
        case AppEventParamType::SHORT:
            return GetParamValueStrByType<int16_t>(param.value);
        case AppEventParamType::INTEGER:
            return GetParamValueStrByType<int>(param.value);
        case AppEventParamType::LONGLONG:
            return GetParamValueStrByType<int64_t>(param.value);
        case AppEventParamType::FLOAT:
            return GetParamValueStrByType<float>(param.value);
        case AppEventParamType::DOUBLE:
            return GetParamValueStrByType<double>(param.value);
        case AppEventParamType::STRING:
            return GetParamValueStrByType<std::string>(param.value);
        case AppEventParamType::BVECTOR:
            return GetParamValuesStrByType<std::vector<bool>>(param.value);
        case AppEventParamType::CVECTOR:
            return GetParamValuesStrByType<std::vector<char>>(param.value);
        case AppEventParamType::SHVECTOR:
            return GetParamValuesStrByType<std::vector<int16_t>>(param.value);
        case AppEventParamType::IVECTOR:
            return GetParamValuesStrByType<std::vector<int>>(param.value);
        case AppEventParamType::LLVECTOR:
            return GetParamValuesStrByType<std::vector<int64_t>>(param.value);
        case AppEventParamType::FVECTOR:
            return GetParamValuesStrByType<std::vector<float>>(param.value);
        case AppEventParamType::DVECTOR:
            return GetParamValuesStrByType<std::vector<double>>(param.value);
        case AppEventParamType::STRVECTOR:
            return GetParamValuesStrByType<std::vector<std::string>>(param.value);
        default:
            return "";
    }
}
}

AppEventParam::AppEventParam(std::string n, AppEventParamValue v) : name(n), value(v)
{}

AppEventParam::AppEventParam(const AppEventParam& param) : name(param.name), value(param.value)
{}

AppEventParam::~AppEventParam()
{}

AppEventPack::AppEventPack(const std::string& name, int type) : AppEventPack(DEFAULT_DOMAIN, name, type)
{}

AppEventPack::AppEventPack(const std::string& domain, const std::string& name, int type)
    : domain_(domain), name_(name), type_(type)
{
    InitTime();
    InitTimeZone();
    InitProcessInfo();
    InitTraceInfo();
    InitRunningId();
}

void AppEventPack::InitTime()
{
    time_ = TimeUtil::GetMilliseconds();
}

void AppEventPack::InitTimeZone()
{
    timeZone_ = TimeUtil::GetTimeZone();
}

void AppEventPack::InitProcessInfo()
{
    pid_ = getprocpid();
    tid_ = getproctid();
}

void AppEventPack::InitTraceInfo()
{
    HiTraceId hitraceId = HiTraceChain::GetId();
    if (!hitraceId.IsValid()) {
        return;
    }
    traceId_ = static_cast<int64_t>(hitraceId.GetChainId());
    spanId_ = static_cast<int64_t>(hitraceId.GetSpanId());
    pspanId_ = static_cast<int64_t>(hitraceId.GetParentSpanId());
    traceFlag_ = hitraceId.GetFlags();
}

void AppEventPack::InitRunningId()
{
    runningId_ = HiAppEventConfig::GetInstance().GetRunningId();
}

void AppEventPack::AddParam(const std::string& key)
{
    baseParams_.emplace_back(AppEventParam(key, std::monostate{}));
}

void AppEventPack::AddParam(const std::string& key, bool b)
{
    baseParams_.emplace_back(AppEventParam(key, b));
}

void AppEventPack::AddParam(const std::string& key, char c)
{
    baseParams_.emplace_back(AppEventParam(key, c));
}

void AppEventPack::AddParam(const std::string& key, int8_t num)
{
    baseParams_.emplace_back(AppEventParam(key, static_cast<int16_t>(num)));
}

void AppEventPack::AddParam(const std::string& key, int16_t s)
{
    baseParams_.emplace_back(AppEventParam(key, s));
}

void AppEventPack::AddParam(const std::string& key, int i)
{
    baseParams_.emplace_back(AppEventParam(key, i));
}

void AppEventPack::AddParam(const std::string& key, int64_t ll)
{
    baseParams_.emplace_back(AppEventParam(key, ll));
}

void AppEventPack::AddParam(const std::string& key, float f)
{
    baseParams_.emplace_back(AppEventParam(key, f));
}

void AppEventPack::AddParam(const std::string& key, double d)
{
    baseParams_.emplace_back(AppEventParam(key, d));
}

void AppEventPack::AddParam(const std::string& key, const char *s)
{
    if (s == nullptr) {
        return;
    }
    baseParams_.emplace_back(AppEventParam(key, s));
}

void AppEventPack::AddParam(const std::string& key, const std::string& s)
{
    baseParams_.emplace_back(AppEventParam(key, s));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<bool>& bs)
{
    baseParams_.emplace_back(AppEventParam(key, bs));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<char>& cs)
{
    baseParams_.emplace_back(AppEventParam(key, cs));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int8_t>& shs)
{
    std::vector<int16_t> values(shs.begin(), shs.end());
    baseParams_.emplace_back(AppEventParam(key, std::move(values)));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int16_t>& shs)
{
    baseParams_.emplace_back(AppEventParam(key, shs));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int>& is)
{
    baseParams_.emplace_back(AppEventParam(key, is));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int64_t>& lls)
{
    baseParams_.emplace_back(AppEventParam(key, lls));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<float>& fs)
{
    baseParams_.emplace_back(AppEventParam(key, fs));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<double>& ds)
{
    baseParams_.emplace_back(AppEventParam(key, ds));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<const char*>& cps)
{
    std::vector<std::string> strs;
    if (cps.size() != 0) {
        for (const auto& cp : cps) {
            if (cp != nullptr) {
                strs.emplace_back(cp);
            }
        }
    }
    baseParams_.emplace_back(AppEventParam(key, std::move(strs)));
}

void AppEventPack::AddParam(const std::string& key, const std::vector<std::string>& strs)
{
    baseParams_.emplace_back(AppEventParam(key, strs));
}

void AppEventPack::AddCustomParams(const std::unordered_map<std::string, std::string>& customParams)
{
    if (customParams.empty()) {
        return;
    }
    std::string paramStr = GetParamStr();
    if (paramStr.size() >= MIN_PARAM_STR_LEN) {
        std::stringstream jsonStr;
        if (paramStr.size() > MIN_PARAM_STR_LEN) {
            jsonStr << ",";
        }
        for (auto it = customParams.begin(); it != customParams.end(); ++it) {
            jsonStr << "\"" << it->first << "\":" << it->second << ",";
        }
        std::string customParamStr = jsonStr.str();
        customParamStr.erase(customParamStr.end() - 1); // -1 for delete ','
        paramStr.insert(paramStr.size() - 2, customParamStr); // 2 for '}\0'
        paramStr_ = paramStr;
    }
}

std::string AppEventPack::GetEventStr() const
{
    std::stringstream jsonStr;
    jsonStr << "{";
    AddBaseInfoToJsonString(jsonStr);
    AddParamsInfoToJsonString(jsonStr);
    jsonStr << "}" << std::endl;
    return jsonStr.str();
}

std::string AppEventPack::GetParamStr() const
{
    if (!paramStr_.empty()) {
        return paramStr_;
    }

    std::stringstream jsonStr;
    jsonStr << "{";
    AddParamsToJsonString(jsonStr);
    jsonStr << "}" << std::endl;
    return jsonStr.str();
}

void AppEventPack::AddBaseInfoToJsonString(std::stringstream& jsonStr) const
{
    jsonStr << "\"" << "domain_" << "\":" << "\"" << domain_ << "\",";
    jsonStr << "\"" << "name_" << "\":" << "\"" << name_ << "\",";
    jsonStr << "\"" << "type_" << "\":" <<  type_ << ",";
    jsonStr << "\"" << "time_" << "\":" << std::to_string(time_) << ",";
    jsonStr << "\"tz_\":\"" << timeZone_ << "\",";
    jsonStr << "\"" << "pid_" << "\":" << pid_ << ",";
    jsonStr << "\"" << "tid_" << "\":" << tid_;
    AddTraceInfoToJsonString(jsonStr);
}

void AppEventPack::AddTraceInfoToJsonString(std::stringstream& jsonStr) const
{
    if (traceId_ == 0) {
        return;
    }
    jsonStr << "," << "\"" << "traceid_" << "\":" << traceId_;
    jsonStr << "," << "\"" << "spanid_" << "\":" << spanId_;
    jsonStr << "," << "\"" << "pspanid_" << "\":" << pspanId_;
    jsonStr << "," << "\"" << "trace_flag_" << "\":" << traceFlag_;
}

void AppEventPack::AddParamsInfoToJsonString(std::stringstream& jsonStr) const
{
    // for event from writing
    if (baseParams_.size() != 0) {
        jsonStr << ",";
        AddParamsToJsonString(jsonStr);
        return;
    }

    // for event from the db
    size_t paramStrLen = paramStr_.length();
    if (paramStrLen > MIN_PARAM_STR_LEN) {
        jsonStr << "," << paramStr_.substr(1, paramStrLen - MIN_PARAM_STR_LEN); // 1: '{' for next char
    }
}

void AppEventPack::AddParamsToJsonString(std::stringstream& jsonStr) const
{
    if (baseParams_.empty()) {
        return;
    }
    for (const auto& param : baseParams_) {
        jsonStr << "\"" << param.name << "\":" << GetParamValueStr(param) << ",";
    }
    jsonStr.seekp(-1, std::ios_base::end); // -1 for delete ','
}

void AppEventPack::GetCustomParams(std::vector<CustomEventParam>& customParams) const
{
    for (const auto& param : baseParams_) {
        CustomEventParam customParam = {
            .key = param.name,
            .value = GetParamValueStr(param),
            .type = param.value.index(),
        };
        customParams.push_back(customParam);
    }
}

int64_t AppEventPack::GetSeq() const
{
    return seq_;
}

std::string AppEventPack::GetDomain() const
{
    return domain_;
}

std::string AppEventPack::GetName() const
{
    return name_;
}

int AppEventPack::GetType() const
{
    return type_;
}

uint64_t AppEventPack::GetTime() const
{
    return time_;
}

std::string AppEventPack::GetTimeZone() const
{
    return timeZone_;
}

int AppEventPack::GetPid() const
{
    return pid_;
}

int AppEventPack::GetTid() const
{
    return tid_;
}

int64_t AppEventPack::GetTraceId() const
{
    return traceId_;
}

int64_t AppEventPack::GetSpanId() const
{
    return spanId_;
}

int64_t AppEventPack::GetPspanId() const
{
    return pspanId_;
}

int AppEventPack::GetTraceFlag() const
{
    return traceFlag_;
}

std::string AppEventPack::GetRunningId() const
{
    return runningId_;
}

std::list<AppEventParam> AppEventPack::GetBaseParams() const
{
    return baseParams_;
}

void AppEventPack::SetSeq(int64_t seq)
{
    seq_ = seq;
}

void AppEventPack::SetDomain(const std::string& domain)
{
    domain_ = domain;
}

void AppEventPack::SetName(const std::string& name)
{
    name_ = name;
}

void AppEventPack::SetType(int type)
{
    type_ = type;
}

void AppEventPack::SetTime(uint64_t time)
{
    time_ = time;
}

void AppEventPack::SetTimeZone(const std::string& timeZone)
{
    timeZone_ = timeZone;
}

void AppEventPack::SetPid(int pid)
{
    pid_ = pid;
}

void AppEventPack::SetTid(int tid)
{
    tid_ = tid;
}

void AppEventPack::SetTraceId(int64_t traceId)
{
    traceId_ = traceId;
}

void AppEventPack::SetSpanId(int64_t spanId)
{
    spanId_ = spanId;
}

void AppEventPack::SetPspanId(int64_t pspanId)
{
    pspanId_ = pspanId;
}

void AppEventPack::SetTraceFlag(int traceFlag)
{
    traceFlag_ = traceFlag;
}

void AppEventPack::SetRunningId(const std::string& runningId)
{
    runningId_ = runningId;
}

void AppEventPack::SetBaseParams(const std::list<AppEventParam>& baseParams)
{
    for (const auto& baseParam : baseParams) {
        baseParams_.push_back(baseParam);
    }
}

void AppEventPack::SetParamStr(const std::string& paramStr)
{
    paramStr_ = paramStr;
}
} // namespace HiviewDFX
} // namespace OHOS
