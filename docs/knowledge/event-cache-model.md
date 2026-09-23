# 事件缓存模型知识

本文记录应用事件持久化（cache）子系统的模型。事件写入与观察者分发见 `event-observer-model.md`；清理策略见 `AGENT.md` 的“存储与清理边界”章节。

## 作用域模型

cache 子系统是 `libhiappevent_base` 的存储后端，只做持久化与查询，不做事件校验、不做策略决策、不格式化日志、不驱动观察者。它由 `AppEventStore` 单例（`cache/app_event_store.h:38`）封装一个 `NativeRdb::RdbStore`（SQLite，经 `relational_store`），库文件 `appevent.db` 位于 `<storageDir>/databases/`，安全级别 `S1`，版本 `3`（`cache/app_event_store.cpp:43-44,205-207`）。

所有公开操作经 `ExecuteDbOperation`（`app_event_store.cpp:236-244`）走**先读后写升级**模式：先取 `shared_lock`（`dbMutex_`，`std::shared_mutex`）尝试执行；成功即返回，失败再升级为 `unique_lock` 重试，并按需 `InitDbStore` 重建或 `CheckAndRepairDbStore` 修复（`:246-270`）。`CheckAndRepairDbStore` 仅在 `E_SQLITE_CORRUPT` 时删除库文件、下次访问重建（`:272-284`）；多个 DAO 把 `E_SQLITE_CORRUPT` 透传上来以触发修复。

七张表均以 `seq INTEGER PRIMARY KEY AUTOINCREMENT` 为主键（`SqlUtil::CreateTable` 固定前置），表名与字段常量集中定义于 `cache/include/app_event_cache_common.h`，建表与升级在 `AppEventStoreCallback::OnCreate/OnUpgrade`（`app_event_store.cpp:124-180`）：

| 表 | 主键 | 关键字段 | DAO |
| --- | --- | --- | --- |
| `events` | seq INT64 | domain/name/tz/params/running_id TEXT；type/time/pid/tid/trace_flag INT；trace_id/span_id/pspan_id | `app_event_dao.cpp` |
| `observers` | seq INT64 | name TEXT、hash INT64、filters TEXT | `app_event_observer_dao.cpp` |
| `event_observer_mapping` | seq INT64 | event_seq INT、observer_seq INT | `app_event_mapping_dao.cpp` |
| `user_ids` | seq INT64 | name TEXT、value TEXT | `user_id_dao.cpp` |
| `user_properties` | seq INT64 | name TEXT、value TEXT | `user_property_dao.cpp` |
| `custom_event_params` | seq INT64 | running_id/domain/name/param_key/param_value TEXT、param_type INT | `custom_event_param_dao.cpp` |
| `api_stats` | seq INT64 | kit_name/api_name/metric TEXT（无 running_id） | `api_stats_dao.cpp` |

升级路径：v1→v2 给 `events` 加 `running_id` 列（`UpToDbVersion2`，`:109-114`）；v2→v3 给 `observers` 加 `filters` 列（`UpToDbVersion3`，`:116-121`）。

## 不要混用的身份

| 身份 | 用途 | 常见误用 |
| --- | --- | --- |
| `events.seq` | 事件行主键 | 与 `event_observer_mapping.seq` 混用 |
| `observers.seq` | 观察者行主键，即 observerSeq / processorId（同一事物） | 把 Watcher 与 Processor 的 seq 当成两个不同空间 |
| `event_observer_mapping` | event_seq↔observer_seq 关联表 | 删 `events` 行前不查映射引用，破坏引用完整性 |
| `observers.hash` | `0` 标识 Watcher，非 0 标识 Processor | 查全表当 Watcher 用，混入 Processor |
| `observers.hash` 作为查询键 | `QuerySeqAndFilters` 按 (name,hash) 唯一定位 | 以为 name 唯一，重复注册时取错行 |
| `custom_event_params` 主键 `(running_id,domain,name,param_key)` | 后绑定参数，查询时合并入事件 | 当成每事件 baseParams 写入 events.params |
| 空名自定义参数组 vs 具名参数组 | 查询时先查空名组再查具名组，两者合并 | 只查具名组，漏掉域级公共参数 |
| `events.params`（TEXT） | 只存事件自身的 baseParams JSON | 以为含 custom params（合并发生在查询时） |
| `api_stats`（无 running_id） vs `events`（有 running_id） | stat 中间表不按运行实例隔离 | 对 `api_stats` 套用 `events` 的 running_id 老化逻辑 |
| `user_ids` vs `user_properties` | 两张表结构相同但值长度限制不同（256 vs 1024），用途不同 | 互换表名 |
| S1 安全级别 | DB 访问控制 | 擅自改安全级别 |
| `shared_lock` 读 / `unique_lock` 写 | 先读后写升级，保护并发 | 每次都直接取 `unique_lock`，杀死读并发 |
| `E_SQLITE_CORRUPT` → 删库重建 | 修复策略 | 当成可原地恢复，跳过删库 |

## 默认作用域边界

