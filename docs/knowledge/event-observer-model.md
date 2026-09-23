# 事件观察者模型知识

本文记录应用事件从写入到分发上报的模型。存储表结构与清理见 `AGENT.md` 的“存储与清理边界”章节；校验规则见 `AGENT.md` 的“校验规则速查”章节。

## 作用域模型

一次应用事件打点经过校验后，会同时进入两条作用域：**落盘作用域**与**观察者作用域**。

落盘作用域把事件持久化到 `events` 表与 `app_event_<YYYYMMDD>.log` 文件；观察者作用域把事件按 filter 投递给所有已注册的 Watcher 与 Processor。两者由 `AppEventObserverMgr::HandleEvents` 统一驱动，但拥有不同的生命周期：

- 落盘记录在存储配额超限或 `ClearData` 时被清理，与观察者是否消费无关。
- 一条事件被某观察者消费后，在 `event_observer_mapping` 中产生一条映射；事件行仅在**所有**引用它的映射都被删除后，才允许从 `events` 表物理删除。

属于某个观察者的事件过滤条件、触发条件与回调，不应读取或更新其他观察者的状态。

## 不要混用的身份

| 身份 | 用途 | 常见误用 |
| --- | --- | --- |
| Domain | 事件领域，`[a-zA-Z][a-zA-Z0-9_]*`，≤32，不允许 `$` | 当成事件名；套用事件名的校验规则 |
| 事件名 | ≤48（`hiappevent.` 前缀时剩余 ≤38），可含 `$` | 当成 domain 处理 |
| 事件类型 | 整数 1–4（FAULT/STATISTIC/SECURITY/BEHAVIOR） | 当成字符串传递 |
| 应用事件 | 应用自写，domain 任意，存 `events` + `*.log` | 与 OS 事件混淆 |
| OS 事件 | hiview 产生，domain 固定 `OS`，经 inotify 进入 | 用 `OH_HiAppEvent_Write` 写入 |
| Watcher | 进程内监听，`observers` 表 `hash=0` | 与 Processor 互换 |
| Processor | 动态库上报插件代理，`observers` 表 `hash!=0` | 当成 Watcher 注册 |
| Observer | 抽象基类（filter + 触发条件），Watcher 与 ProcessorProxy 派生 | 直接实例化基类 |
| observerSeq / processorId | 同一事物，即 `observers` 表自增 `seq` | 误以为是两个 ID |
| userId | 存 `user_ids`，值 ≤256 | 与 userProperty 互换 |
| userProperty | 存 `user_properties`，值 ≤1024 | 与 userId 互换 |
| baseParams | 每事件参数，≤32，`VerifyAppEvent` 校验 | 与 customEventParams 混淆 |
| customEventParams | 按 `(runningId,domain,name)` 持久化，≤64，查询时合并 | 当成 baseParams 写入 |
| ReportConfig | 描述整个 Processor（含 EventConfig 列表） | 与 EventConfig 混淆 |
| EventConfig | ReportConfig 内单条 `(domain,name,isRealTime)` 过滤 | 当成 ReportConfig |
| TriggerCondition | row/size/timeout/onStartup/onBackground | 给 Processor 设 size（会被置 0） |
| external_log | 所有观察者共享的扁平数组 | 与 link_external_log 混用 |
| link_external_log | 每观察者一份，用于 solid link（1:1） | 当成共享数组 |

函数只收到 domain/name 或 observerSeq 时，先确认它属于 Watcher 还是 Processor（看 `observers.hash` 是否为 0），再读写其过滤或触发状态。

## 默认作用域边界

落盘默认作用域只适合：

- 应用通过 `OH_HiAppEvent_Write` / `hiAppEvent.write` / `HiAppEvent::Write` 主动写入的事件。
- `AppEventStat`/`ApiStatsManager` 定时回写的 `api_exec_end` / `api_called_stat` 统计事件（domain `api_diagnostic`）。
- 不携带外部日志、不需要 solid link 的普通事件。

非默认事件链（OS 事件经 inotify 进入、带 `external_log`/`link_external_log` 的事件、Processor 触发上报）已经解析出观察者集合或 solid link 快照后，过滤、触发、回调、上报、外部日志清理都应继续使用已解析上下文，不应回退到默认落盘路径重新分发。

## 修改前检查

- 这次写入属于应用事件还是 OS 事件？走的是 `FacadeWriteEvent` 还是 `OsEventListener::HandleEvents`？
- 事件被哪个观察者集合消费？`observers.hash` 是否正确区分了 Watcher 与 Processor？
- 事件行删除前，是否检查了 `event_observer_mapping` 仍存在引用？
- userId 与 userProperty 是否被互换？值长度限制（256 vs 1024）是否正确？
- baseParams 与 customEventParams 是否被混淆？上限（32 vs 64）是否对应正确校验函数？
- `TriggerCondition.size` 对 Processor 无效（会被 `VerifyTriggerCondOfReportConfig` 置 0），是否仍被依赖？
- `external_log` 与 `link_external_log` 是否被混用？solid link 是否按观察者克隆？
- 公开层是否经 `AppEvent*Facade`？是否绕过 Facade 直接调用引擎内部类？
- 新增公开符号是否已加入对应 `*.map` 链接脚本？

## 代码和测试

事件写入从 `frameworks/native/libhiappevent/hiappevent_c.cpp` 的 `HiAppEventInnerWrite`、`hiappevent_write.cpp` 的 `WriteEvent` 开始追踪。分发与观察者管理从 `frameworks/native/libhiappevent/observer/app_event_observer_mgr.cpp` 的 `HandleEvents`、`AddWatcher`、`AddProcessor` 开始追踪。

Watcher 与 Processor 的过滤、触发、回调使用 `observer/app_event_observer.cpp` 与 `observer/app_event_processor_proxy.cpp`；OS 事件监听使用 `observer/os_event_listener.cpp`；外部日志使用 `observer/app_event_external_log_manager.cpp`。落盘与三表关系从 `frameworks/native/libhiappevent/cache/app_event_store.cpp` 的 `InsertEvent`、`DeleteEvent`、`TakeEvents` 开始追踪。

校验入口在 `frameworks/native/libhiappevent/hiappevent_verify.cpp`，错误码在 `frameworks/native/libhiappevent/include/hiappevent_base.h` 的 `ErrorCode` 命名空间。单测见 `test/unittest/common/native/` 下的 `hiappevent_observer_test.cpp`、`hiappevent_watcher_test.cpp`、`hiappevent_cache_test.cpp`、`hiappevent_verify_test.cpp`；可动态加载的示例 Processor 见 `test/processor/test_processor.cpp`。
