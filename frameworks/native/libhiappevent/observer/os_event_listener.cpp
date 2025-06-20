/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "event_json_util.h"
#include "file_util.h"
#include "hiappevent_base.h"
#include "hiappevent_common.h"
#include "hilog/log.h"
#include "parameters.h"
#include "storage_acl.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "OsEventListener"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int BUF_SIZE = 2048;
const std::string APP_EVENT_DIR = "/hiappevent";
const std::string RUNNING_ID_PROPERTY = "app_running_unique_id";
const std::string OS_LOG_PATH = "/data/storage/el2/log/hiappevent";
const std::string XATTR_NAME = "user.appevent";
const std::string KEY_HIAPPEVENT_ENABLE = "hiviewdfx.hiappevent.enable";

bool UpdateListenedEvents(const std::string& dir, uint64_t eventsMask)
{
    if (!FileUtil::SetDirXattr(dir, XATTR_NAME, std::to_string(eventsMask))) {
        HILOG_ERROR(LOG_CORE, "failed to set xattr dir=%{public}s, value=%{public}" PRIu64, dir.c_str(), eventsMask);
        return false;
    }
    return true;
}

uint64_t GetMaskFromDirXattr(const std::string& path)
{
    std::string value;
    if (!FileUtil::GetDirXattr(path, XATTR_NAME, value)) {
        HILOG_ERROR(LOG_CORE, "failed to get xattr.");
        return 0;
    }
    HILOG_INFO(LOG_CORE, "getxattr success value=%{public}s.", value.c_str());
    return static_cast<uint64_t>(std::strtoull(value.c_str(), nullptr, 0));
}
}

OsEventListener::OsEventListener()
{
    Init();
}

