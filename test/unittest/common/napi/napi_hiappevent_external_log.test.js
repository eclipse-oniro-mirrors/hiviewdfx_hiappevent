/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

import hiAppEvent from "@ohos.hiviewdfx.hiAppEvent"

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('HiAppEventJsTest', function () {
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all test cases
         */
        console.info('HiAppEventJsTest beforeAll called')
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all test cases
         */
        console.info('HiAppEventJsTest afterAll called')
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each test case
         */
        console.info('HiAppEventJsTest beforeEach called')
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each test case
         */
        console.info('HiAppEventJsTest afterEach called')
    })

    const ERROR_CODE_PARAM = "401";
    const ERROR_CODE_LOG_MANAGER = "11106001";
    const TEST_SYS_EVENT = "APP_CRASH";
    const TEST_TIMESTAMP = 1000000000;
    const TEST_FILE_SIZE = 1024;
    const TEST_LOG_COUNT = 10;

    /**
     * @tc.name: ExternalLogManagerTest001
     * @tc.desc: Test isExternalLogManagerRegistered before registerExternalLogManager.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogManagerTest001', 0, function () {
        console.info('ExternalLogManagerTest001 start');
        let isRegistered = hiAppEvent.isExternalLogManagerRegistered();
        expect(typeof isRegistered).assertEqual("boolean");
        console.info('ExternalLogManagerTest001 end');
    });

    /**
     * @tc.name: ExternalLogManagerTest002
     * @tc.desc: Test registerExternalLogManager with invalid params.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogManagerTest002', 0, function () {
        console.info('ExternalLogManagerTest002 start');
        try {
            hiAppEvent.registerExternalLogManager();
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_PARAM);
            expect(err.message).assertEqual("Parameter error. Incorrect parameter count. " +
                "Expected: 1, Actual: 0.");
        }
        try {
            hiAppEvent.registerExternalLogManager("invalid");
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_PARAM);
            expect(err.message).assertEqual("Parameter error. Param[0] type mismatch. " +
                "Expected: object, Actual: string.");
        }
        try {
            hiAppEvent.registerExternalLogManager({onCapacityReached: "invalid"});
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_PARAM);
            expect(err.message).assertEqual("Parameter error. Param[0].onCapacityReached type mismatch. " +
                "Expected: function, Actual: string.");
        }
        try {
            hiAppEvent.registerExternalLogManager({onCapacityReached: 123});
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_PARAM);
            expect(err.message).assertEqual("Parameter error. Param[0].onCapacityReached type mismatch. " +
                "Expected: function, Actual: number.");
        }
        console.info('ExternalLogManagerTest002 end');
    });

    /**
     * @tc.name: ExternalLogManagerTest003
     * @tc.desc: Test registerExternalLogManager with valid params.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogManagerTest003', 0, function () {
        console.info('ExternalLogManagerTest003 start');
        try {
            hiAppEvent.registerExternalLogManager({onCapacityReached: function (logs) {
                console.info('ExternalLogManagerTest003 onCapacityReached logs size=' + logs.length);
            }});
            expect(true).assertTrue();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_LOG_MANAGER);
        }
        console.info('ExternalLogManagerTest003 end');
    });

    /**
     * @tc.name: ExternalLogManagerTest004
     * @tc.desc: Test registerExternalLogManager with duplicated registration.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogManagerTest004', 0, function () {
        console.info('ExternalLogManagerTest004 start');
        try {
            hiAppEvent.registerExternalLogManager({onCapacityReached: function (logs) {
                console.info('ExternalLogManagerTest004 onCapacityReached logs size=' + logs.length);
            }});
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_LOG_MANAGER);
        }
        console.info('ExternalLogManagerTest004 end');
    });

    /**
     * @tc.name: ExternalLogManagerTest005
     * @tc.desc: Test isExternalLogManagerRegistered after registerExternalLogManager.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogManagerTest005', 0, function () {
        console.info('ExternalLogManagerTest005 start');
        let isRegistered = hiAppEvent.isExternalLogManagerRegistered();
        expect(typeof isRegistered).assertEqual("boolean");
        console.info('ExternalLogManagerTest005 end');
    });

    /**
     * @tc.name: ExternalLogContainerTest001
     * @tc.desc: Test ExternalLogContainer getAllLogs and getAllLogFiles.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogContainerTest001', 0, function () {
        console.info('ExternalLogContainerTest001 start');
        let container = new hiAppEvent.ExternalLogContainer();
        expect(container != null).assertTrue();
        let logs = container.getAllLogs();
        expect(Array.isArray(logs)).assertTrue();
        let files = container.getAllLogFiles();
        expect(Array.isArray(files)).assertTrue();
        console.info('ExternalLogContainerTest001 end');
    });

    /**
     * @tc.name: ExternalLogContainerTest002
     * @tc.desc: Test ExternalLogContainer getLogFilesOfSysEvent.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogContainerTest002', 0, function () {
        console.info('ExternalLogContainerTest002 start');
        let container = new hiAppEvent.ExternalLogContainer();
        let files = container.getLogFilesOfSysEvent(TEST_SYS_EVENT);
        expect(Array.isArray(files)).assertTrue();
        try {
            container.getLogFilesOfSysEvent(123);
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_PARAM);
        }
        console.info('ExternalLogContainerTest002 end');
    });

    /**
     * @tc.name: ExternalLogContainerTest003
     * @tc.desc: Test ExternalLogContainer getLogFilesGeneratedAfter and getLogFilesGeneratedBefore.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogContainerTest003', 0, function () {
        console.info('ExternalLogContainerTest003 start');
        let container = new hiAppEvent.ExternalLogContainer();
        let filesAfter = container.getLogFilesGeneratedAfter(TEST_TIMESTAMP);
        expect(Array.isArray(filesAfter)).assertTrue();
        let filesBefore = container.getLogFilesGeneratedBefore(TEST_TIMESTAMP);
        expect(Array.isArray(filesBefore)).assertTrue();
        console.info('ExternalLogContainerTest003 end');
    });

    /**
     * @tc.name: ExternalLogContainerTest004
     * @tc.desc: Test ExternalLogContainer getLogFilesLargerThan and getLogFilesSmallerThan.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogContainerTest004', 0, function () {
        console.info('ExternalLogContainerTest004 start');
        let container = new hiAppEvent.ExternalLogContainer();
        let filesLarger = container.getLogFilesLargerThan(TEST_FILE_SIZE);
        expect(Array.isArray(filesLarger)).assertTrue();
        let filesSmaller = container.getLogFilesSmallerThan(TEST_FILE_SIZE);
        expect(Array.isArray(filesSmaller)).assertTrue();
        console.info('ExternalLogContainerTest004 end');
    });

    /**
     * @tc.name: ExternalLogContainerTest005
     * @tc.desc: Test ExternalLogContainer getLogNumber and getFirstGeneratedLogFiles.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogContainerTest005', 0, function () {
        console.info('ExternalLogContainerTest005 start');
        let container = new hiAppEvent.ExternalLogContainer();
        let logNumber = container.getLogNumber();
        expect(logNumber).assertEqual(0);
        let firstLogs = container.getFirstGeneratedLogFiles(TEST_LOG_COUNT);
        expect(Array.isArray(firstLogs)).assertTrue();
        console.info('ExternalLogContainerTest005 end');
    });

    /**
     * @tc.name: ExternalLogContainerTest006
     * @tc.desc: Test ExternalLogContainer getFirstGeneratedLogFiles with invalid params.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogContainerTest006', 0, function () {
        console.info('ExternalLogContainerTest006 start');
        let container = new hiAppEvent.ExternalLogContainer();
        try {
            container.getFirstGeneratedLogFiles("invalid");
            expect().assertFail();
        } catch (err) {
            expect(err.code.toString()).assertEqual(ERROR_CODE_PARAM);
        }
        console.info('ExternalLogContainerTest006 end');
    });

    /**
     * @tc.name: ExternalLogWrapperTest001
     * @tc.desc: Test ExternalLogWrapper getFilePath and getSysEvent.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogWrapperTest001', 0, function () {
        console.info('ExternalLogWrapperTest001 start');
        let wrapper = new hiAppEvent.ExternalLogWrapper();
        expect(wrapper != null).assertTrue();
        let filePath = wrapper.getFilePath();
        expect(filePath).assertNull();
        let sysEvent = wrapper.getSysEvent();
        expect(sysEvent).assertNull();
        console.info('ExternalLogWrapperTest001 end');
    });

    /**
     * @tc.name: ExternalLogWrapperTest002
     * @tc.desc: Test ExternalLogWrapper getGenerationTime and getSizeInKb.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ExternalLogWrapperTest002', 0, function () {
        console.info('ExternalLogWrapperTest002 start');
        let wrapper = new hiAppEvent.ExternalLogWrapper();
        let generationTime = wrapper.getGenerationTime();
        expect(generationTime).assertNull();
        let sizeInKb = wrapper.getSizeInKb();
        expect(sizeInKb).assertNull();
        console.info('ExternalLogWrapperTest002 end');
    });
})
