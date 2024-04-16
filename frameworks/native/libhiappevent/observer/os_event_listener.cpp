/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "os_event_listener.h"

#include <cerrno>
#include <fstream>
#include <sys/inotify.h>

#include "app_event_observer_mgr.h"
#include "app_event_store.h"
#include "application_context.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "storage_acl.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "HiAppEventOsEventListener"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int BUF_SIZE = 2048;
const std::string APP_EVENT_DIR = "/hiappevent";
const std::string DOMAIN_PROPERTY = "domain";
const std::string NAME_PROPERTY = "name";
const std::string EVENT_TYPE_PROPERTY = "eventType";
const std::string PARAM_PROPERTY = "params";
const std::string RUNNING_ID_PROPERTY = "app_running_unique_id";
const std::string OS_LOG_PATH = "/data/storage/el2/log/hiappevent";
}

OsEventListener::OsEventListener()
{
    Init();
}

OsEventListener::~OsEventListener()
{
    HILOG_INFO(LOG_CORE, "~OsEventListener");
    inotifyStopFlag_ = true;
    inotifyThread_ = nullptr;
    if (inotifyFd_ != -1) {
        (void)inotify_rm_watch(inotifyFd_, inotifyWd_);
        close(inotifyFd_);
        inotifyFd_ = -1;
    }
}

void OsEventListener::Init()
{
    std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> context =
        OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "Context is null.");
        return;
    }
    if (context->GetCacheDir().empty()) {
        HILOG_ERROR(LOG_CORE, "The files dir obtained from context is empty.");
        return;
    }
    osEventPath_ = context->GetCacheDir() + APP_EVENT_DIR;

    std::vector<std::string> files;
    FileUtil::GetDirFiles(osEventPath_, files);
    GetEventsFromFiles(files, historyEvents_);
    for (auto& event : historyEvents_) {
        int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(event);
        if (eventSeq <= 0) {
            HILOG_WARN(LOG_CORE, "failed to store event to db");
            continue;
        }
        event->SetSeq(eventSeq);
    }
    for (const auto& file : files) {
        (void)FileUtil::RemoveFile(file);
    }
}

void OsEventListener::GetEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    events = historyEvents_;
}

bool OsEventListener::StartListening()
{
    return InitDir(OS_LOG_PATH) && InitDir(osEventPath_) && RegisterDirListener(osEventPath_);
}

bool OsEventListener::RemoveOsEventDir()
{
    HILOG_INFO(LOG_CORE, "rm dir");
    return FileUtil::ForceRemoveDirectory(osEventPath_) && FileUtil::ForceRemoveDirectory(OS_LOG_PATH);
}

bool OsEventListener::InitDir(const std::string& dirPath)
{
    if (!FileUtil::IsFileExists(dirPath) && !FileUtil::ForceCreateDirectory(dirPath)) {
        HILOG_ERROR(LOG_CORE, "failed to create dir=%{public}s", dirPath.c_str());
        return false;
    }
    if (OHOS::StorageDaemon::AclSetAccess(dirPath, "g:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set acl access dir=%{public}s", dirPath.c_str());
        return false;
    }
    return true;
}

bool OsEventListener::RegisterDirListener(const std::string& dirPath)
{
    if (inotifyFd_ < 0) {
        inotifyFd_ = inotify_init();
        if (inotifyFd_ < 0) {
            HILOG_ERROR(LOG_CORE, "failed to inotify init : %s(%s).\n", strerror(errno), dirPath.c_str());
            return false;
        }
        inotifyWd_ = inotify_add_watch(inotifyFd_, dirPath.c_str(), IN_MOVED_TO | IN_CLOSE_WRITE);
        if (inotifyWd_ < 0) {
            HILOG_ERROR(LOG_CORE, "failed to add watch entry : %s(%s).\n", strerror(errno), dirPath.c_str());
            close(inotifyFd_);
            inotifyFd_ = -1;
            return false;
        }
        HILOG_INFO(LOG_CORE, "inotify add watch dir=%{public}s successfully", dirPath.c_str());
    }
    inotifyStopFlag_ = false;
    if (inotifyThread_ == nullptr) {
        inotifyThread_ = std::make_unique<std::thread>(&OsEventListener::HandleDirEvent, this);
        inotifyThread_->detach();
    }
    return true;
}

