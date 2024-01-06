/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

    function createError1(name, type) {
        return { code: "401", message: "Parameter error. The type of " + name + " must be " + type + "." };
    }

    function createError2(message) {
        return { code: "401", message: message };
    }

    function assertErrorEqual(actualErr, expectErr) {
        expect(actualErr.code).assertEqual(expectErr.code)
        expect(actualErr.message).assertEqual(expectErr.message)
    }

    function writeTest() {
        hiAppEvent.write({
            domain: "test_domain",
            name: "test_name",
            eventType: hiAppEvent.EventType.FAULT,
            params: {"str_key":"str_value"}
        }, (err) => {
            expect(err).assertNull()
        });
    }

    function validProcessorTest(processor, done) {
        let processorId = hiAppEvent.addProcessor(processor);
        expect(processorId).assertLarger(0);
        writeTest();
        setTimeout(() => {
            hiAppEvent.removeProcessor(processorId);
            done();
        }, 1000);
    }

    function invalidProcessorTest(processor, expectErr) {
        let processorId = 0;
        try {
            processorId = hiAppEvent.addProcessor(processor);
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }
        hiAppEvent.removeProcessor(processorId);
    }

    /**
     * @tc.number: HiAppEventProcessorTest001_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with all configs.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest001_1', 0, async function (done) {
        let processor1 = {
            name: "test_processor",
            debugMode: true,
            routeInfo: "routeInfo",
            appId: "appId",
            onStartReport: true,
            onBackgroundReport: true,
            periodReport: 0,
            batchReport: 0,
            userIds: ["id1", "id2"],
            userProperties: ["prop1", "props"],
            eventConfigs: [
                {
                    domain: "test_domain",
                    name: "test_name",
                    isRealTime: true,
                }
            ]
        };
        validProcessorTest(processor1, done);
    });

    /**
     * @tc.number: HiAppEventProcessorTest001_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with special eventConfigs.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest001_2', 0, async function (done) {
        let processor1 = {
            name: "test_processor",
            eventConfigs: [
                {
                    name: "test_name",
                    isRealTime: true,
                }
            ]
        };
        validProcessorTest(processor1, done);

        let processor2 = {
            name: "test_processor",
            eventConfigs: [
                {
                    domain: "test_domain",
                    isRealTime: true,
                }
            ]
        };
        validProcessorTest(processor2, done);

        let processor3 = {
            name: "test_processor",
            eventConfigs: [
                {
                    isRealTime: false,
                }
            ]
        };
        validProcessorTest(processor3, done);
    });

    /**
     * @tc.number: HiAppEventProcessorTest002_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid types.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest002_1', 0, function () {
        let expectErr = createError1("config", "Processor");
        invalidProcessorTest("str", expectErr);
        invalidProcessorTest(0, expectErr);
        invalidProcessorTest(null, expectErr);
        invalidProcessorTest(undefined, expectErr);
    });

    /**
     * @tc.number: HiAppEventProcessorTest002_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid name.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest002_2', 0, function () {
        let expectErr2 = createError2("Invalid processor name.");
        let processor1 = {
            name: "",
        };
        invalidProcessorTest(processor1, expectErr2);

        let expectErr1 = createError1("name", "string");
        let processor2 = {
            name: null,
        };
        invalidProcessorTest(processor2, expectErr1);

        let processor3 = {
            name: 0,
        };
        invalidProcessorTest(processor3, expectErr1);

        let processor4 = {
            name: "xxx***",
        };
        invalidProcessorTest(processor4, expectErr2);

        let processor5 = {
            name: "123_processor",
        };
        invalidProcessorTest(processor5, expectErr2);

        const maxLen = 256;
        let processor6 = {
            name: 'a'.repeat(maxLen + 1),
        };
        invalidProcessorTest(processor6, expectErr2);
    });
});