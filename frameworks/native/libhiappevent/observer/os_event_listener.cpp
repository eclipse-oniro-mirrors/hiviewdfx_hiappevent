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

namespace OHOS {
namespace HiviewDFX {
namespace {
const HiLogLabel LABEL = { LOG_CORE, HIAPPEVENT_DOMAIN, "HiAppEvent_OsEventListener" };
constexpr int BUF_SIZE = 2048;
const std::string APP_EVENT_DIR = "/hiappevent";
const std::string DOMAIN_PROPERTY = "domain";
const std::string NAME_PROPERTY = "name";
const std::string EVENT_TYPE_PROPERTY = "eventType";
const std::string PARAM_PROPERTY = "params";
}

OsEventListener::OsEventListener()
{
    Init();
}

OsEventListener::~OsEventListener()
{
    HiLog::Info(LABEL, "~OsEventListener");
    inotifyStopFlag_ = true;
    if (inotifyThread_ != nullptr) {
        inotifyThread_ = nullptr;
    }
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
        HiLog::Error(LABEL, "Context is null.");
        return;
    }
    if (context->GetCacheDir().empty()) {
        HiLog::Error(LABEL, "The files dir obtained from context is empty.");
        return;
    }
    osEventPath_ = context->GetCacheDir() + APP_EVENT_DIR;

    std::vector<std::string> files;
    FileUtil::GetDirFiles(osEventPath_, files);
    GetEventsFromFiles(files, historyEvents_);
    for (auto& event : historyEvents_) {
        int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(event);
        if (eventSeq <= 0) {
            HiLog::Warn(LABEL, "failed to store event to db");
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
    return InitDir(osEventPath_) && RegisterDirListener(osEventPath_);
}

bool OsEventListener::RemoveOsEventDir()
{
    HiLog::Info(LABEL, "rm dir");
    return FileUtil::ForceRemoveDirectory(osEventPath_);
}

bool OsEventListener::InitDir(const std::string& dirPath)
{
    if (!FileUtil::IsFileExists(dirPath) && !FileUtil::ForceCreateDirectory(dirPath)) {
        HiLog::Error(LABEL, "failed to create dir=%{public}s", dirPath.c_str());
        return false;
    }
    if (OHOS::StorageDaemon::AclSetAccess(dirPath, "g:1201:rwx") != 0) {
        HiLog::Error(LABEL, "failed to set acl access dir=%{public}s", dirPath.c_str());
        return false;
    }
    return true;
}

bool OsEventListener::RegisterDirListener(const std::string& dirPath)
{
    if (inotifyFd_ < 0) {
        inotifyFd_ = inotify_init();
        if (inotifyFd_ < 0) {
            HiLog::Error(LABEL, "failed to inotify init : %s(%s).\n", strerror(errno), dirPath.c_str());
            return false;
        }
        inotifyWd_ = inotify_add_watch(inotifyFd_, dirPath.c_str(), IN_MOVED_TO | IN_CLOSE_WRITE);
        if (inotifyWd_ < 0) {
            HiLog::Error(LABEL, "failed to add watch entry : %s(%s).\n", strerror(errno), dirPath.c_str());
            close(inotifyFd_);
            inotifyFd_ = -1;
            return false;
        }
        HiLog::Info(LABEL, "inotify add watch dir=%{public}s successfully", dirPath.c_str());
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
        char *offset = buffer;
        struct inotify_event *event = (struct inotify_event *)buffer;
        if (inotifyFd_ < 0) {
            HiLog::Error(LABEL, "Invalid inotify fd=%{public}d", inotifyFd_);
            break;
        }
        int len = read(inotifyFd_, buffer, BUF_SIZE);
        if (len <= 0) {
            HiLog::Error(LABEL, "failed to read event");
            continue;
        }
        while ((offset - buffer) < len) {
            if (event->len != 0) {
                HiLog::Info(LABEL, "fileName: %{public}s event->mask: 0x%{public}x, event->len: %{public}d",
                    event->name, event->mask, event->len);
                std::string fileName = FileUtil::GetFilePathByDir(osEventPath_, std::string(event->name));
                HandleInotify(fileName);
            }
            uint32_t tmpLen = sizeof(struct inotify_event) + event->len;
            event = (struct inotify_event *)(offset + tmpLen);
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
            HiLog::Error(LABEL, "file open failed, file=%{public}s", filePath.c_str());
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
        HiLog::Error(LABEL, "parse event detail info failed, please check the style of json");
        return nullptr;
    }

    auto appEventPack = std::make_shared<AppEventPack>();
    auto eventNameList = eventJson.getMemberNames();
    for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); ++it) {
        auto propertyName = *it;
        if (propertyName == DOMAIN_PROPERTY && eventJson[propertyName].isString()) {
            appEventPack->SetDomain(eventJson[propertyName].asString());
        } else if (propertyName == NAME_PROPERTY && eventJson[propertyName].isString()) {
            appEventPack->SetName(eventJson[propertyName].asString());
        } else if (propertyName == EVENT_TYPE_PROPERTY && eventJson[propertyName].isInt()) {
            appEventPack->SetType(eventJson[propertyName].asInt());
        } else if (propertyName == PARAM_PROPERTY) {
            std::string paramStr = Json::FastWriter().write(eventJson[propertyName]);
            appEventPack->SetParamStr(paramStr);
        }
    }
    return appEventPack;
}
} // namespace HiviewDFX
} // namespace OHOS