cache 子系统的默认作用域只适合：

- `AppEventObserverMgr` 写事件与映射、查观察者待消费事件、删已消费映射（`InsertEvent`/`InsertObserver`/`InsertEventMapping`/`TakeEvents`/`DeleteEventMapping`/`DeleteEvent`）。
- `HiAppEventClean` 的老化与清空（`DeleteHistoryEvent(1000,150)`、`DeleteUnusedEventMapping`、`DeleteUnusedParamsExceptCurId`、`DeleteCustomEventParams`、各表清空）。
- `UserInfo` 读写 userId/userProperty（`user_ids`/`user_properties` 的增删查改）。
- `SetEventParam` 事务性写自定义参数（`InsertCustomEventParams`，≤64 条，溢出回滚）。
- `ApiStatsStorage` 暂存与清空 API 指标（`InsertApiMetricInfo`/`QueryApiMetricInfoAll`/`ClearApiMetricInfo`）。

非默认行为链（事件校验、策略下发、日志文件格式化、观察者触发回调）不在 cache 子系统，改动它们不应改 `cache/`。关键边界：

- 事件行**仅在无映射引用时**才允许从 `events` 物理删除：`DeleteEvent(vector<eventSeqs>)` 先 `QueryExistEvent` 查仍被引用的 seq，仅删未被引用者（`app_event_store.cpp:651-673`）。
- `TakeEvents` 只删映射、不删事件行（事件存活到所有观察者消费完）（`:499-523`）。
- 自定义参数在**查询时**合并入事件（`QueryEvents` 内联 `CustomEventParamDao::Query` 先空名后具名，`:547-550`；`QueryCustomParamsAdd2EventPack` 同理，`:563-579`），`InsertEvent` 写入的 `params` 列不含自定义参数。
- `api_stats` 行短期存活（备份写入→上报清空），不参与 `events` 的老化与引用计数。
- `DeleteUnusedParamsExceptCurId` 仅当 distinct `running_id` 组数 ≥ 50 才触发，保留最新 20 组（按 `MAX(seq)`）、当前 runningId 及 `events` 表中仍存在的 runningId（`:675-721`）。
- `InsertCustomEventParams` 事务内查旧 key、计总数 ≤ 64、拆分 insert/update、`Commit`；超限 `RollBack` 返回 `ERROR_INVALID_CUSTOM_PARAM_NUM`（`:358-409`）。
- `api_diagnostic` 域抑制“自定义参数为空”告警（`:571`）。

## 修改前检查

- 删 `events` 行前是否查了 `event_observer_mapping` 引用？是否用了 `DeleteEvent(vector)` 而非 `AppEventDao::Delete` 直删？
- `TakeEvents` 是否只删映射不删事件？改它时是否保留了事件行？
- 新增表：是否在 `OnCreate`（`:124-156`）加 `*Dao::Create`？是否在 `app_event_cache_common.h` 加表/字段常量？是否提升 DB 版本并在 `OnUpgrade`（`:158-180`）加 `UpToDbVersionN`？已有安装是否走 ALTER 升级而非重建？
- 改自定义参数写入：是否保持事务、≤64 上限、insert/update 拆分、超限回滚？
- 查自定义参数：是否先查空名组再查具名组并合并？是否对 `api_diagnostic` 域保留空告警抑制？
- 并发：是否经 `ExecuteDbOperation`（先读后写升级）？是否绕过直接用 `dbStore_`？
- `observers.hash=0` 的 Watcher 不变量是否被破坏？`QueryWatchers` 过滤是否仍正确（`app_event_observer_dao.cpp:150`）？
- 老化：`DeleteHistoryEvent` 是否保留最新 1000 普通事件 + 150 OS 事件（DOMAIN_OS=`"OS"`，`hiappevent_common.h:21`）？
- 安全级别 S1 是否被擅自改动？
- DAO 是否把 `E_SQLITE_CORRUPT` 透传以触发删库重建？

## 代码和测试

存储编排从 `frameworks/native/libhiappevent/cache/app_event_store.cpp` 的 `InitDbStore`、`ExecuteDbOperation`、`InsertEvent`、`TakeEvents`、`DeleteEvent(vector)`、`InsertCustomEventParams`、`DeleteHistoryEvent`、`DeleteUnusedParamsExceptCurId` 开始追踪。建表与升级从同文件的 `AppEventStoreCallback::OnCreate`/`OnUpgrade`（`:124-180`）开始。

各表 DAO：`cache/app_event_dao.cpp`（events）、`cache/app_event_observer_dao.cpp`（observers，`QueryWatchers` 过滤 `hash=0` 在 `:147-150`）、`cache/app_event_mapping_dao.cpp`（`QueryExistEvent` 在 `:90-121`）、`cache/custom_event_param_dao.cpp`、`cache/user_id_dao.cpp`、`cache/user_property_dao.cpp`、`cache/api_stats_dao.cpp`。表/字段常量在 `cache/include/app_event_cache_common.h`。单测见 `test/unittest/common/native/hiappevent_cache_test.cpp`。
