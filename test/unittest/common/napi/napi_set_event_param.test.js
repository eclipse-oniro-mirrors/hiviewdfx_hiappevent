/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

    const TEST_DOMAIN = 'test_domain1';
    const TEST_NAME = 'test_name1';
    const TEST_PARAMS = { "key_int": 100 };
    const MAX_LEN_OF_DOMAIN = 32;
    const MAX_LENGTH_OF_EVENT_NAME = 48;
    const MAX_NUM_OF_CUSTOM_PARAMS = 64;

    function createError(code, message) {
        return { code: code.toString(), message: message };
    }

    function createError1(name, type) {
        return { code: "401", message: "Parameter error. The type of " + name + " must be " + type + "." };
    }

    function createError2(name) {
        return { code: "401", message: "Parameter error. The " + name + " parameter is mandatory." };
    }

    function assertErrorEqual(actualErr, expectErr) {
        expect(actualErr.code).assertEqual(expectErr.code)
        expect(actualErr.message).assertEqual(expectErr.message)
    }

    function writeTest(params) {
        hiAppEvent.write({
            domain: TEST_DOMAIN,
            name: TEST_NAME,
            eventType: hiAppEvent.EventType.FAULT,
            params: params
        }, (err) => {
            expect(err).assertNull()
        });
    }

    function setEventParamTestWithNameTestCatch(params, domain, name, expectErr, done) {
        try {
            hiAppEvent.setEventParam(params, domain, name);
        } catch (err) {
            assertErrorEqual(err, expectErr);
            console.info('HiAppEventSetEventParamTest setEventParam catch end');
            done();
        }
    }

    function setEventParamTestWithNameTest(params, domain, name, expectErr, done) {
        hiAppEvent.setEventParam(params, domain, name).then(() => {
            expect(expectErr).assertNull();
            done();
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
            done();
        });
    }

    function setEventParamTestCatch(params, domain, expectErr, done) {
        try {
            hiAppEvent.setEventParam(params, domain);
        } catch (err) {
            assertErrorEqual(err, expectErr);
            console.info('HiAppEventSetEventParamTest setEventParam catch end');
            done();
        }
    }

    function setEventParamTest(params, domain, expectErr, done) {
        hiAppEvent.setEventParam(params, domain).then(() => {
            expect(expectErr).assertNull();
            done();
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
            done();
        });
    }

    /**
     * @tc.number: HiAppEventSetEventParamTest001
     * @tc.name: HiAppEventSetEventParamTest
     * @tc.desc: SetEventParam with valid params.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventSetEventParamTest001', 0, async function (done) {
        let params = {
            "key_int": 1000,
            "key_string": "strValue",
            "key_bool": true,
            "key_float": 30949.3780498,
            "key_string_arr": ["a", "b", "c"]
        };
        setEventParamTestWithNameTest(params, TEST_DOMAIN, TEST_NAME, null, done);
        setEventParamTest(params, TEST_DOMAIN, null, done);
    });

    /**
     * @tc.number: HiAppEventSetEventParamTest002
     * @tc.name: HiAppEventSetEventParamTest
     * @tc.desc: SetEventParam with invalid parameters num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventSetEventParamTest002', 0, async function (done) {
        let expectErr = createError2("setEventParam");
        try {
            hiAppEvent.setEventParam();
        } catch (err) {
            assertErrorEqual(err, expectErr);
            done();
        }

        try {
            hiAppEvent.setEventParam("test");
        } catch (err) {
            assertErrorEqual(err, expectErr);
            done();
        }
    });

    /**
     * @tc.number: HiAppEventSetEventParamTest003
     * @tc.name: HiAppEventSetEventParamTest
     * @tc.desc: SetEventParam with invalid domain.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventSetEventParamTest003', 0, async function (done) {
        let expectErr = createError1("domain", "string");
        setEventParamTestCatch(TEST_PARAMS, 0, expectErr, done);
        setEventParamTestCatch(TEST_PARAMS, null, expectErr, done);
        setEventParamTestCatch(TEST_PARAMS, undefined, expectErr, done);

        expectErr = createError(11101001, "Invalid event domain.");
        setEventParamTest(TEST_PARAMS, "xxx***", expectErr, done);
        setEventParamTest(TEST_PARAMS, "123_domain", expectErr, done);
        setEventParamTest(TEST_PARAMS, "", expectErr, done);
        setEventParamTest(TEST_PARAMS, "a".repeat(MAX_LEN_OF_DOMAIN + 1), expectErr, done);

        setEventParamTest(TEST_PARAMS, "Domain", null, done);
        setEventParamTest(TEST_PARAMS, 'a'.repeat(MAX_LEN_OF_DOMAIN), null, done);
    });

    /**
     * @tc.number: HiAppEventSetEventParamTest004
     * @tc.name: HiAppEventSetEventParamTest
     * @tc.desc: Add processor with invalid name.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventSetEventParamTest004', 0, async function (done) {
        let expectErr = createError1("name", "string");
        setEventParamTestWithNameTestCatch(TEST_PARAMS, TEST_DOMAIN, 0, expectErr, done);
        setEventParamTestWithNameTestCatch(TEST_PARAMS, TEST_DOMAIN, null, expectErr, done);
        setEventParamTestWithNameTestCatch(TEST_PARAMS, TEST_DOMAIN, undefined, expectErr, done);

        expectErr = createError(11101002, "Invalid event name.");
        setEventParamTestWithNameTest(TEST_PARAMS, TEST_DOMAIN, "xxx***", expectErr, done);
        setEventParamTestWithNameTest(TEST_PARAMS, TEST_DOMAIN, "123_name", expectErr, done);
        setEventParamTestWithNameTest(TEST_PARAMS, TEST_DOMAIN, "a".repeat(MAX_LENGTH_OF_EVENT_NAME + 1), expectErr, done);

        setEventParamTestWithNameTest(TEST_PARAMS, TEST_DOMAIN, "", null, done);
        setEventParamTestWithNameTest(TEST_PARAMS, TEST_DOMAIN, "Name", null, done);
        setEventParamTestWithNameTest(TEST_PARAMS, TEST_DOMAIN, 'a'.repeat(MAX_LENGTH_OF_EVENT_NAME), null, done);
    });

    /**
     * @tc.number: HiAppEventSetEventParamTest005
     * @tc.name: HiAppEventSetEventParamTest
     * @tc.desc: Add processor with invalid params.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventSetEventParamTest005', 0, async function (done) {
        let expectErr = createError1("params", "object");
        setEventParamTestCatch(0, TEST_DOMAIN, expectErr, done);
        setEventParamTestCatch(true, TEST_DOMAIN, expectErr, done);
        setEventParamTestCatch("params_test", TEST_DOMAIN, expectErr, done);
        setEventParamTestCatch(null, TEST_DOMAIN, expectErr, done);
        setEventParamTestCatch(undefined, TEST_DOMAIN, expectErr, done);

        expectErr = createError(11101007, "The number of parameter keys exceeds the limit.");
        let params = {};
        for (var i = 1; i <= MAX_NUM_OF_CUSTOM_PARAMS + 1; ++i) {
            params["key" + i] = "value" + i;
        }
        setEventParamTest(params, TEST_DOMAIN, expectErr, done);

        // fail
        let params1 = {};
        for (var i = 1; i <= MAX_NUM_OF_CUSTOM_PARAMS; ++i) {
            params1["key" + i] = "value" + i;
        }
        setEventParamTest(params1, TEST_DOMAIN, null, done);
    });
});