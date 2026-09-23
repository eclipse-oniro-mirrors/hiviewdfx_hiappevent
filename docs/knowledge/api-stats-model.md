# API 统计模型知识

本文记录应用 API 指标统计（stat）子系统的模型。事件写入与观察者分发见 `event-observer-model.md`；存储表结构与清理见 `AGENT.md` 的“存储与清理边界”章节。

## 作用域模型

API 指标统计有**两条独立路径**，共用 `api_diagnostic` 域但产出不同事件名，粒度不同：

1. **实时直写路径**（`AppEventStat::WriteApiEndEventAsync`，`utility/app_event_stat.cpp:45`）：每次 API 调用立即写一条 `api_exec_end` 行为事件，带 `trans_id`/`api_name`/`sdk_name`/`begin_time`/`end_time`/`result`/`error_code`。经 `SubmitWritingTask(pack, "appevent_api_end")` 进入正常写流程。不聚合，一调用一事件。
2. **聚合上报路径**（`ApiMetricProcessor::ProcessApiMetric`，`stat/hiappevent_api_metric.cpp:39`）：每次调用只把一条原始 `ApiMetric{errCode,duration,successful}` 记入内存聚合器，不立即落事件。由两个 FFRT 延时定时器驱动周期性产出**聚合**事件 `api_called_stat`。

聚合路径的生命周期是**两阶段、顺序相关**的（`stat/api_stats_mgr.cpp`）：

- **备份定时器**（10 秒，`BACKUP_TIME_MS`，`api_stats_timer.h:36`）：`ScheduleBackUp` 在 `mut_` 锁内把 `aggregator_` 整体拷出并清空内存，再把拷出部分按条写入 `api_stats` 表（每条 metric 序列化为 JSON `{"errCode","duration","successful"}`，`api_stats_storage.cpp:69-76`）。备份失败则内存已清空的数据丢失（无回滚）。
- **上报定时器**（60 秒，`REPORT_TIME_MS`，`api_stats_timer.h:37`）：`ScheduleReport` 先调用 `ScheduleBackUpInner` 把残余内存备份入库，再 `QueryAll` 全表，`AggregateStats` 聚合成每 `(kit,api)` 一份 `ApiStatsReport`，逐份转成 `api_called_stat` 事件经 `SubmitWritingTask(pack, "api_stats_report")` 写入，最后 `Clear()` 清空 `api_stats` 表。

聚合产出的 `api_called_stat` 事件参数（`api_stats_mgr.cpp:120-131`）：`api_name`、`sdk_name`、`begin_time`（取 `now-60s`，非真实累积起点）、`call_times`、`success_times`、`max_cost_time`、`min_cost_time`、`total_cost_time`、`error_code_types`（数组）、`error_code_num`（数组）。

定时器自重排：回调执行后立即重新 `ffrt::submit(...delay(...))` 续排下一轮（`api_stats_timer.cpp:84-101,115,128`），用 `weak_ptr` 防止析构后回调。`isRunning_` 原子量在 `Stop` 时置假并清空回调。

## 不要混用的身份

| 身份 | 用途 | 常见误用 |
| --- | --- | --- |
| `api_exec_end` | 直写路径事件名，每调用一条 | 当成聚合事件，期望含统计字段 |
| `api_called_stat` | 聚合路径事件名，每 `(kit,api)` 每 60s 一条 | 当成每调用一条，重复打点 |
| `api_diagnostic` 域 | 两个事件名共用 | 以为该域下只有一个统计事件名 |
| `ApiDescriptor`（`kit:api`） | 聚合键，如 `PerformanceAnalysisKit:SomeApi` | 只用 `apiName` 当键，跨 kit 串并 |
| `ApiMetric` `{errCode,duration,successful}` | 单次调用的原始指标 | 与 `ApiStatsReport` 聚合结果混用 |
| `ApiStatsReport` | 聚合结果（callTimes/successTimes/min/max/total/错误码分布） | 当成原始单次数据写库 |
| 内存聚合器 vs `api_stats` 表 | 两阶段：10s 备份入表、60s 从表读再清 | 以为上报读的是内存（实际读 DB） |
| 备份定时器（10s） vs 上报定时器（60s） | 备份持久化、上报聚合+发射+清表 | 当成一个定时器 |
| `WriteApiEndEventAsync`（直写） vs `WriteApiEndMetric`/`ProcessApiMetric`（聚合） | 两条路径、不同事件名 | 混调导致同一 API 产生两种粒度事件 |
| `ENABLE_API_METRICS` 直方图宏 | 编译期门控（`hiappevent.gni:19-22`），仅在直写路径 | 以为聚合路径也有直方图 |
| `ReportApiMetric` 内部 API | 供**其它 kit** 上报自己的 `(kit,api)` 指标（`app_api_metric.cpp:24`） | 以为 kit 恒为 `PerformanceAnalysisKit` |
| `api_stats` 表（无 `running_id`） vs `events` 表（有 `running_id`） | stat 中间表按 `(kit,api,metric)` 存，与运行实例无关 | 对 `api_stats` 套用 `events` 的 running_id 清理逻辑 |
| `begin_time` 语义 | `api_exec_end` 用真实起止时间；`api_called_stat` 用 `now-60s` | 在聚合报告里期望单次调用起始时间 |
| `kit`（SDK/kit 标识） vs `domain`（事件域） | kit 是 `api_name`/`sdk_name` 参数，不是 domain | 把 kit 当成事件 domain |

