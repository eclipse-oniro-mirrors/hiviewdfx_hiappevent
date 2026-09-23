# AGENT.md — HiAppEvent 仓编码代理指引

本文件为 AI 编码代理（以及新维护者）在本仓 (`hiviewdfx_hiappevent`) 工作时提供指引。深入的概念模型见 `docs/knowledge/` 下的知识文档——**动手编辑前请先按 [知识路由](#知识路由) 选定并阅读对应文档，并陈述任务类别、已读文档与发现的约束**。

- [项目定位](#项目定位)
- [构建与测试](#构建与测试)
- [目录结构](#目录结构)
- [知识路由](#知识路由)
- [分层与 Facade 边界](#分层与-facade-边界)
- [核心数据流](#核心数据流)
- [不可混用的身份](#不可混用的身份)
- [校验规则速查](#校验规则速查)
- [存储与清理边界](#存储与清理边界)
- [并发与线程模型](#并发与线程模型)
- [常见改动入口](#常见改动入口)
- [修改前检查](#修改前检查)
- [修改前需升级](#修改前需升级)
- [编码约定](#编码约定)
- [验证](#验证)

## 项目定位

HiAppEvent 是 OpenHarmony DFX 子系统为应用提供的事件打点模块，用于记录应用运行过程中的故障、统计、安全、行为事件，并支持事件监听（Watcher）、事件上报处理（Processor）、用户标识（UserId/UserProperty）、外部日志（ExternalLog）等能力。

- 子系统：`hiviewdfx`，部件名 `hiappevent`
- SysCap：`SystemCapability.HiviewDFX.HiAppEvent`，仅 standard 系统
- 日志 domain：`0xD002D07`（`frameworks/native/libhiappevent/include/hiappevent_base.h:58`）

## 构建与测试

- 编译器：Clang 8.0.0 及以上，C++11 及以上。
- 构建系统：GN + Ninja（OpenHarmony 标准构建）。
- 部件声明：`bundle.json`。
- 构建产物（六个目标，见 `bundle.json:53-60`）：
  - `libhiappevent_base`：核心引擎（`frameworks/native/libhiappevent/`）
  - `hiappevent_ndk`：NDK C ABI（`frameworks/native/ndk/`）
  - `hiappevent` / `hiappevent_v9`：JS NAPI 模块（`frameworks/js/napi/`）
  - `cj_hiappevent_ffi`：仓颉 FFI（`frameworks/cj/ffi/`）
  - `ani_hiappevent_package`：ArkTS ANI（`frameworks/ets/ani/hiappevent/`）
  - `hiappevent_innerapi`：C++ 内部 API（`interfaces/native/inner_api/`）
- 版本符号锁定：所有动态库均使用 `*.map` 链接脚本锁定导出符号。新增公开 API 必须同步更新对应 `.map` 文件。
- 测试：`test/unittest/common/native/*.cpp`（gtest 单测）、`test/unittest/common/napi/*.js`（JS 测试应用）、`test/processor/`（可动态加载的示例 Processor）。测试目标声明于 `bundle.json:93`。
- 条件编译：`hiappevent.gni` 中 `hiappevent_hiviewdfx_api_metrics_enable` 控制是否启用 api_metrics 相关能力。

## 目录结构

```
hiviewdfx_hiappevent/
├── bundle.json / hiappevent.gni / hiappevent_aafwk.gni   # 构建编排
├── figures/                                              # 架构图
├── docs/knowledge/                                       # 概念模型知识文档
├── frameworks/
│   ├── native/libhiappevent/   # 核心引擎 libhiappevent_base
│   │   ├── cache/              # SQLite 持久化（事件、观察者、用户信息、自定义参数、api_stats）→ event-cache-model.md
│   │   ├── cleaner/            # 存储清理（DB cleaner / Log cleaner）
│   │   ├── observer/           # 观察者、监听器、Processor 代理、外部日志管理 → event-observer-model.md
│   │   ├── policy/             # 事件策略（崩溃、卡死、资源超限、ASAN、主线程卡帧、CPU 高负载）→ event-policy-model.md
│   │   ├── stat/               # API 统计聚合与定时上报 → api-stats-model.md
│   │   ├── utility/            # JSON、文件、SQL、时间、打点统计工具
│   │   ├── load/               # 动态库加载、Processor 配置加载
│   │   └── hiappevent_*.cpp    # write / config / verify / userinfo / clean / facade / c / base
│   ├── native/ndk/             # NDK C ABI（hiappevent_ndk.c + src/*_service.cpp + src/*_proxy.cpp）
│   ├── js/napi/                # JS NAPI（hiappevent / hiappevent_v9）
│   ├── cj/ffi/                 # 仓颉 FFI
│   └── ets/ani/                # ArkTS ANI
├── interfaces/native/
│   ├── inner_api/              # C++ 内部 API（AppEventProcessorMgr / AppEvent / base_type / AppApiMetric）
│   └── kits/include/hiappevent/# 公开 C 头文件
└── test/                       # 测试
```

## 知识路由

`docs/knowledge/` 下有四篇概念模型文档。**编辑前必读与当前任务对应的那一篇**，并在动手时陈述：(1) 任务类别；(2) 已读文档；(3) 从中发现的约束。文档之间相互交叉引用，落地后可顺链读相邻文档。

### 按任务路由

| 任务 | 必读文档 |
| --- | --- |
| 改事件写入/Watcher/Processor 注册与触发/OS 事件分发/外部日志 solid link | `docs/knowledge/event-observer-model.md` |
| 改崩溃/卡死/资源超限/ASAN/主线程卡帧/CPU 高负载策略、xattr 下发、弱符号/Watchdog 转发 | `docs/knowledge/event-policy-model.md` |
| 改 API 指标统计、聚合备份/上报时序、`api_stats` 表、直写 vs 聚合路径 | `docs/knowledge/api-stats-model.md` |
| 改 SQLite 表结构/DAO、`events`/`observers`/`event_observer_mapping` 三表关系、自定义参数、老化清理 | `docs/knowledge/event-cache-model.md` |

### 按路径路由

| 编辑目录 | 必读文档 |
| --- | --- |
| `frameworks/native/libhiappevent/observer/` | `event-observer-model.md` |
| `frameworks/native/libhiappevent/policy/` | `event-policy-model.md` |
| `frameworks/native/libhiappevent/stat/` | `api-stats-model.md` |
| `frameworks/native/libhiappevent/cache/` | `event-cache-model.md` |
| `frameworks/native/libhiappevent/cleaner/` | `event-cache-model.md`（清理章节） |
| `frameworks/native/libhiappevent/hiappevent_write.cpp` / `hiappevent_c.cpp` | `event-observer-model.md` |

### 按词汇路由

当任务描述、日志、issue 或代码中出现下列术语时，先读对应文档：

| 术语 | 必读文档 |
| --- | --- |
| `solid link`、`link_external_log`、`observers.hash`、`observerSeq`/`processorId`、`TriggerCondition.size` | `event-observer-model.md` |
| `pageSwitchLogEnable`、`DFX_SetCrashLogConfig`、`eventConfig`/`rawheap` 目录、`PageSwitchLogEnableCode` | `event-policy-model.md` |
| `api_exec_end`/`api_called_stat`、`BACKUP_TIME_MS`/`REPORT_TIME_MS`、`ApiDescriptor`、`ENABLE_API_METRICS` | `api-stats-model.md` |
| `QueryExistEvent`、`DeleteEvent(vector)`、`MAX_NUM_OF_CUSTOM_PARAMS`、`api_stats` 表、`E_SQLITE_CORRUPT` | `event-cache-model.md` |

## 分层与 Facade 边界

公开层（NDK、NAPI、CJ FFI、ANI、inner_api）**禁止**直接调用核心引擎内部类（`AppEventObserverMgr`、`HiAppEventConfig`、`VerifyAppEvent`、`AppEventStore` 等）。必须经过 `frameworks/native/libhiappevent/hiappevent_facade.h` 中的 Facade 静态类：

| Facade | 职责 |
| --- | --- |
| `AppEventConfigFacade` | disable / maxStorage / storageDir / runningId / 刷新空闲空间 |
| `AppEventWriteFacade` | `FacadeWriteEvent`、`FacadeSetEventParam`、`SetEventPolicy` |
| `AppEventObserverFacade` | `AddWatcher`、`AddProcessor`、`Load`、`RegisterProcessor`、`HandleEvents`、`HandleTimeout`、`HandleBackground`、`SubmitTaskToFFRTQueue`、`SetReportConfig`、`GetReportConfig`、`RemoveObserver` |
| `AppEventUserInfoFacade` | userId / userProperty 增删查 + 版本号 |
| `AppEventStoreFacade` | `QueryEvents`、`TakeEvents`、`DeleteData`、`QueryObserverSeq(s)`、`InitDbStore`、`DestroyDbStore`、`CheckStorageSpace`、`ClearData`、`IsStorageSpaceFull`、`ReleaseSomeStorageSpace` |
| `AppEventVerifyFacade` | 所有 `Verify*` / `IsValid*` 校验 |
| `AppEventUtilityFacade` | 时间、目录、文件等工具转发 |

Facade 是 ABI 稳定的公开面与演进中的引擎内部之间唯一被允许的接缝。新增公开能力时，先在引擎内部实现，再在对应 Facade 增加转发，最后在公开层调用 Facade。

## 核心数据流

### 事件写入
1. NDK：`OH_HiAppEvent_Write`（`frameworks/native/ndk/hiappevent_ndk.c`）→ `HiAppEventInnerWrite`（`frameworks/native/libhiappevent/hiappevent_c.cpp`）→ `VerifyAppEvent` → `SubmitWritingTask`。
2. 内部 API：`HiAppEvent::Write`（`interfaces/native/inner_api/src/app_event.cpp`）→ `VerifyIsApp` → `VerifyTheAppEvent` → `SubmitTaskToFFRTQueue` → `FacadeWriteEvent`。
3. NAPI：`NapiHiAppEventWrite::Write`（`frameworks/js/napi/src/napi_hiappevent_write.cpp`）经 `napi_create_async_work`，在 execute 回调中直接调用 `AppEventWriteFacade::FacadeWriteEvent`（注意：NAPI 写入走 libuv worker，**不经过** FFRT 队列）。
4. 实际落盘：`WriteEvent`（`frameworks/native/libhiappevent/hiappevent_write.cpp:68-106`）顺序守卫：`disable` → 空闲磁盘 < 300MB → pack 非空 → 存储目录非空 → 加锁建目录/按 1000 次频度触发清理 → 写 `app_event_<YYYYMMDD>.log` → `AppEventObserverMgr::HandleEvents` 分发。
5. 之后 `AppEventObserverMgr` 将事件插入 `events` 表，并按每个观察者的 filter 建立 `event_observer_mapping`。

### 事件监听（Watcher）
注册：`OH_HiAppEvent_CreateWatcher` → `NdkAppEventWatcherProxy` → `AppEventObserverFacade::AddWatcher` → `AppEventObserverMgr::AddWatcher`（持久化到 `observers` 表，`hash=0` 标识 watcher；按需启动 `OsEventListener`）。
触发：`AppEventObserver::ProcessEvent` 累加 row/size，达阈值调 `OnTrigger`；`ProcessTimeout` 每 30s 检查；若设置了 `onReceive` 则事件实时下发。

### 事件上报处理（Processor）
注册：`OH_HiAppEvent_AddProcessor` → 校验 `ReportConfig` → `Load(name)` 动态加载 `lib<name>.z.so` → `AppEventObserverFacade::AddProcessor` → `AppEventProcessorProxy`（持久化到 `observers` 表，`hash!=0`，hash 为 `configId` 或 `hash(ReportConfig.ToString())`）。
上报：`AppEventProcessorProxy::OnTrigger` 查询至多 100 条事件 → `OnReport` → 成功后删除已消费映射。

### OS 事件
`OsEventListener`（`observer/os_event_listener.cpp`）通过 inotify 监听 `<cacheDir>/hiappevent` 目录，解析 hiview 落盘的 JSON 事件，提取 `external_log`/`link_external_log`，经 `InsertLinkEvents` 克隆并按观察者建立 solid link，再 `HandleEvents`。由系统参数 `hiviewdfx.hiappevent.enable` 开关。

完整概念模型见 `docs/knowledge/event-observer-model.md`。

## 不可混用的身份

| 身份 | 用途 | 常见误用 |
| --- | --- | --- |
| Domain | 事件领域，字符串 `[a-zA-Z][a-zA-Z0-9_]*`，≤32，不允许 `$` | 与事件名混淆；校验规则不同 |
| Event name | 事件名，≤48（`hiappevent.` 前缀时剩余 ≤38），可含 `$` | 当成 domain 处理 |
| Event type | 整数 1–4（FAULT/STATISTIC/SECURITY/BEHAVIOR） | 当成字符串 |
| App event | 应用自写事件，domain 任意，存 `events` 表 + `app_event_*.log` | 与 OS 事件混淆 |
| OS event | hiview/OS 产生，domain 为 `OS`，经 inotify 进入 | 当成应用事件直接 Write |
| Watcher | `AppEventWatcher`，进程内监听，`observers.hash=0` | 与 Processor 混淆 |
| Processor | `AppEventProcessorProxy` 包装的动态库上报插件，`hash!=0` | 当成 Watcher |
| Observer | `AppEventObserver` 抽象基类（filter + 触发条件），Watcher 与 ProcessorProxy 均派生 | 直接实例化基类 |
| observerSeq / processorId | 同一事物——`observers` 表自增 `seq`。`AddProcessor` 返回值即 observerSeq | 误以为是两个不同 ID |
| userId | name/value，存 `user_ids` 表，值 ≤256 | 与 userProperty 互换 |
| userProperty | name/value，存 `user_properties` 表，值 ≤1024 | 与 userId 互换 |
| baseParams | 每事件参数列表，≤32，在 `AppEventPack::baseParams_`，由 `VerifyAppEvent` 校验 | 与 customEventParams 混淆 |
| customEventParams | 按 `(runningId,domain,name)` 持久化的参数，≤64，查询时合并，由 `VerifyCustomEventParams` 校验 | 当成 baseParams 写入 |
| ReportConfig | 描述整个 Processor（含 EventConfig 列表、configId、customConfigs） | 与 EventConfig 混淆 |
| EventConfig | ReportConfig 内单条 `(domain,name,isRealTime)` 过滤 | 当成 ReportConfig |
| TriggerCondition | row/size/timeout/onStartup/onBackground；`size` 仅对 watcher.onTrigger 有效，Processor 的 size 被置 0 | 给 Processor 设 size |
| external_log | 所有观察者共享的扁平数组 | 与 link_external_log 混用 |
| link_external_log | 每观察者一份的数组，用于 solid link（1:1） | 当成共享数组 |
| Facade 调用 | 公开层必须经 `AppEvent*Facade` | NAPI/NDK 直接调引擎内部类 |

## 校验规则速查

校验集中实现于 `frameworks/native/libhiappevent/hiappevent_verify.cpp`，错误码见 `hiappevent_base.h:29-53`。负值=硬失败（不写入），正值=部分成功（仍写入）。下表为硬限制，改动时不得突破：

| 常量 | 值 | 适用对象 |
| --- | --- | --- |
| `MAX_LEN_OF_DOMAIN` | 32 | 事件 domain |
| `MAX_LENGTH_OF_EVENT_NAME` | 48 | 事件名 |
| `MAX_LENGTH_OF_PARAM_NAME` | 32 | 参数名 |
| `MAX_NUM_OF_PARAMS` | 32 | 单事件 base 参数数（超出丢弃尾部） |
| `MAX_LENGTH_OF_STR_PARAM` | 8192 | 字符串参数值 |
| `MAX_LENGTH_OF_SPECIAL_STR_PARAM` | 1MB | 名为 `crash`/`anr` 的字符串参数 |
| `MAX_SIZE_OF_LIST_PARAM` | 100 | 数组参数长度（超出截断，仍写入） |
| `MAX_LEN_OF_WATCHER` | 32 | watcher 名 |
| `MAX_LENGTH_OF_USER_INFO_NAME` | 256 | userId / userProperty 名 |
| `MAX_LENGTH_OF_USER_ID_VALUE` | 256 | userId 值 |
| `MAX_LENGTH_OF_USER_PROPERTY_VALUE` | 1024 | userProperty 值 |
| `MAX_LENGTH_OF_PROCESSOR_NAME` | 256 | processor 名与 configName |
| `MAX_LEN_OF_BATCH_REPORT` | 1000 | `triggerCond.row` 上限 |
| `MAX_NUM_OF_CUSTOM_CONFIGS` | 32 | ReportConfig 中 customConfigs 数 |
| `MAX_LENGTH_OF_CUSTOM_CONFIG_NAME` | 32 | custom config 键 |
| `MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE` | 1024 | custom config 值 |
| `MAX_NUM_OF_CUSTOM_PARAMS` | 64 | 每 `(runningId,domain,name)` 自定义参数数（在 `app_event_store.cpp:378` 强制） |
| `MAX_LENGTH_OF_CUSTOM_PARAM` | 1024 | 自定义参数字符串值 |
| `MIN_APP_UID` | 20000 | `IsApp()` 判定（uid ≥ 20000 才视为应用） |

命名规则（`IsValidName`）：首字符 `[a-zA-Z]`（允许 `$` 时为 `[a-zA-Z$]`）；末字符 `[a-zA-Z0-9]`；中间 `[a-zA-Z0-9_]`。domain、watcher 名不允许 `$`。Prop 名（`IsValidPropName`，用于 processor/userId/userProperty 名）：首字符 `[a-zA-Z_$]`，其余 `[a-zA-Z0-9_$]`。

## 存储与清理边界

- 数据库：`appevent.db`（SQLite via `relational_store`），位于 `<storageDir>/databases/`，安全级别 S1，版本 3。建表与升级见 `cache/app_event_store.cpp` 的 `AppEventStoreCallback::OnCreate/OnUpgrade`。
- 表：`events`、`observers`、`event_observer_mapping`、`user_ids`、`user_properties`、`custom_event_params`、`api_stats`。字段常量在 `cache/include/app_event_cache_common.h`。
- `observers.hash`：`0` 表示 Watcher，非 0 表示 Processor（`app_event_observer_dao.cpp:147-150` `QueryWatchers` 过滤 `hash=0`）。
- `event_observer_mapping`：事件仅在无映射引用时才从 `events` 表删除（`AppEventStore::DeleteEvent` 先 `QueryExistEvent`）。
- 清理：`HiAppEventClean`（`hiappevent_clean.cpp`）每 1000 次写入检查一次空间；超限时先清日志空间（`AppEventLogCleaner`，按文件名时间戳从旧到新删），再清 DB（`AppEventDbCleaner`，保留最新 1000 条普通事件 + 150 条 OS 事件）。
- 空闲磁盘阈值：`FREE_SIZE_LIMIT = 300MB`（`hiappevent_config.cpp:50`），低于此值停止写入。由 FFRT 定时器每 10 分钟刷新。
- 外部日志目录容量阈值：`/data/storage/el2/log/hiappevent` 4MB，`…/resourcelimit` 1500MB（`observer/app_event_external_log_manager.cpp:35-38`）。

## 并发与线程模型

- **FFRT 队列** `AppEventQueue`（`app_event_observer_mgr.cpp`）：写任务、background 处理、AddProcessorAsync、api_stats 上报均提交至此。公开层只能通过 `AppEventObserverFacade::SubmitTaskToFFRTQueue` 提交。
- **FFRT 定时器**：`refreshTimer_`（10 分钟，刷新空闲磁盘）、`timeoutTimer_`（30 秒，驱动 `HandleTimeout`）。
- **初次 AddProcessor 同步**：用 `ffrt::promise` + 500ms 超时等待 DB seq 初始化（`app_event_observer_mgr.cpp:463-499`）。
- **OS 事件监听线程**：detach 的 `std::thread`，名 `OS_AppEvent_Ls`，阻塞 `read(inotifyFd_)`（`os_event_listener.cpp:245-282`）。
- **NAPI 异步工作**：`napi_create_async_work` + `napi_queue_async_work_with_qos`，execute 回调在 libuv worker 池运行（**不经过** FFRT 队列），直接调 `FacadeWriteEvent`。
- **锁**：写入/配置/清理用 `g_mutex`；观察者映射用 `watcherMutex_`/`processorMutex_`（shared_mutex）；每个观察者触发条件用 `condMutex_`；ProcessorProxy 配置/缓存用 `mutex_`；DB 用 `dbMutex_`（shared_mutex）；UserInfo 用 `g_mutex`；外部日志管理用 `mutex_`；`EventPolicyUtils::runningId_` 用 `rwMutex_`；`ModuleLoader` 用 `moduleMutex_`/`processorMutex_`。
- **重要**：`AppEventObserverMgr` 头文件中 `moduleLoader_` 成员必须声明在 `observers_` **之前**（`app_event_observer_mgr.h:87` 有注释），否则析构崩溃，禁止调换顺序。

## 常见改动入口

| 任务 | 起步位置 | 必读文档 |
| --- | --- | --- |
| 新增公开 NDK C API | `interfaces/native/kits/include/hiappevent/hiappevent.h` 声明，`frameworks/native/ndk/hiappevent_ndk.c` 实现，委托给 `*_service.cpp`，并更新 `frameworks/native/ndk/libhiappevent_ndk.map` | event-observer-model.md |
| 新增校验规则 | `frameworks/native/libhiappevent/hiappevent_verify.cpp`（加常量 + `IsValid*` + `Verify*`），经 `AppEventVerifyFacade` 暴露 | 本节“校验规则速查” |
| 新增 DB 表/DAO | `cache/include/app_event_cache_common.h`（表与字段常量），新增 `cache/*_dao.{h,cpp}`，在 `AppEventStoreCallback::OnCreate` 注册，提升 DB 版本并加 `UpToDbVersionN` | event-cache-model.md |
| 新增监听的 OS 事件 | 在 `observer/app_event_observer.cpp:38-53` `OS_EVENT_POS_INFOS` 加 name+bit 位，并在 `app_event_observer_mgr.cpp:58-69` `externalLogEvents` 加条目 | event-observer-model.md |
| 新增策略 | 在 `policy/` 派生 `EventPolicyBase`，在 `EventPolicyMgr::InitializePolicies`（`policy/event_policy_mgr.cpp:42-54`）注册 | event-policy-model.md |
| 改存储/清理阈值 | `cleaner/app_event_db_cleaner.cpp`（保留 1000/150）、`hiappevent_clean.cpp`（检查频度 1000）、`hiappevent_config.cpp`（300MB）、`observer/app_event_external_log_manager.cpp`（4MB/1500MB） | event-cache-model.md |
| 改 api_stats 时序 | `stat/include/api_stats_timer.h`（备份 10s、上报 60s） | api-stats-model.md |
| 新增 JS API | `frameworks/js/napi/`，在 `napi_hiappevent_init.cpp` 注册，更新 `libhiappevent.map`/`libhiappevent_napi.map` | event-observer-model.md |
| 新增 ANI 能力 | `frameworks/ets/ani/hiappevent/`，`hiappevent_ani.cpp` 实现，`@ohos.hiviewdfx.hiAppEvent.ets` 暴露 | event-observer-model.md |

## 修改前检查

- 这次写入/查询属于哪个 domain、哪个事件名？校验规则与该 domain 一致吗？
- 走的是 Watcher 路径还是 Processor 路径？`observers.hash` 取值正确吗？
- 公开层是否经 Facade？是否误直接调用了引擎内部类？
- 是否触碰 `observers`/`events`/`event_observer_mapping` 三表关系？删除事件前是否检查映射引用？
- userId 与 userProperty 是否被互换？值长度限制是否正确（256 vs 1024）？
- baseParams 与 customEventParams 是否被混淆？上限（32 vs 64）是否正确？
- 新增公开符号是否已加入对应 `*.map`？
- 是否在 FFRT 队列外提交了写任务？NAPI 的 execute 回调是否绕过了 FFRT 队列？
- `moduleLoader_` 成员顺序是否被破坏？
- OS 事件的 bit 位是否与 `OS_EVENT_POS_INFOS` 一致？是否同步更新了 `externalLogEvents`？
- 策略改动是否同时考虑了 xattr 持久化与弱符号 `DFX_*` 转发？

## 修改前需升级

下列边界为安全/兼容敏感项，改动前须先与对应模块负责人确认（Do not 自行变更）：

- **uid 阈值**：`MIN_APP_UID = 20000`（`hiappevent_verify.cpp`）是 `IsApp()` 判定边界，影响哪些进程被视为应用。改动会改变事件准入范围。
- **数据库安全级别**：`appevent.db` 设为 `SecurityLevel::S1`（`app_event_store.cpp:206`）。升降级影响沙箱数据访问控制。
- **系统参数门控**：`hiviewdfx.hiappevent.enable` 控制 OS 事件监听开关（`os_event_listener.cpp`）。改动影响 hiview 与本模块的交互契约。
- **DB schema 兼容**：表结构/版本号变更（`OnCreate`/`OnUpgrade`）影响已安装设备的升级路径，必须配套 `UpToDbVersionN` 的 ALTER 迁移。
- **公开 API 兼容**：NDK/NAPI/inner_api 签名、错误码、生命周期变更影响 ABI；新增/改动公开符号须更新对应 `*.map` 并走兼容性评审。
- **第三方依赖**：在 `bundle.json` 的 `component.deps.components` 新增依赖前须做许可证审查（Apache 2.0 兼容性）。

## 编码约定

- 遵循 OpenHarmony C++ 代码规范；头文件守卫采用 `#ifndef XXX_H` 形式。
- 每个 `.cpp` 文件顶部 `#define LOG_TAG "..."`（如 `"Write"`、`"ObserverMgr"`、`"Store"`、`"Verify"`、`"ProcessorProxy"`），`LOG_DOMAIN` 统一 `0xD002D07`。
- 公开头文件放在 `interfaces/native/kits/include/hiappevent/`（NDK）或 `interfaces/native/inner_api/include/`（内部 API）；引擎内部头文件放在 `frameworks/native/libhiappevent/include/` 及各子目录 `include/`。
- 公开 C API 使用 `extern "C"`，并配 Doxygen 注释（`@since`/`@syscap`/`@param`/`@return`）。
- 错误码负值表示硬失败、正值表示部分成功，新增错误码需在 `hiappevent_base.h` 的 `ErrorCode` 命名空间登记，并在 NAPI 层映射为 JS 错误（`napi_hiappevent_write.cpp:36-48`）。
- 子目录通过各自 `BUILD.gn` 暴露 `*_config` 公共配置与 source_set，供父目标聚合。
- 不在代码中添加注释，除非用户明确要求。

## 验证

完成改动后，按下列步骤自证任务完成。命令须在 OpenHarmony 源码根目录执行（本仓映射到 `base/hiviewdfx/hiappevent`）。

### 最小检查

1. **构建**（编译本部件）：
   ```
   hb build --part-name hiappevent
   ```
   若 `hb` 不可用，退化为全量构建：`./build.sh --product-name <产品名>` 后确认 `libhiappevent_base.z.so` 等六个产物生成。
2. **单测**（C++ gtest + JS）：
   ```
   hb test --part-name hiappevent
   ```
   或直接跑产物二进制：`out/<产品名>/tests/hiappevent_unittest`（对应 `test/unittest/common/native/*.cpp`）；JS 测试应用见 `test/unittest/common/napi/`（`config.json` 配置）。
3. **符号导出兼容性检查**（改动公开 API 时必做）：对比导出符号与 `*.map` 是否一致：
   ```
   llvm-nm -D --defined-only out/<产品名>/.../libhiappevent_ndk.z.so
   ```
   重点核对 `frameworks/native/ndk/libhiappevent_ndk.map`；NAPI 同理核对 `libhiappevent.map`/`libhiappevent_napi.map`、inner_api 核对 `libhiappevent_innerapi.map`。
4. **格式检查**（无独立 lint 脚本时至少跑）：
   ```
   clang-format --dry-run -i <改动文件>
   ```

### 任务专项验证

- 改 DB 表/DAO：补 `test/unittest/common/native/hiappevent_cache_test.cpp` 用例，覆盖建表、升级、增删查、引用完整性。
- 改策略：补 `hiappevent_policy_test.cpp`，覆盖注册键与页签事件名不对称、xattr 持久化、弱符号缺失分支。
- 改观察者/Watcher/Processor：补 `hiappevent_observer_test.cpp`/`hiappevent_watcher_test.cpp`，覆盖 `hash=0/!=0`、触发条件、solid link 克隆。
- 改 API 统计：补 `hiappevent_api_metric_test.cpp`，覆盖备份→查表→聚合→上报→清表的顺序。

### Done 定义

任务视为完成，当且仅当：

- 构建通过（上述步骤 1 无报错）。
- 受影响单测全绿（步骤 2），新增逻辑有对应用例且通过。
- 若触碰公开 API，步骤 3 符号导出与 `*.map` 一致或已同步更新 `.map`。
- 已按 [知识路由](#知识路由) 选定并阅读对应文档，[修改前检查](#修改前检查) 与 [修改前需升级](#修改前需升级) 清单逐条确认。
- 未触碰需升级边界，或已取得对应负责人确认。

### 最终回报

完成时回报：改动摘要、构建/测试结果（通过数/总数）、`.map` 是否变更、已读知识文档、遵循的约束、是否触发需升级项及其处理。

### 无法运行验证时的回退

若环境无法 `hb build`/`hb test`：至少跑步骤 4 的 `clang-format` 与步骤 3 的 `llvm-nm`（若已有产物）；用 `grep`/编译器前端静态核对改动是否引入新公开符号、是否破坏三表引用、是否误绕过 Facade；在回报中明确标注“未运行构建/单测，原因 <环境限制>”，并列出待人工验证项。
