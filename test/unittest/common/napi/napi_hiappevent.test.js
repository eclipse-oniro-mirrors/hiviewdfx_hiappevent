/*
 * Copyright (C) 2021-2025 Huawei Device Co., Ltd.
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

import hiAppEventV9 from "@ohos.hiviewdfx.hiAppEvent"
import hiAppEvent from "@ohos.hiAppEvent"

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

    const TEST_DOMAIN = 'test_domain';
    const TEST_NAME = 'test_name';
    const TEST_TYPE = hiAppEvent.EventType.FAULT;
    const TEST_TYPE_V9 = hiAppEventV9.EventType.FAULT;
    const TEST_PARAMS = {};

    function createError(code, message) {
        return { code: code.toString(), message: message };
    }

    function createError2(name, type) {
        return { code: "401", message: "Parameter error. The type of " + name + " must be " + type + "." };
    }

    function createError3(name) {
        return { code: "401", message: "Parameter error. The " + name + " parameter is mandatory." };
    }

    function assertErrorEqual(actualErr, expectErr) {
        if (expectErr) {
            expect(actualErr.code).assertEqual(expectErr.code)
            expect(actualErr.message).assertEqual(expectErr.message)
        } else {
            expect(actualErr).assertNull();
        }
    }

    function writeTest(name, type, params, code, done) {
        hiAppEvent.write(name, type, params, (err, value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(code);
            console.info('HiAppEventJsTest writeTest end, result=' + result);
            done();
        });
    }

    function writeParamsTest(params, code, done) {
        writeTest(TEST_NAME, TEST_TYPE, params, code, done);
    }

    function writeNameTest(name, code, done) {
        writeTest(name, TEST_TYPE, TEST_PARAMS, code, done);
    }

    function writeV9Test(eventInfo, expectErr, done, hasCatch) {
        if (hasCatch) {
            try {
                hiAppEventV9.write(eventInfo, (err) => {
                    expect(err).assertNull()
                });
            } catch (err) {
                assertErrorEqual(err, expectErr);
                console.info('HiAppEventJsTest writeV9Test_catch end');
                done();
            }
        } else {
            hiAppEventV9.write(eventInfo, (err) => {
                assertErrorEqual(err, expectErr);
                console.info('HiAppEventJsTest writeV9Test end');
                done();
            });
        }
    }

    function writeParamsV9Test(params, expectErr, done, hasCatch) {
        let eventInfo = {
            domain: TEST_DOMAIN,
            name: TEST_NAME,
            eventType: TEST_TYPE_V9,
            params: params
        };
        writeV9Test(eventInfo, expectErr, done, hasCatch);
    }

    function writeDomainV9Test(domain, expectErr, done, hasCatch) {
        let eventInfo = {
            domain: domain,
            name: TEST_NAME,
            eventType: TEST_TYPE_V9,
            params: TEST_PARAMS
        };
        writeV9Test(eventInfo, expectErr, done, hasCatch);
    }

    function writeNameV9Test(name, expectErr, done, hasCatch) {
        let eventInfo = {
            domain: TEST_DOMAIN,
            name: name,
            eventType: TEST_TYPE_V9,
            params: TEST_PARAMS
        };
        writeV9Test(eventInfo, expectErr, done, hasCatch);
    }

    function writeTypeV9Test(type, expectErr, done, hasCatch) {
        let eventInfo = {
            domain: TEST_DOMAIN,
            name: TEST_NAME,
            eventType: type,
            params: TEST_PARAMS
        };
        writeV9Test(eventInfo, expectErr, done, hasCatch);
    }

    /**
     * @tc.number HiAppEventJsTest001_1
     * @tc.name: HiAppEventJsTest001_1
     * @tc.desc: Test the write interface using callback.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest001_1', 0, async function (done) {
        console.info('HiAppEventJsTest001_1 start');
        let params = {
            "key_int": 100,
            "key_string": "strValue",
            "key_bool": true,
            "key_float": 30949.374,
            "key_int_arr": [1, 2, 3],
            "key_string_arr": ["a", "b", "c"],
            "key_float_arr": [1.1, 2.2, 3.0],
            "key_bool_arr": [true, false, true]
        };
        writeParamsTest(params, 0, done);
    });

    /**
     * @tc.number HiAppEventJsTest001_2
     * @tc.name: HiAppEventJsTest001_2
     * @tc.desc: Test the write interface using callback.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest001_2', 0, async function (done) {
        console.info('HiAppEventJsTest001_2 start');
        let params = {
            "key_int": 100,
            "key_string": "strValue",
            "key_bool": true,
            "key_float": 30949.374,
            "key_int_arr": [1, 2, 3],
            "key_string_arr": ["a", "b", "c"],
            "key_float_arr": [1.1, 2.2, 3.0],
            "key_bool_arr": [true, false, true]
        };
        writeParamsV9Test(params, null, done);
    });

    /**
     * @tc.number HiAppEventJsTest002_1
     * @tc.name: HiAppEventJsTest002_1
     * @tc.desc: Test the write interface using promise.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest002_1', 0, async function (done) {
        console.info('HiAppEventJsTest002_1 start');
        hiAppEvent.write(TEST_NAME, TEST_TYPE_V9, TEST_PARAMS).then((value) => {
            let result = value;
            expect(result).assertEqual(0);
            console.info('HiAppEventJsTest002_1 succ');
            done()
        }).catch((err) => {
            expect().assertFail();
            done()
        });
    });

    /**
     * @tc.number HiAppEventJsTest002_2
     * @tc.name: HiAppEventJsTest002_2
     * @tc.desc: Test the write interface using promise.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest002_2', 0, async function (done) {
        console.info('HiAppEventJsTest002_2 start');
        let eventInfo = {
            domain: TEST_DOMAIN,
            name: TEST_NAME,
            eventType: TEST_TYPE_V9,
            params: TEST_PARAMS
        };
        hiAppEventV9.write(eventInfo).then(() => {
            console.info('HiAppEventJsTest002_2 succ');
            done();
        }).catch((err) => {
            expect().assertFail();
            done();
        });
    });

    /**
     * @tc.number HiAppEventJsTest003_1
     * @tc.name: HiAppEventJsTest003_1
     * @tc.desc: Error code 1 is returned when the event has an invalid key name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest003_1', 0, async function (done) {
        console.info('HiAppEventJsTest003_1 start');
        let params = {
            "**":"ha",
            "key_int":1,
            "HH22":"ha",
            "key_str":"str",
            "":"empty",
            "aa_":"underscore"
        };
        writeParamsTest(params, 1, done);
    });

    /**
     * @tc.number HiAppEventJsTest003_2
     * @tc.name: HiAppEventJsTest003_2
     * @tc.desc: Error code 11101005 is returned when the event has an invalid key name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest003_2', 0, async function (done) {
        console.info('HiAppEventJsTest003_2 start');
        let params = {
            "**":"ha",
            "key_int":1,
            "HH22":"ha",
            "key_str":"str",
            "":"empty",
            "aa_":"underscore"
        };
        let expectErr = createError(11101005, "Invalid event parameter name. Possible causes: 1. Contain invalid " +
            "characters; 2. Length is invalid.");
        writeParamsV9Test(params, expectErr, done);

        const MAX_LENGTH_OF_PARAM_NAME = 32;
        params = {};
        params['a'.repeat(MAX_LENGTH_OF_PARAM_NAME + 1)] = "value";
        writeParamsV9Test(params, expectErr, done);

        params = {};
        params['a'.repeat(MAX_LENGTH_OF_PARAM_NAME - 1) + "_"] = "value";
        writeParamsV9Test(params, expectErr, done);

        params = {};
        params['a'.repeat(MAX_LENGTH_OF_PARAM_NAME)] = "value";
        writeParamsV9Test(params, null, done);
    });

    /**
     * @tc.number HiAppEventJsTest004_1
     * @tc.name: HiAppEventJsTest004_1
     * @tc.desc: Error code 3 is returned when the event has an invalid value type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest004_1', 0, async function (done) {
        console.info('HiAppEventJsTest004_1 start');
        let params = {
            key_1_invalid: {},
            key_2_invalid: null,
            key_str: "str"
        };
        writeParamsTest(params, 3, done);
    });

    /**
     * @tc.number HiAppEventJsTest004_2
     * @tc.name: HiAppEventJsTest004_2
     * @tc.desc: Error code 401 is returned when the event has an invalid value type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest004_2', 0, async function (done) {
        console.info('HiAppEventJsTest004_2 start');
        let params = {
            key_1_invalid: {},
            key_2_invalid: null,
            key_str: "str"
        };
        let expectErr = createError2("param value", "boolean|number|string|array[boolean|number|string]");
        writeParamsV9Test(params, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest005_1
     * @tc.name: HiAppEventJsTest005_1
     * @tc.desc: Error code 4 is returned when the event has an invalid string length.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest005_1', 0, async function (done) {
        console.info('HiAppEventJsTest005_1 start');
        let longStr = "a".repeat(8 * 1024);
        let invalidStr = "a".repeat(8 * 1024 + 1);
        let params = {
            key_long: longStr,
            key_i_long: invalidStr,
            key_long_arr: ["ha", longStr],
            key_i_long_arr: ["ha", invalidStr],
            key_str: "str"
        };
        writeParamsTest(params, 4, done);
    });

    /**
     * @tc.number HiAppEventJsTest005_2
     * @tc.name: HiAppEventJsTest005_2
     * @tc.desc: Error code 11101004 is returned when the event has an invalid string length.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest005_2', 0, async function (done) {
        console.info('HiAppEventJsTest005_2 start');
        let longStr = "a".repeat(8 * 1024);
        let invalidStr = "a".repeat(8 * 1024 + 1);
        let params = {
            key_long: longStr,
            key_i_long: invalidStr,
            key_long_arr: ["ha", longStr],
            key_i_long_arr: ["ha", invalidStr],
            key_str: "str"
        };
        let expectErr = createError(11101004, "Invalid string length of the event parameter.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsTest006_1
     * @tc.name: HiAppEventJsTest006_1
     * @tc.desc: Error code 5 is returned when the event has too many params.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest006_1', 0, async function (done) {
        console.info('HiAppEventJsTest006_1 start');
        let params = {};
        for (var i = 1; i <= 33; i++) {
            params["key" + i] = "value" + i;
        }
        writeParamsTest(params, 5, done);
    });

    /**
     * @tc.number HiAppEventJsTest006_2
     * @tc.name: HiAppEventJsTest006_2
     * @tc.desc: Error code 11101003 is returned when the event has too many params.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest006_2', 0, async function (done) {
        console.info('HiAppEventJsTest006_2 start');
        let params = {};
        for (var i = 1; i <= 33; i++) {
            params["key" + i] = "value" + i;
        }
        let expectErr = createError(11101003, "Invalid number of event parameters. Possible caused by the number of " +
            "parameters is over 32.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsTest007_1
     * @tc.name: HiAppEventJsTest007_1
     * @tc.desc: Error code 6 is returned when there is an array with too many elements.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest007_1', 0, async function (done) {
        console.info('HiAppEventJsTest007_1 start');
        let longArr = new Array(100).fill(1);
        let iLongArr = new Array(101).fill("a");
        let params = {
            key_long_arr: longArr,
            key_i_long_arr: iLongArr,
            key_str: "str"
        };
        writeParamsTest(params, 6, done);
    });

    /**
     * @tc.number HiAppEventJsTest007_2
     * @tc.name: HiAppEventJsTest007_2
     * @tc.desc: Error code 11101006 is returned when there is an array with too many elements.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest007_2', 0, async function (done) {
        console.info('HiAppEventJsTest007_2 start');
        let longArr = new Array(100).fill(1);
        let iLongArr = new Array(101).fill("a");
        let params = {
            key_long_arr: longArr,
            key_i_long_arr: iLongArr,
            key_str: "str"
        };
        let expectErr = createError(11101006, "Invalid array length of the event parameter.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsTest008_1
     * @tc.name: HiAppEventJsTest008_1
     * @tc.desc: Error code 7 is returned when there is an array with inconsistent or illegal parameter types.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest008_1', 0, async function (done) {
        console.info('HiAppEventJsTest008_1 start');
        let params = {
            key_arr_null: [null, null],
            key_arr_obj: [{}],
            key_arr_test1:[true, "ha"],
            key_arr_test2:[123, "ha"],
            key_str: "str"
        };
        writeParamsTest(params, 7, done);
    });

    /**
     * @tc.number HiAppEventJsTest008_2
     * @tc.name: HiAppEventJsTest008_2
     * @tc.desc: Error code 401 is returned when there is an array with inconsistent or illegal parameter types.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest008_2', 0, async function (done) {
        console.info('HiAppEventJsTest008_2 start');
        let params = {
            key_arr_null: [null, null],
            key_arr_obj: [{}],
            key_arr_test1:[true, "ha"],
            key_arr_test2:[123, "ha"],
            key_str: "str"
        };
        let expectErr = createError2("param value", "boolean|number|string|array[boolean|number|string]");
        writeParamsV9Test(params, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest009_1
     * @tc.name: HiAppEventJsTest009_1
     * @tc.desc: Error code -1 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_1', 0, async function (done) {
        console.info('HiAppEventJsTest009_1 start');
        writeNameTest("verify_test_1.**1", -1, done);
    });

    /**
     * @tc.number HiAppEventJsTest009_2
     * @tc.name: HiAppEventJsTest009_2
     * @tc.desc: Error code 11101002 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_2', 0, async function (done) {
        console.info('HiAppEventJsTest009_2 start');
        let expectErr = createError(11101002, "Invalid event name. Possible causes: 1. Contain invalid characters; " +
            "2. Length is invalid.");
        writeNameV9Test("", expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsTest009_3
     * @tc.name: HiAppEventJsTest009_3
     * @tc.desc: Error code 11101002 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_3', 0, async function (done) {
        console.info('HiAppEventJsTest009_3 start');
        let expectErr = createError(11101002, "Invalid event name. Possible causes: 1. Contain invalid characters; " +
            "2. Length is invalid.");
        writeNameV9Test("VVtt_", expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsTest009_4
     * @tc.name: HiAppEventJsTest009_4
     * @tc.desc: Error code 11101002 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_4', 0, async function (done) {
        console.info('HiAppEventJsTest009_3 start');
        const MAX_LENGTH_OF_EVENT_NAME = 48;
        let expectErr = createError(11101002, "Invalid event name. Possible causes: 1. Contain invalid characters; " +
            "2. Length is invalid.");
        writeNameV9Test("a".repeat(MAX_LENGTH_OF_EVENT_NAME + 1), expectErr, done);

        writeNameV9Test("a".repeat(MAX_LENGTH_OF_EVENT_NAME - 1) + "_", expectErr, done);

        writeNameV9Test("a".repeat(MAX_LENGTH_OF_EVENT_NAME), null, done);
    });

    /**
     * @tc.number HiAppEventJsTest010_1
     * @tc.name: HiAppEventJsTest010_1
     * @tc.desc: Error code -2 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_1', 0, async function (done) {
        console.info('HiAppEventJsTest010_1 start');
        writeTest(null, TEST_TYPE, TEST_PARAMS, -2, done);
    });

    /**
     * @tc.number HiAppEventJsTest010_2
     * @tc.name: HiAppEventJsTest010_2
     * @tc.desc: Error code -2 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_1', 0, async function (done) {
        console.info('HiAppEventJsTest010_1 start');
        writeTest(TEST_NAME, "invalid", TEST_PARAMS, -2, done);
    });

    /**
     * @tc.number HiAppEventJsTest010_3
     * @tc.name: HiAppEventJsTest010_3
     * @tc.desc: Error code 401 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_3', 0, async function (done) {
        console.info('HiAppEventJsTest010_3 start');

        // invalid AppEventInfo type
        let expectErr = createError2("info", "AppEventInfo");
        writeV9Test(null, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest010_4
     * @tc.name: HiAppEventJsTest010_4
     * @tc.desc: Error code 401 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_4', 0, async function (done) {
        console.info('HiAppEventJsTest010_4 start');

        // invalid event domain type
        let expectErr = createError2("domain", "string");
        writeDomainV9Test(true, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest010_5
     * @tc.name: HiAppEventJsTest010_5
     * @tc.desc: Error code 401 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_5', 0, async function (done) {
        console.info('HiAppEventJsTest010_5 start');

        // invalid event name type
        let expectErr = createError2("name", "string");
        writeNameV9Test(null, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest010_6
     * @tc.name: HiAppEventJsTest010_6
     * @tc.desc: Error code 401 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_6', 0, async function (done) {
        console.info('HiAppEventJsTest010_6 start');

        // invalid eventType type
        let expectErr = createError2("eventType", "EventType");
        writeTypeV9Test(-1, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest010_7
     * @tc.name: HiAppEventJsTest010_7
     * @tc.desc: Error code 401 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest010_7', 0, async function (done) {
        console.info('HiAppEventJsTest010_7 start');

        // invalid event params type
        let expectErr = createError2("params", "object");
        writeParamsV9Test(null, expectErr, done, true);
    });

    /**
     * @tc.number HiAppEventJsTest011_1
     * @tc.name: HiAppEventJsTest011_1
     * @tc.desc: Error code -3 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest011_1', 0, async function (done) {
        console.info('HiAppEventJsTest011_1 start');
        hiAppEvent.write().then(() => {
            expect().assertFail();
            done();
        }).catch((err) => {
            let result = err.code;
            expect(result).assertEqual(-3);
            done();
            console.info('HiAppEventJsTest011_1 end');
        });
    });

    /**
     * @tc.number HiAppEventJsTest011_2
     * @tc.name: HiAppEventJsTest011_2
     * @tc.desc: Error code 401 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest011_2', 0, async function (done) {
        console.info('HiAppEventJsTest011_2 start');

        // AppEventInfo not passed in
        try {
            hiAppEventV9.write();
        } catch (err) {
            let expectErr = createError3("info")
            assertErrorEqual(err, expectErr)
            console.info('HiAppEventJsTest011_2 end');
        }
        done();
    });

    /**
     * @tc.number HiAppEventJsTest011_3
     * @tc.name: HiAppEventJsTest011_3
     * @tc.desc: Error code 401 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest011_3', 0, async function (done) {
        console.info('HiAppEventJsTest011_3 start');

        // event domain not passed in
        try {
            hiAppEventV9.write({
                name: TEST_NAME,
                eventType: TEST_TYPE_V9,
                params: TEST_PARAMS,
            });
        } catch (err) {
            let expectErr = createError3("domain")
            assertErrorEqual(err, expectErr)
            console.info('HiAppEventJsTest011_3 end');
        }
        done();
    });

    /**
     * @tc.number HiAppEventJsTest011_4
     * @tc.name: HiAppEventJsTest011_4
     * @tc.desc: Error code 401 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest011_4', 0, async function (done) {
        console.info('HiAppEventJsTest011_4 start');

        // event name not passed in
        try {
            hiAppEventV9.write({
                domain: TEST_DOMAIN,
                eventType: TEST_TYPE_V9,
                params: TEST_PARAMS,
            });
        } catch (err) {
            let expectErr = createError3("name")
            assertErrorEqual(err, expectErr)
            console.info('HiAppEventJsTest011_4 end');
        }
        done();
    });

    /**
     * @tc.number HiAppEventJsTest011_5
     * @tc.name: HiAppEventJsTest011_5
     * @tc.desc: Error code 401 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest011_5', 0, async function (done) {
        console.info('HiAppEventJsTest011_5 start');

        // event type not passed in
        try {
            hiAppEventV9.write({
                domain: TEST_DOMAIN,
                name: TEST_NAME,
                params: TEST_PARAMS,
            });
        } catch (err) {
            let expectErr = createError3("eventType")
            assertErrorEqual(err, expectErr)
            console.info('HiAppEventJsTest011_5 end');
        }
        done();
    });

    /**
     * @tc.number HiAppEventJsTest011_6
     * @tc.name: HiAppEventJsTest011_6
     * @tc.desc: Error code 401 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest011_6', 0, async function (done) {
        console.info('HiAppEventJsTest011_6 start');

        // event params not passed in
        try {
            hiAppEventV9.write({
                domain: TEST_DOMAIN,
                name: TEST_NAME,
                eventType: TEST_TYPE_V9,
            });
        } catch (err) {
            let expectErr = createError3("params")
            assertErrorEqual(err, expectErr)
            console.info('HiAppEventJsTest011_6 end');
        }
        done();
    });

    /**
     * @tc.number HiAppEventJsTest012
     * @tc.name: HiAppEventJsTest012
     * @tc.desc: Test event domain.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest012', 0, async function (done) {
        console.info('HiAppEventJsTest012 start');

        const MAX_LEN_OF_DOMAIN = 32;
        // Error code 11101001 is returned when the event has invalid event domain.
        let expectErr = createError(11101001, "Invalid event domain. Possible causes: 1. Contain invalid characters; " +
            "2. Length is invalid.");
        writeDomainV9Test("domain***", expectErr, done);
        writeDomainV9Test("123domain", expectErr, done);
        writeDomainV9Test("_domain", expectErr, done);
        writeDomainV9Test("domain_", expectErr, done);
        writeDomainV9Test("a".repeat(MAX_LEN_OF_DOMAIN - 1) + "_", expectErr, done);
        writeDomainV9Test("", expectErr, done);
        writeDomainV9Test("a".repeat(MAX_LEN_OF_DOMAIN + 1), expectErr, done);

        // valid event domain.
        writeDomainV9Test("a", null, done);
        writeDomainV9Test("a1", null, done);
        writeDomainV9Test("domainTest", null, done);
        writeDomainV9Test("a".repeat(MAX_LEN_OF_DOMAIN), null, done);
    });

    /**
     * @tc.number HiAppEventJsTest013
     * @tc.name: HiAppEventJsTest013
     * @tc.desc: The number of event params exceeds 32 and invalid params exist.
     * @tc.type: FUNC
     * @tc.require: issueI8GWHC
     */
     it('HiAppEventJsTest013', 0, async function (done) {
        console.info('HiAppEventJsTest013 start');
        let params = {};
        for (var i = 1; i <= 33; i++) {
            params["key" + i] = "value" + i;
        }
        let invalidKey = 'a'.repeat(17);
        params[invalidKey] = 'value_invalid';
        let expectErr = createError(11101003, "Invalid number of event parameters. Possible caused by the number of " +
            "parameters is over 32.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsTest014
     * @tc.name: HiAppEventJsTest014
     * @tc.desc: The number of event params exceeds 32 and invalid params exist.
     * @tc.type: FUNC
     * @tc.require: issueI8GWHC
     */
     it('HiAppEventJsTest014', 0, async function (done) {
        console.info('HiAppEventJsTest014 start');
        let params = {};
        params["123xxx"] = "value_invalid"; // invalid param name
        params["xxx_"] = "value_invalid"; // invalid param name
        for (var i = 1; i <= 33; i++) {
            params["key" + i] = "value" + i;
        }
        params['a'.repeat(33)] = 'value_invalid'; // invalid param name
        let expectErr = createError(11101003, "Invalid number of event parameters. Possible caused by the number of " +
            "parameters is over 32.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
     * @tc.number HiAppEventJsPresetTest001_1
     * @tc.name: HiAppEventJsPresetTest001_1
     * @tc.desc: Test preset events and preset parameters.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsPresetTest001_1', 0, async function (done) {
        console.info('HiAppEventJsPresetTest001_1 start');
        writeTest(hiAppEvent.Event.USER_LOGIN, hiAppEvent.EventType.FAULT, {
            [hiAppEvent.Param.USER_ID]: "123456"
        }, 0, done);
    });

    /**
     * @tc.number HiAppEventJsPresetTest001_2
     * @tc.name: HiAppEventJsPresetTest001_2
     * @tc.desc: Test preset events and preset parameters.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
     it('HiAppEventJsPresetTest001_2', 0, async function (done) {
        console.info('HiAppEventJsPresetTest001_2 start');
        writeTest(hiAppEvent.Event.USER_LOGOUT, hiAppEvent.EventType.STATISTIC, {
            [hiAppEvent.Param.USER_ID]: "123456"
        }, 0, done);
    });

    /**
     * @tc.number HiAppEventJsPresetTest001_3
     * @tc.name: HiAppEventJsPresetTest001_3
     * @tc.desc: Test preset events and preset parameters.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
     it('HiAppEventJsPresetTest001_3', 0, async function (done) {
        console.info('HiAppEventJsPresetTest001_3 start');
        let eventInfo = {
            domain: TEST_DOMAIN,
            name: hiAppEventV9.event.DISTRIBUTED_SERVICE_START,
            eventType: hiAppEventV9.EventType.SECURITY,
            params: {
                [hiAppEventV9.param.DISTRIBUTED_SERVICE_NAME]: "test_service",
                [hiAppEventV9.param.DISTRIBUTED_SERVICE_INSTANCE_ID]: "123",
            },
        };
        writeV9Test(eventInfo, null, done);
    });

    /**
     * @tc.number HiAppEventConfigureTest001_1
     * @tc.name: HiAppEventConfigureTest001_1
     * @tc.desc: Error code -99 is returned when the logging function is disabled.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventConfigureTest001_1', 0, async function (done) {
        console.info('HiAppEventConfigureTest001_1 start');
        let res = hiAppEvent.configure({
            disable: true
        });
        expect(res).assertTrue();

        writeNameTest("config_test", -99, done);
    });

    /**
     * @tc.number HiAppEventConfigureTest001_2
     * @tc.name: HiAppEventConfigureTest001_2
     * @tc.desc: Error code 11100001 is returned when the logging function is disabled.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
     it('HiAppEventConfigureTest001_2', 0, async function (done) {
        console.info('HiAppEventConfigureTest001_2 start');
        hiAppEventV9.configure({
            disable: true
        });

        let expectErr = createError(11100001, "Function disabled. Possible caused by the param disable in " +
            "ConfigOption is true.");
        writeNameV9Test("config_test", expectErr, done);
    });

    /**
     * @tc.number HiAppEventConfigureTest002
     * @tc.name: HiAppEventConfigureTest002
     * @tc.desc: Correctly configure the event logging function.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventConfigureTest002', 0, function () {
        console.info('HiAppEventConfigureTest002 start');
        let result = false;
        result = hiAppEvent.configure({
            disable: false,
            maxStorage: "10G"
        });
        expect(result).assertTrue()

        try {
            hiAppEventV9.configure({
                disable: true,
                maxStorage: "100m"
            });
            hiAppEventV9.configure({
                disable: false,
                maxStorage: "10M"
            });
        } catch (err) {
            expect().assertFail();
        }

        console.info('HiAppEventConfigureTest002 end');
    });

    /**
     * @tc.number HiAppEventConfigureTest003
     * @tc.name: HiAppEventConfigureTest003
     * @tc.desc: Incorrectly configure the event logging function.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventConfigureTest003', 0, function () {
        console.info('HiAppEventConfigureTest003 start');
        let result = true;

        result = hiAppEvent.configure({
            disable: false,
            maxStorage: "xxx"
        })
        expect(result).assertFalse()

        result = hiAppEvent.configure(null)
        expect(result).assertFalse()

        result = hiAppEvent.configure({
            disable: null,
            maxStorage: {}
        })
        expect(result).assertFalse()

        // ConfigOption not passed in
        try {
            hiAppEventV9.configure();
        } catch (err) {
            let expectErr = createError3("config")
            assertErrorEqual(err, expectErr)
        }

        // invalid ConfigOption type
        function configureTest(configOption, expectErr) {
            try {
                hiAppEventV9.configure(configOption);
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        let expectErr = createError2("config", "ConfigOption")
        configureTest(null, expectErr)
        configureTest([], expectErr)

        // invalid ConfigOption.disable type
        expectErr = createError2("disable", "boolean")
        configureTest({ disable: 123 }, expectErr)

        // invalid ConfigOption.maxStorage type
        expectErr = createError2("maxStorage", "string")
        configureTest({ maxStorage: null }, expectErr)

        // invalid ConfigOption.maxStorage value
        expectErr = createError(11103001, "Invalid max storage quota value. Possible caused by incorrectly formatted.")
        configureTest({ maxStorage: "**22" }, expectErr)

        console.info('HiAppEventConfigureTest003 end');
    });

    /**
     * @tc.number HiAppEventClearTest001
     * @tc.name: HiAppEventClearTest001
     * @tc.desc: clear the local data.
     * @tc.type: FUNC
     * @tc.require: issueI5NTOS
     */
    it('HiAppEventClearTest001', 0, async function (done) {
        console.info('HiAppEventClearTest001 start');

        // 1. clear data
        let result = hiAppEventV9.clearData();
        expect(result).assertUndefined();

        // 2. write event after clear data
        writeNameV9Test("clear_test", null, done);
    });
});