## 默认作用域边界

stat 子系统的默认作用域只适合：

- 应用自身经 `AppEventUtilityFacade::WriteApiEndEventAsync` / `WriteApiEndMetric` 上报的 API 耗时与结果。
- 其它系统部件经内部 API `HiAppEvent::ReportApiMetric(apiInfo, metric)` 上报的 `(kit,api)` 指标（先经 `VerifyIsApp`，uid ≥ 20000）。
- 把原始 metric 暂存 `api_stats` 表、周期性聚合成 `api_called_stat` 事件再经正常写流程落入 `events` 表与日志。

非默认行为链（聚合事件经 `SubmitWritingTask` 进入的写盘、观察者分发、清理）不属于 stat 子系统职责，改动它们不应改 `stat/`；反之 `stat/` 也不应绕过 `SubmitWritingTask` 直接写 `events` 表。

聚合路径按设计有损：进程在内存未备份前退出则丢失该窗口数据；`api_stats` 中间表行短期存活（备份写入、上报清空），不参与 `events` 表的映射引用计数与老化清理。

## 修改前检查

- 走的是直写路径（`api_exec_end`）还是聚合路径（`api_called_stat`）？事件名与字段是否对应正确？
- 聚合键是否为完整 `ApiDescriptor`（`kit:api`）？是否漏了 kit 导致跨 kit 合并？
- `ScheduleReport` 的顺序是否保持“先备份残余 → 查全表 → 聚合 → 上报 → 清表”？调换会导致丢数据。
- 备份失败时内存已清空（无回滚），改动备份逻辑是否需引入失败重试或回滚？
- 定时器是否自重排？`Stop` 是否置 `isRunning_=false` 并清回调？`weak_ptr` 是否防析构后回调？
- `api_stats` 表是否被误套用 `events` 表的 `running_id` 清理或映射引用计数？
- `ProcessApiMetric` 是否校验 `duration >= 0`、`kit`/`api` 非空？`ReportApiMetric` 是否先 `VerifyIsApp`？
- 直方图宏是否在 `ENABLE_API_METRICS` 门控内？是否只在直写路径？
- `begin_time` 语义是否正确（直写真实时间 vs 聚合 `now-60s`）？
- 改动 `api_stats` 表结构是否同步改 `cache/api_stats_dao.cpp` 与 `AppEventStore` 的 `InsertApiMetricInfo`/`QueryApiMetricInfoAll`/`ClearApiMetricInfo`？

## 代码和测试

入口从 `frameworks/native/libhiappevent/utility/app_event_stat.cpp` 的 `WriteApiEndEventAsync`（直写）与 `WriteApiEndMetric`（聚合委托 `ApiMetricProcessor`）开始追踪；内部 API 入口在 `interfaces/native/inner_api/src/app_api_metric.cpp:24` 的 `ReportApiMetric`。聚合核心在 `frameworks/native/libhiappevent/stat/api_stats_mgr.cpp` 的 `AddRecord`、`ScheduleBackUp`、`ScheduleReport`、`ConvertReportToEventPack`。

定时器在 `stat/api_stats_timer.cpp`（`ScheduleBackUpTask`/`ScheduleReportTask`，FFRT 延时任务）；聚合算法在 `stat/api_stats_aggregator.cpp` 的 `Record`、`AggregateStats`；持久化在 `stat/api_stats_storage.cpp` 的 `Backup`/`QueryAll`/`Clear`，经 `cache/api_stats_dao.cpp` 的 `MetricInsert`/`MetricQueryAll`/`MetricClear` 操作 `api_stats` 表。类型定义在 `stat/include/api_stats_types.h`（`ApiDescriptor`）、`stat/include/api_stats_aggregator.h`（`ApiStatsReport`）、`interfaces/native/inner_api/include/base_type.h:26-35`（`ApiInfo`/`ApiMetric`）。单测见 `test/unittest/common/native/hiappevent_api_metric_test.cpp`。
