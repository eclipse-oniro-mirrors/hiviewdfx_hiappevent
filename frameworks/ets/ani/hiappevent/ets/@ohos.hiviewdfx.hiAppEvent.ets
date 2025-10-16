/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { AsyncCallback, BusinessError, RecordData } from '@ohos.base';

class AppEventInfoInner implements hiAppEvent.AppEventInfo {
    domain: string = "";
    name: string = "";
    eventType: hiAppEvent.EventType = hiAppEvent.EventType.FAULT;
    params: RecordData = {};
}

class AppEventPackageInner implements hiAppEvent.AppEventPackage {
    packageId: int = 0;
    row: int = 0;
    size: int = 0;
    data: string[] = [];
    appEventInfos: Array<hiAppEvent.AppEventInfo> = {};
}

class TriggerConditionInner implements hiAppEvent.TriggerCondition {
    row?: int | undefined;
    size?: int | undefined;
    timeOut?: int | undefined;
}

class AppEventFilterInner implements hiAppEvent.AppEventFilter {
    domain: string = "";
    eventTypes?: hiAppEvent.EventType[] | undefined;
    names?: string[] | undefined;
}

class AppEventGroupInner implements hiAppEvent.AppEventGroup {
    name: string = "";
    appEventInfos: Array<hiAppEvent.AppEventInfo> = {};
}

class WatcherInner implements hiAppEvent.Watcher {
    name: string = "";
    triggerCondition?: hiAppEvent.TriggerCondition | undefined;
    appEventFilters?: hiAppEvent.AppEventFilter[] | undefined;
    onTrigger?: ((curRow: int, curSize: int, holder: hiAppEvent.AppEventPackageHolder) => void) | undefined;
    onReceive?: ((domain: string, appEventGroups: Array<hiAppEvent.AppEventGroup>) => void) | undefined;
}

class AppEventReportConfigInner implements hiAppEvent.AppEventReportConfig {
    domain?: string | undefined;
    name?: string | undefined;
    isRealTime?: boolean | undefined;
}

class ProcessorInner implements hiAppEvent.Processor {
    name: string = "";
    debugMode?: boolean | undefined;
    routeInfo?: string | undefined;
    appId?: string | undefined;
    onStartReport?: boolean | undefined;
    onBackgroundReport?: boolean | undefined;
    periodReport?: int | undefined;
    batchReport?: int | undefined;
    userIds?: string[] | undefined;
    userProperties?: string[] | undefined;
    eventConfigs?: hiAppEvent.AppEventReportConfig[] | undefined;
    configId?: int | undefined;
    customConfigs?: Record<string, string> | undefined;
}

class ConfigOptionInner implements hiAppEvent.ConfigOption {
    disable?: boolean | undefined;
    maxStorage?: string | undefined;
}

interface Results {
    code: int;
    message: string;
}

class ResultsInner implements Results {
    code: int = 0;
    message: string = "";
}

export default namespace hiAppEvent {
    loadLibrary("hiappevent_ani");

    export enum EventType {
        FAULT = 1,
        STATISTIC = 2,
        SECURITY = 3,
        BEHAVIOR = 4
    }

    export namespace domain {
        const OS: string = "OS";
    }

    export namespace event {
        const USER_LOGIN: string = "hiappevent.user_login";
        const USER_LOGOUT: string = "hiappevent.user_logout";
        const DISTRIBUTED_SERVICE_START: string = "hiappevent.distributed_service_start";
        const APP_CRASH: string = "APP_CRASH";
        const APP_FREEZE: string = "APP_FREEZE";
        const APP_LAUNCH: string = "APP_LAUNCH";
        const SCROLL_JANK: string = "SCROLL_JANK";
        const CPU_USAGE_HIGH: string = "CPU_USAGE_HIGH";
        const BATTERY_USAGE: string = "BATTERY_USAGE";
        const RESOURCE_OVERLIMIT: string = "RESOURCE_OVERLIMIT";
        const ADDRESS_SANITIZER: string = "ADDRESS_SANITIZER";
        const MAIN_THREAD_JANK: string = "MAIN_THREAD_JANK";
    }

    export namespace param {
        const USER_ID: string = "user_id";
        const DISTRIBUTED_SERVICE_NAME: string = "ds_name";
        const DISTRIBUTED_SERVICE_INSTANCE_ID: string = "ds_instance_id";
    }

    export native function configure(config: ConfigOption): void;

    export interface ConfigOption {
        disable?: boolean;
        maxStorage?: string;
    }

    export interface AppEventInfo {
        domain: string;
        name: string;
        eventType: EventType;
        params: RecordData;
    }

    native function writeSync(info: AppEventInfo): Results;

