# 事件策略模型知识

本文记录应用事件策略（policy）子系统的模型。事件写入与观察者分发见 `event-observer-model.md`；校验规则与错误码见 `AGENT.md` 的“校验规则速查”章节。

## 作用域模型

策略子系统只做“配置下发”，不产生事件、不落盘到 `appevent.db`、不触碰观察者与事件三表（`events`/`observers`/`event_observer_mapping`）。它把外部传入的键值配置，经三种转发机制之一送到 OS 侧的生产者（hiview / faultloggerd / hicollie）：

1. **沙箱目录 xattr**：在 `<cacheDir>/eventConfig` 或 `<cacheDir>/rawheap` 目录上写扩展属性 `user.event_config.<key>`，并为 hiview 进程（uid 1201）设置 ACL `u:1201:rwx`（`event_policy_utils.cpp:108`），由 hiview 读取决定日志行为。页签开关（pageSwitchLogEnable）、CPU 高负载、资源超限 rawheap、崩溃 minidump 持久化均走此路。
2. **弱符号直调**：`DFX_SetCrashLogConfig(type, value)`（`app_crash_policy.cpp:49` 的弱符号，由 faultloggerd 提供），仅 `AppCrashPolicy` 的二进制路径使用。
3. **Watchdog API**：`Watchdog::GetInstance().ConfigEventPolicy / SetEventConfig`（hicollie，`main_thread_jank_policy.cpp:54,64`），仅主线程卡帧策略使用。

一次 `SetEventPolicy` 调用先按 `name` 在 `EventPolicyMgr::policies_` 查表（`event_policy_mgr.cpp:69-77`），命中后委托给具体策略；未命中返回 `ERROR_INVALID_PARAM_VALUE`。策略内部按自身规则解析键值并转发。策略之间不共享状态，但共享 `EventPolicyUtils` 单例（xattr 读写、目录创建与 ACL、`runningId_` 缓存，`event_policy_utils.h:24-45`）。

## 不要混用的身份

| 身份 | 用途 | 常见误用 |
| --- | --- | --- |
| 策略名（注册键） | 在 `EventPolicyMgr::policies_` 查表 | 用页签事件名当策略名，如 `SetEventPolicy("APP_FREEZE", …)` 会查表失败（仅 `appFreezePolicy` 注册） |
| 页签事件名 | 映射到 `PageSwitchLogEnableCode`（0–3） | 当成策略名传入；只有 ADDRESS_SANITIZER/APP_CRASH/APP_FREEZE/RESOURCE_OVERLIMIT 支持 |
| `APP_CRASH` / `RESOURCE_OVERLIMIT` | 同时是注册键与页签事件名（双重身份） | 误以为所有页签事件名都能当注册键 |
| `ADDRESS_SANITIZER` / `APP_FREEZE` | 仅页签事件名，**非**注册键 | 当注册键调用，返回 `ERROR_INVALID_PARAM_VALUE` |
| 字符串配置 `map<string,string>` | 公开 API 下发的可读配置 | 以为所有策略等价处理；实则各自解析 |
| 二进制配置 `map<uint8_t,uint32_t>` | 直接类型化配置 | 除 `AppCrashPolicy` 外全部返回 `INVALID_PARAM=-1` |
| `PageSwitchLogEnableCode`（0–3） | 页签开关位码 | 与 `CrashLogConfigType`（0–4）混用，二者是不同枚举 |
| `CrashLogConfigType`（0–4） | `DFX_SetCrashLogConfig` 的 type 码 | 当成页签位码写入 xattr |
| configDir 子目录 `/eventConfig` | 页签、CPU、minidump、refinedFileName 的 xattr 目录 | 用 `/rawheap` 存这些键 |
| configDir 子目录 `/rawheap` | 资源超限 `js_heap_logtype` 的 xattr 目录 | 用 `/eventConfig` 存 rawheap 键 |
| `needRunningId=true` | xattr 值前缀 `<runningId>,`，用于跨重启失效判定 | 对页签/rawheap 用 false，导致旧 runningId 的开关不清除 |
| `needRunningId=false` | xattr 值不前缀 runningId（CPU、refinedFileName、minidump） | 对页签用 false，导致跨应用实例开关串扰 |
| camelCase 与 snake_case 键名 | 崩溃参数与卡帧参数两种写法均支持 | 只支持一种，另一侧调用方失效 |
| 弱符号 `DFX_SetCrashLogConfig` | 运行时由 faultloggerd 注入 | 未判空直接调用；缺符号时崩溃 |
| `MainThreadJankConfig` vs `MainThreadJankPolicy` | 前者走 `Watchdog::SetEventConfig`（不做命名统一），后者走 `ConfigEventPolicy`（先 TS→C 命名统一） | 混用二者注册键 `MAIN_THREAD_JANK` 与 `MAIN_THREAD_JANK_V2`/`mainThreadJankPolicy` |

