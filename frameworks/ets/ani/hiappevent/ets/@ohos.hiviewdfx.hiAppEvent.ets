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

import { AsyncCallback, BusinessError } from '@ohos.base';

class AppEventInfoInner implements hiAppEvent.AppEventInfo {
    domain: string = "";
    name: string = "";
    eventType: hiAppEvent.EventType = hiAppEvent.EventType.FAULT;
    params: object = {};
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
    periodReport?: number | undefined;
    batchReport?: number | undefined;
    userIds?: string[] | undefined;
    userProperties?: string[] | undefined;
    eventConfigs?: hiAppEvent.AppEventReportConfig[] | undefined;
    configId?: number | undefined;
    customConfigs?: Record<string, string> | undefined;
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

    export interface AppEventInfo {
        domain: string;
        name: string;
        eventType: EventType;
        params: object;
    }

    native function writeSync(info: AppEventInfo): Results;

    export function write(info: AppEventInfo): Promise<void> {
        return new Promise<void>((resolve: (v: undefined) => void, reject: (e: BusinessError<void>) => void): void => {
            let cb = (): Results => { return writeSync(info) }
            taskpool.execute(cb).then((ret: NullishType): void => {
                let retError = new BusinessError<void>();
                let writeResults: Results = ret as Results;
                if (writeResults.code === 0) {
                    resolve(undefined);
                } else {
                    retError.code = writeResults.code as number;
                    retError.message = writeResults.message as string;
                    reject(retError)
                }
            });
        })
    }

    export function write(info: AppEventInfo, callback: AsyncCallback<void>): void {
        let cb = (): Results => { return writeSync(info) }
        taskpool.execute(cb).then((ret: NullishType): void => {
            let retError = new BusinessError<void>();
            let writeResults: Results = ret as Results;
            retError.code = writeResults.code as number;
            retError.message = writeResults.message as string;
            callback(retError, undefined);
        });
    }

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
        periodReport?: number;
        batchReport?: number;
        userIds?: string[];
        userProperties?: string[];
        eventConfigs?: AppEventReportConfig[];
        configId?: number;
        customConfigs?: Record<string, string>;
    }

    export native function addProcessor(processor: Processor): number;
}