OsEventListener::~OsEventListener()
{
    HILOG_INFO(LOG_CORE, "~OsEventListener");
    inotifyThread_ = nullptr;
    if (inotifyFd_ != -1) {
        (void)inotify_rm_watch(inotifyFd_, inotifyWd_);
        fdsan_close_with_tag(inotifyFd_, fdsan_create_owner_tag(FDSAN_OWNER_TYPE_FILE, LOG_DOMAIN));
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
    if (!FileUtil::IsFileExists(osEventPath_)) {
        return;
    }
    // get subscribed events from dir xattr
    osEventsMask_ = GetMaskFromDirXattr(osEventPath_);

    // read os events from dir files
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
        AppEventStore::GetInstance().QueryCustomParamsAdd2EventPack(event);
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
    if (!OHOS::system::GetBoolParameter(KEY_HIAPPEVENT_ENABLE, true)) {
        HILOG_INFO(LOG_CORE, "hiappevent is disabled");
        RemoveOsEventDir();
        return false;
    }
    return InitDir(OS_LOG_PATH) && InitDir(osEventPath_) && RegisterDirListener(osEventPath_);
}

bool OsEventListener::RemoveOsEventDir()
{
    inotifyStopFlag_ = true;
    HILOG_INFO(LOG_CORE, "rm dir");
    return FileUtil::ForceRemoveDirectory(osEventPath_) && FileUtil::ForceRemoveDirectory(OS_LOG_PATH);
}

bool OsEventListener::InitDir(const std::string& dirPath)
{
    if (!FileUtil::IsFileExists(dirPath) && !FileUtil::ForceCreateDirectory(dirPath)) {
        HILOG_ERROR(LOG_CORE, "failed to create dir=%{public}s", dirPath.c_str());
        return false;
    }
    if (OHOS::StorageDaemon::AclSetAccess(dirPath, "u:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "failed to set acl access dir=%{public}s", dirPath.c_str());
        return false;
    }
    return true;
}

bool OsEventListener::AddListenedEvents(uint64_t eventsMask)
{
    osEventsMask_ |= eventsMask;
    HILOG_INFO(LOG_CORE, "add mask=%{public}" PRIu64 ", eventsMask=%{public}" PRIu64, eventsMask, osEventsMask_);
    return UpdateListenedEvents(osEventPath_, osEventsMask_);
}

bool OsEventListener::SetListenedEvents(uint64_t eventsMask)
{
    osEventsMask_ = eventsMask;
    HILOG_INFO(LOG_CORE, "set eventsMask=%{public}" PRIu64, osEventsMask_);
    return UpdateListenedEvents(osEventPath_, osEventsMask_);
}

bool OsEventListener::RegisterDirListener(const std::string& dirPath)
{
    if (inotifyFd_ < 0) {
        inotifyFd_ = inotify_init();
        if (inotifyFd_ < 0) {
            HILOG_ERROR(LOG_CORE, "failed to inotify init : %s(%s).\n", strerror(errno), dirPath.c_str());
            return false;
        }
        uint64_t ownerTag = fdsan_create_owner_tag(FDSAN_OWNER_TYPE_FILE, LOG_DOMAIN);
        fdsan_exchange_owner_tag(inotifyFd_, 0, ownerTag);
        inotifyWd_ = inotify_add_watch(inotifyFd_, dirPath.c_str(), IN_MOVED_TO | IN_CLOSE_WRITE);
        if (inotifyWd_ < 0) {
            HILOG_ERROR(LOG_CORE, "failed to add watch entry : %s(%s).\n", strerror(errno), dirPath.c_str());
            fdsan_close_with_tag(inotifyFd_, ownerTag);
            inotifyFd_ = -1;
            return false;
        }
        HILOG_INFO(LOG_CORE, "inotify add watch dir=%{public}s successfully", dirPath.c_str());
    }
    inotifyStopFlag_ = false;
    if (inotifyThread_ == nullptr) {
        auto listenerPtr = shared_from_this();
        inotifyThread_ = std::make_unique<std::thread>([listenerPtr] { listenerPtr->HandleDirEvent(); });
        inotifyThread_->detach();
    }
    return true;
}

void OsEventListener::HandleDirEvent()
{
    if (pthread_setname_np(pthread_self(), "OS_AppEvent_Ls") != 0) {
        HILOG_WARN(LOG_CORE, "Failed to set threadName, errno=%{public}d", errno);
    }
    while (!inotifyStopFlag_) {
        char buffer[BUF_SIZE] = {0};
        char* offset = buffer;
        struct inotify_event* event = reinterpret_cast<struct inotify_event*>(buffer);
        if (inotifyFd_ < 0) {
            HILOG_ERROR(LOG_CORE, "Invalid inotify fd=%{public}d", inotifyFd_);
            break;
        }
        int len = read(inotifyFd_, buffer, sizeof(buffer) - 1);
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
    cJSON *eventJson = EventJsonUtil::GetJsonObjectFromJsonString(jsonStr);
    if (!eventJson) {
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json");
        return nullptr;
    }
    auto appEventPack = std::make_shared<AppEventPack>();
    appEventPack->SetDomain(EventJsonUtil::ParseString(eventJson, HiAppEvent::DOMAIN_PROPERTY));
    appEventPack->SetName(EventJsonUtil::ParseString(eventJson, HiAppEvent::NAME_PROPERTY));
    appEventPack->SetType(EventJsonUtil::ParseInt(eventJson, HiAppEvent::EVENT_TYPE_PROPERTY));
    cJSON* paramsJson = cJSON_GetObjectItemCaseSensitive(eventJson, HiAppEvent::PARAM_PROPERTY);
    if (paramsJson && cJSON_IsObject(paramsJson)) {
        cJSON *property = cJSON_GetObjectItemCaseSensitive(paramsJson, RUNNING_ID_PROPERTY.c_str());
        if (property && cJSON_IsString(property)) {
            appEventPack->SetRunningId(property->valuestring);
            cJSON_DeleteItemFromObject(paramsJson, RUNNING_ID_PROPERTY.c_str());
        }
        auto itemStr = cJSON_PrintUnformatted(paramsJson);
        appEventPack->SetParamStr(itemStr);
        cJSON_free(itemStr);
    }
    cJSON_Delete(eventJson);
    return appEventPack;
}
} // namespace HiviewDFX
} // namespace OHOS