    export function write(info: AppEventInfo): Promise<void> {
        return new Promise<void>((resolve: (v: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
            let cb = (): Results => { return writeSync(info) }
            taskpool.execute(cb).then((ret: Any): void => {
                let retError = new BusinessError<void>();
                let writeResults: Results = ret as Results;
                if (writeResults.code === 0) {
                    resolve(undefined);
                } else {
                    retError.code = writeResults.code;
                    retError.message = writeResults.message as string;
                    reject(retError)
                }
            });
        })
    }

    export function write(info: AppEventInfo, callback: AsyncCallback<void>): void {
        let cb = (): Results => { return writeSync(info) }
        taskpool.execute(cb).then((ret: Any): void => {
            let retError = new BusinessError<void>();
            let writeResults: Results = ret as Results;
            retError.code = writeResults.code;
            retError.message = writeResults.message as string;
            callback(retError, undefined);
        });
    }

    type ParamType = int | long | double | string | boolean | Array<string>;
    native function setEventParamSync(params: Record<string, ParamType>, domain: string, name?: string): Results;

    export function setEventParam(params: Record<string, ParamType>, domain: string, name?: string): Promise<void> {
        return new Promise<void>((resolve: (v: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
            let cb = (): Results => { return setEventParamSync(params, domain, name); }
            taskpool.execute(cb).then((ret: Any): void => {
                let retError = new BusinessError<void>();
                let setResults: Results = ret as Results;
                if (setResults.code === 0) {
                    resolve(undefined);
                } else {
                    retError.code = setResults.code;
                    retError.message = setResults.message as string;
                    reject(retError);
                }
            });
        })
    }

    native function setEventConfigSync(name: string, config: Record<string, ParamType>): Results;

    export function setEventConfig(name: string, config: Record<string, ParamType>): Promise<void> {
        return new Promise<void>((resolve: (v: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
            let cb = (): Results => { return setEventConfigSync(name, config); }
            taskpool.execute(cb).then((ret: Any): void => {
                let retError = new BusinessError<void>();
                let setResults: Results = ret as Results;
                if (setResults.code === 0) {
                    resolve(undefined);
                } else {
                    retError.code = setResults.code;
                    retError.message = setResults.message as string;
                    reject(retError);
                }
            });
        })
    }

    export interface AppEventPackage {
        packageId: int;
        row: int;
        size: int;
        data: string[];
        appEventInfos: Array<AppEventInfo>;
    }

    export class AppEventPackageHolder {
        private nativeHolder: long;
        private static native _finalize(nativeHolder: long): void;
        private static _registry = new FinalizationRegistry<long>((nativeHolder: long) => {
            AppEventPackageHolder._finalize(nativeHolder);
        });
        private native nativeConstructor(watcherName: string): void;
        constructor(watcherName: string) {
            this.nativeConstructor(watcherName);
            AppEventPackageHolder._registry.register(this, this.nativeHolder);
        }
        native setSize(size: int): void;
        native setRow(size: int): void;
        native takeNext(): AppEventPackage | null;
    }

    export interface TriggerCondition {
        row?: int;
        size?: int;
        timeOut?: int;
    }

    export interface AppEventFilter {
        domain: string;
        eventTypes?: EventType[];
        names?: string[];
    }

    export interface AppEventGroup {
        name: string;
        appEventInfos: Array<AppEventInfo>;
    }

    export interface Watcher {
        name: string;
        triggerCondition?: TriggerCondition;
        appEventFilters?: AppEventFilter[];
        onTrigger?: (curRow: int, curSize: int, holder: AppEventPackageHolder) => void;
        onReceive?: (domain: string, appEventGroups: Array<AppEventGroup>) => void;
    }

    export native function addWatcher(watcher: Watcher): AppEventPackageHolder;
    export native function removeWatcher(watcher: Watcher): void;
    export native function clearData(): void;
    export native function setUserId(name: string, value: string): void;
    export native function getUserId(name: string): string;
    export native function setUserProperty(name: string, value: string): void;
    export native function getUserProperty(name: string): string;

    export interface AppEventReportConfig {
        domain?: string;
        name?: string;
        isRealTime?: boolean;
    }

    export interface Processor {
        name: string;
        debugMode?: boolean;
        routeInfo?: string;
        appId?: string;
        onStartReport?: boolean;
        onBackgroundReport?: boolean;
        periodReport?: int;
        batchReport?: int;
        userIds?: string[];
        userProperties?: string[];
        eventConfigs?: AppEventReportConfig[];
        configId?: int;
        customConfigs?: Record<string, string>;
    }

    export native function addProcessor(processor: Processor): long;
    export native function removeProcessor(id: long): void;
}