## 默认作用域边界

策略子系统的默认作用域只适合：

- 通过 `OH_HiAppEvent_SetEventConfig` / `AppEventWriteFacade::SetEventPolicy` 传入的配置下发。
- 写 `<cacheDir>/eventConfig` 或 `<cacheDir>/rawheap` 目录 xattr 并设置 ACL。
- `AppCrashPolicy` 构造期一次性清理 `/data/storage/el2/log/hiappevent/info`（`app_crash_policy.cpp:35,139`）与从持久化 xattr 恢复 minidump（`InitMiniDumpConfig`）。

非默认事件链（事件采集、日志附件、外部日志 solid link、上报、`appevent.db` 写入、观察者触发）不在策略子系统，改动它们不应改 `policy/`；反之改 `policy/` 也不应影响这些链路。

## 修改前检查

- 传入的 `name` 是注册键吗？注意部分策略同时有 camelCase 与 UPPER_CASE 两个键，而 `ADDRESS_SANITIZER`/`APP_FREEZE` 仅是页签事件名、非注册键。
- 该事件支持页签开关吗？只有 ADDRESS_SANITIZER/APP_CRASH/APP_FREEZE/RESOURCE_OVERLIMIT 四个，CPU 高负载与主线程卡帧不支持（`event_policy_utils.cpp:48-52`）。
- 该策略支持二进制（`map<uint8_t,uint32_t>`）路径吗？只有 `AppCrashPolicy` 支持，其余返回 `INVALID_PARAM`。
- xattr 写到哪个子目录？`/eventConfig` 还是 `/rawheap`？`needRunningId` 取 true 还是 false？
- 崩溃/卡帧参数是否同时处理了 camelCase 与 snake_case 两种键名？
- 弱符号 `DFX_SetCrashLogConfig` 是否判空？`Watchdog` 是否可用？
- ACL `u:1201:rwx` 是否在 `GetConfigDir` 中设置（`event_policy_utils.cpp:108`）？hiview 读不到 xattr 等于配置未生效。
- 新增策略是否在 `EventPolicyMgr::InitializePolicies`（`event_policy_mgr.cpp:42-54`）注册？

## 代码和测试

策略注册与分发从 `frameworks/native/libhiappevent/policy/event_policy_mgr.cpp` 的 `InitializePolicies`、`SetEventPolicy` 开始追踪。共享的 xattr/目录/ACL/runningId 工具从 `policy/event_policy_utils.cpp` 的 `ConfigPageSwitch`、`GetConfigDir`、`SaveEventConfig`、`GetEventPageSwitchStatus` 开始追踪。

各策略实现在 `policy/address_sanitizer_policy.cpp`、`policy/app_crash_policy.cpp`、`policy/app_freeze_policy.cpp`、`policy/cpu_usage_high_policy.cpp`、`policy/main_thread_jank_policy.cpp`、`policy/resource_overlimit_policy.cpp`。公开入口在 `frameworks/native/libhiappevent/hiappevent_c.cpp:306` 的 `HiAppEventSetEventConfig`（`HiAppEvent_Config` 实为 `map<string,string>` 的 `reinterpret_cast`），以及 `frameworks/native/libhiappevent/hiappevent_facade.cpp:80-90` 的 `AppEventWriteFacade::SetEventPolicy`。单测见 `test/unittest/common/native/hiappevent_policy_test.cpp`。