void OsEventListener::HandleDirEvent()
{
    while (!inotifyStopFlag_) {
        char buffer[BUF_SIZE] = {0};
        char* offset = buffer;
        struct inotify_event* event = reinterpret_cast<struct inotify_event*>(buffer);
        if (inotifyFd_ < 0) {
            HILOG_ERROR(LOG_CORE, "Invalid inotify fd=%{public}d", inotifyFd_);
            break;
        }
        int len = read(inotifyFd_, buffer, BUF_SIZE);
        if (len <= 0) {
            HILOG_ERROR(LOG_CORE, "failed to read event");
            continue;
        }
        while ((offset - buffer) < len) {
            if (event->len != 0) {
                HILOG_INFO(LOG_CORE, "fileName: %{public}s event->mask: 0x%{public}x, event->len: %{public}d",
                    event->name, event->mask, event->len);
                std::string fileName = FileUtil::GetFilePathByDir(osEventPath_, std::string(event->name));
                HandleInotify(fileName);
            }
            uint32_t tmpLen = sizeof(struct inotify_event) + event->len;
            event = reinterpret_cast<struct inotify_event*>(offset + tmpLen);
            offset += tmpLen;
        }
    }
}

void OsEventListener::HandleInotify(const std::string& file)
{
    std::vector<std::shared_ptr<AppEventPack>> events;
    GetEventsFromFiles({file}, events);
    AppEventObserverMgr::GetInstance().HandleEvents(events);
    (void)FileUtil::RemoveFile(file);
}

void OsEventListener::GetEventsFromFiles(
    const std::vector<std::string>& files, std::vector<std::shared_ptr<AppEventPack>>& events)
{
    for (const auto& filePath : files) {
        std::vector<std::string> lines;
        if (!FileUtil::LoadLinesFromFile(filePath, lines)) {
            HILOG_ERROR(LOG_CORE, "file open failed, file=%{public}s", filePath.c_str());
            continue;
        }
        for (const auto& line : lines) {
            auto event = GetAppEventPackFromJson(line);
            if (event != nullptr) {
                events.emplace_back(event);
            }
        }
    }
}

std::shared_ptr<AppEventPack> OsEventListener::GetAppEventPackFromJson(const std::string& jsonStr)
{
    Json::Value eventJson;
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, eventJson)) {
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json");
        return nullptr;
    }
    if (!eventJson.isObject()) {
        return nullptr;
    }
    auto appEventPack = std::make_shared<AppEventPack>();
    if (eventJson.isMember(DOMAIN_PROPERTY) && eventJson[DOMAIN_PROPERTY].isString()) {
        appEventPack->SetDomain(eventJson[DOMAIN_PROPERTY].asString());
    }
    if (eventJson.isMember(NAME_PROPERTY) && eventJson[NAME_PROPERTY].isString()) {
        appEventPack->SetName(eventJson[NAME_PROPERTY].asString());
    }
    if (eventJson.isMember(EVENT_TYPE_PROPERTY) && eventJson[EVENT_TYPE_PROPERTY].isInt()) {
        appEventPack->SetType(eventJson[EVENT_TYPE_PROPERTY].asInt());
    }
    if (eventJson.isMember(PARAM_PROPERTY) && eventJson[PARAM_PROPERTY].isObject()) {
        Json::Value paramsJson = eventJson[PARAM_PROPERTY];
        if (paramsJson.isMember(RUNNING_ID_PROPERTY) && paramsJson[RUNNING_ID_PROPERTY].isString()) {
            appEventPack->SetRunningId(paramsJson[RUNNING_ID_PROPERTY].asString());
            paramsJson.removeMember(RUNNING_ID_PROPERTY);
        }
        appEventPack->SetParamStr(Json::FastWriter().write(paramsJson));
    }
    return appEventPack;
}
} // namespace HiviewDFX
} // namespace OHOS
