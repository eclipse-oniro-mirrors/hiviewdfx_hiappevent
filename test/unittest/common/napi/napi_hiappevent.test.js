/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

    function simpleTrigger(curRow, curSize, holder) {
        console.info("HiAppEventJsTest onTrigger curRow=" + curRow);
        console.info("HiAppEventJsTest onTrigger curSize=" + curSize);
        if (holder == null) {
            console.info("HiAppEventJsTest onTrigger holder is null");
        }
    }

    function simpleWriteV9Test() {
        hiAppEventV9.write({
            domain: "test_domain",
            name: "test_name",
            eventType: hiAppEventV9.EventType.FAULT,
            params: {}
        }, (err) => {
            expect(err).assertNull()
        });
    }

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
        let expectErr = createError(11101005, "Invalid event parameter name.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
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
        let expectErr = createError(11101003, "Invalid number of event parameters.");
        writeParamsV9Test(params, expectErr, done);
    });

    /**
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
            key_arr_not_same1:[true, "ha"],
            key_arr_not_same2:[123, "ha"],
            key_str: "str"
        };
        writeParamsTest(params, 7, done);
    });

    /**
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
            key_arr_not_same1:[true, "ha"],
            key_arr_not_same2:[123, "ha"],
            key_str: "str"
        };
        let expectErr = createError2("param value", "boolean|number|string|array[boolean|number|string]");
        writeParamsV9Test(params, expectErr, done, true);
    });

    /**
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
     * @tc.name: HiAppEventJsTest009_2
     * @tc.desc: Error code 11101002 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_2', 0, async function (done) {
        console.info('HiAppEventJsTest009_2 start');
        let expectErr = createError(11101002, "Invalid event name.");
        writeNameV9Test("", expectErr, done);
    });

    /**
     * @tc.name: HiAppEventJsTest009_3
     * @tc.desc: Error code 11101002 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_3', 0, async function (done) {
        console.info('HiAppEventJsTest009_3 start');
        let expectErr = createError(11101002, "Invalid event name.");
        writeNameV9Test("VVtt", expectErr, done);
    });

    /**
     * @tc.name: HiAppEventJsTest009_4
     * @tc.desc: Error code 11101002 is returned when the event has invalid event name.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest009_4', 0, async function (done) {
        console.info('HiAppEventJsTest009_3 start');
        let expectErr = createError(11101002, "Invalid event name.");
        writeNameV9Test("a".repeat(49), expectErr, done);
    });

    /**
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
     * @tc.name: HiAppEventJsTest012_1
     * @tc.desc: Error code 11101001 is returned when the event has invalid event domain.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest012_1', 0, async function (done) {
        console.info('HiAppEventJsTest012_1 start');
        let expectErr = createError(11101001, "Invalid event domain.");
        writeDomainV9Test("domain***", expectErr, done);
    });

    /**
     * @tc.name: HiAppEventJsTest012_2
     * @tc.desc: Error code 11101001 is returned when the event has invalid event domain.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest012_2', 0, async function (done) {
        console.info('HiAppEventJsTest012_2 start');
        let expectErr = createError(11101001, "Invalid event domain.");
        writeDomainV9Test("domainTest", expectErr, done);
    });

    /**
     * @tc.name: HiAppEventJsTest012_3
     * @tc.desc: Error code 11101001 is returned when the event has invalid event domain.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest012_3', 0, async function (done) {
        console.info('HiAppEventJsTest012_3 start');
        let expectErr = createError(11101001, "Invalid event domain.");
        writeDomainV9Test("", expectErr, done);
    });

    /**
     * @tc.name: HiAppEventJsTest012_4
     * @tc.desc: Error code 11101001 is returned when the event has invalid event domain.
     * @tc.type: FUNC
     * @tc.require: issueI4BY0R
     */
    it('HiAppEventJsTest012_4', 0, async function (done) {
        console.info('HiAppEventJsTest012_4 start');
        let expectErr = createError(11101001, "Invalid event domain.");
        writeDomainV9Test("a".repeat(17), expectErr, done);
    });

    /**
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

        let expectErr = createError(11100001, "Function is disabled.");
        writeNameV9Test("config_test", expectErr, done);
    });

    /**
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
        expectErr = createError(11103001, "Invalid max storage quota value.")
        configureTest({ maxStorage: "**22" }, expectErr)

        console.info('HiAppEventConfigureTest003 end');
    });

    /**
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

    /**
     * @tc.name: HiAppEventWatcherTest001
     * @tc.desc: invalid watcher type.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest001', 0, function () {
        console.info('HiAppEventWatcherTest001 start');

        // watcher not passed in
        let expectErr = createError3("watcher")
        try {
            hiAppEventV9.addWatcher();
        } catch (err) {
            assertErrorEqual(err, expectErr)
        }
        try {
            hiAppEventV9.removeWatcher();
        } catch (err) {
            assertErrorEqual(err, expectErr)
        }

        // invalid watcher type
        expectErr = createError2("watcher", "Watcher")
        function addWatcherTypeTest(watcher) {
            try {
                hiAppEventV9.addWatcher(watcher);
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        function removeWatcherTypeTest(watcher) {
            try {
                hiAppEventV9.removeWatcher(watcher);
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        addWatcherTypeTest(null);
        addWatcherTypeTest(123);
        removeWatcherTypeTest(null);
        removeWatcherTypeTest(123);

        console.info('HiAppEventWatcherTest001 end');
    });

    /**
     * @tc.name: HiAppEventWatcherTest002
     * @tc.desc: invalid watcher name.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest002', 0, function () {
        console.info('HiAppEventWatcherTest002 start');

        // watcher name not passed in
        try {
            hiAppEventV9.addWatcher({});
        } catch (err) {
            let expectErr = createError3("name")
            assertErrorEqual(err, expectErr)
        }

        // invalid watcher name type
        function watcherNameTest(name, expectErr) {
            try {
                hiAppEventV9.addWatcher({
                    name: name
                });
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        let expectErr = createError2("name", "string");
        watcherNameTest(null, expectErr);
        watcherNameTest(true, expectErr);
        watcherNameTest(123, expectErr);

        // invalid watcher name value
        expectErr = createError(11102001, "Invalid watcher name.")
        watcherNameTest("a".repeat(33), expectErr);
        watcherNameTest("", expectErr);
        watcherNameTest("domain_***", expectErr);
        watcherNameTest("Domain_test", expectErr);
        watcherNameTest("_domain_test", expectErr);

        console.info('HiAppEventWatcherTest002 end');
    });

    /**
     * @tc.name: HiAppEventWatcherTest003
     * @tc.desc: invalid watcher trigger condition.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest003', 0, function () {
        console.info('HiAppEventWatcherTest003 start');

        // invalid triggerCondition type
        function triggerConditionTest(condition, expectErr) {
            try {
                hiAppEventV9.addWatcher({
                    name: "watcher",
                    triggerCondition: condition
                });
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        let expectErr = createError2("triggerCondition", "TriggerCondition")
        triggerConditionTest(null, expectErr);
        triggerConditionTest(123, expectErr);

        // invalid triggerCondition.row type
        function rowTest(row, expectErr) {
            triggerConditionTest({ row: row }, expectErr);
        }
        expectErr = createError2("row", "number")
        rowTest(null, expectErr)
        rowTest("str", expectErr)

        // invalid triggerCondition.row value
        expectErr = createError(11102003, "Invalid row value.")
        rowTest(-1, expectErr)
        rowTest(-100, expectErr)

        // invalid triggerCondition.size type
        function sizeTest(size, expectErr) {
            triggerConditionTest({ size: size }, expectErr);
        }
        expectErr = createError2("size", "number")
        sizeTest(null, expectErr)
        sizeTest(true, expectErr)

        // invalid triggerCondition.size value
        expectErr = createError(11102004, "Invalid size value.")
        sizeTest(-1, expectErr)
        sizeTest(-100, expectErr)

        // invalid triggerCondition.timeout type
        function timeoutTest(timeout) {
            triggerConditionTest({ timeout: timeout }, expectErr);
        }
        expectErr = createError2("timeout", "number")
        timeoutTest(null, expectErr)
        timeoutTest({}, expectErr)

        // invalid triggerCondition.timeout value
        expectErr = createError(11102005, "Invalid timeout value.")
        timeoutTest(-1, expectErr)
        timeoutTest(-100, expectErr)

        console.info('HiAppEventWatcherTest003 end');
    });

    /**
     * @tc.name: HiAppEventWatcherTest004
     * @tc.desc: invalid watcher filters.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest004', 0, function () {
        console.info('HiAppEventWatcherTest004 start');

        // invalid appEventFilters type
        function appEventFiltersTest(filters, expectErr) {
            try {
                hiAppEventV9.addWatcher({
                    name: "watcher",
                    appEventFilters: filters
                });
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        let expectErr = createError2("appEventFilters", "AppEventFilter[]")
        appEventFiltersTest(null, expectErr)
        appEventFiltersTest({}, expectErr)
        appEventFiltersTest("invalid", expectErr)
        appEventFiltersTest([1, 2], expectErr)
        appEventFiltersTest(["str1", "str2"], expectErr)

        // appEventFilter.domain not passed in
        function appEventFilterTest(filter, expectErr) {
            appEventFiltersTest([filter], expectErr)
        }
        expectErr = createError3("domain")
        appEventFilterTest({}, expectErr)

        // invalid appEventFilter.domain type
        function domainTest(domain, expectErr) {
            appEventFilterTest({ domain: domain }, expectErr)
        }
        expectErr = createError2("domain", "string")
        domainTest(null, expectErr)
        domainTest(123, expectErr)

        // invalid appEventFilter.domain value
        expectErr = createError(11102002, "Invalid filtering event domain.")
        domainTest("**xx", expectErr)
        domainTest("123test", expectErr)
        domainTest("a".repeat(17), expectErr)
        domainTest("", expectErr)

        // invalid appEventFilter.eventTypes type
        function eventTypesTest(eventTypes, expectErr) {
            appEventFilterTest({
                domain: "test_domain",
                eventTypes: eventTypes
            }, expectErr)
        }
        expectErr = createError2("eventTypes", "EventType[]")
        eventTypesTest(null, expectErr)
        eventTypesTest("invalid", expectErr)
        eventTypesTest(["invalid"], expectErr)
        eventTypesTest([10, -1], expectErr)

        console.info('HiAppEventWatcherTest004 end');
    });

    /**
     * @tc.name: HiAppEventWatcherTest005
     * @tc.desc: invalid watcher onTrigger.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest005', 0, function () {
        console.info('HiAppEventWatcherTest005 start');

        function onTriggerTest(onTrigger, expectErr) {
            try {
                hiAppEventV9.addWatcher({
                    name: "watcher",
                    onTrigger: onTrigger
                });
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        let expectErr = createError2("onTrigger", "function")
        onTriggerTest(null, expectErr)
        onTriggerTest("invalid", expectErr)

        console.info('HiAppEventWatcherTest005 end');
    });

    /**
     * @tc.name: HiAppEventWatcherTest006
     * @tc.desc: add valid watcher.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest006', 0, function () {
        console.info('HiAppEventWatcherTest006 start');
        let result = true;
        let watcher1 = {
            name: "watcher1",
        };
        result = hiAppEventV9.addWatcher(watcher1);
        expect(result != null).assertTrue()

        let watcher2 = {
            name: "watcher2",
            triggerCondition: {}
        };
        result = hiAppEventV9.addWatcher(watcher2);
        expect(result != null).assertTrue()

        let watcher3 = {
            name: "watcher3",
            triggerCondition: {
                row: 5
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher3);
        expect(result != null).assertTrue()

        let watcher4 = {
            name: "watcher4",
            triggerCondition: {
                size: 1000
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher4);
        expect(result != null).assertTrue()

        let watcher5 = {
            name: "watcher5",
            triggerCondition: {
                timeOut: 2
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher5);
        expect(result != null).assertTrue()

        let watcher6 = {
            name: "watcher6",
            triggerCondition: {
                row: 5,
                size: 1000,
                timeOut: 2
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher6);
        expect(result != null).assertTrue()

        let watcher7 = {
            name: "watcher7",
            appEventFilters: []
        };
        result = hiAppEventV9.addWatcher(watcher7);
        expect(result != null).assertTrue()

        let watcher8 = {
            name: "watcher8",
            appEventFilters: [
                {domain: "domain_test", eventTypes: []},
                {domain: "default", eventTypes: [hiAppEventV9.EventType.FAULT, hiAppEventV9.EventType.BEHAVIOR]},
            ]
        };
        result = hiAppEventV9.addWatcher(watcher8);
        expect(result != null).assertTrue()

        hiAppEventV9.removeWatcher(watcher1);
        hiAppEventV9.removeWatcher(watcher2);
        hiAppEventV9.removeWatcher(watcher3);
        hiAppEventV9.removeWatcher(watcher4);
        hiAppEventV9.removeWatcher(watcher5);
        hiAppEventV9.removeWatcher(watcher6);
        hiAppEventV9.removeWatcher(watcher7);
        hiAppEventV9.removeWatcher(watcher8);
        console.info('HiAppEventWatcherTest006 end');
    });

    /**
     * @tc.name: HiAppEventWatcherTest007
     * @tc.desc: watcher.onTrigger row test.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest007', 0, async function (done) {
        console.info('HiAppEventWatcherTest007 start');
        let watcher = {
            name: "watcher",
            triggerCondition: {
                row: 1
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest007.onTrigger start');
                expect(curRow).assertEqual(1)
                expect(curSize > 0).assertTrue()
                expect(holder != null).assertTrue()

                let eventPkg = holder.takeNext();
                expect(eventPkg != null).assertTrue()
                expect(eventPkg.packageId).assertEqual(0)
                expect(eventPkg.row).assertEqual(1)
                expect(eventPkg.size > 0).assertTrue()
                expect(eventPkg.data.length).assertEqual(1)
                expect(eventPkg.data[0].length > 0).assertTrue()
                console.info('HiAppEventWatcherTest007.onTrigger end');
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue()

        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            console.info('HiAppEventWatcherTest007 end');
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest008
     * @tc.desc: watcher.onTrigger size test.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest008', 0, async function (done) {
        console.info('HiAppEventWatcherTest008 start');
        let watcher = {
            name: "watcher",
            triggerCondition: {
                row: 10,
                size: 200,
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest008.onTrigger start');
                expect(curRow).assertEqual(2)
                expect(curSize >= 200).assertTrue()
                expect(holder != null).assertTrue()

                let eventPkg = holder.takeNext();
                expect(eventPkg != null).assertTrue()
                expect(eventPkg.packageId).assertEqual(0)
                expect(eventPkg.row).assertEqual(2)
                expect(eventPkg.size >= 200).assertTrue()
                expect(eventPkg.data.length).assertEqual(2)
                expect(eventPkg.data[0].length > 0).assertTrue()
                expect(eventPkg.data[1].length > 0).assertTrue()
                console.info('HiAppEventWatcherTest008.onTrigger end');
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue()

        simpleWriteV9Test();
        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            console.info('HiAppEventWatcherTest008 end');
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest009
     * @tc.desc: watcher.onTrigger timeout test.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest009', 0, async function (done) {
        console.info('HiAppEventWatcherTest009 start');
        let watcher = {
            name: "watcher",
            triggerCondition: {
                timeOut: 1
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest009.onTrigger start');
                expect(curRow).assertEqual(1)
                expect(curSize > 0).assertTrue()
                expect(holder != null).assertTrue()

                let eventPkg = holder.takeNext();
                expect(eventPkg != null).assertTrue()
                expect(eventPkg.packageId).assertEqual(0)
                expect(eventPkg.row).assertEqual(1)
                expect(eventPkg.size > 0).assertTrue()
                expect(eventPkg.data.length).assertEqual(1)
                expect(eventPkg.data[0].length > 0).assertTrue()
                console.info('HiAppEventWatcherTest009.onTrigger end');
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue()

        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            console.info('HiAppEventWatcherTest009 end');
            done();
        }, 3000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest010
     * @tc.desc: watcher.holder test.
     * @tc.type: FUNC
     * @tc.require: issueI5NTOD
     */
    it('HiAppEventWatcherTest010', 0, async function (done) {
        console.info('HiAppEventWatcherTest010 start');
        let watcher = {
            name: "watcher",
        };
        let holder = hiAppEventV9.addWatcher(watcher);
        expect(holder != null).assertTrue()

        simpleWriteV9Test();

        setTimeout(() => {
            let eventPkg = holder.takeNext();
            expect(eventPkg != null).assertTrue();
            expect(eventPkg.packageId).assertEqual(0);
            expect(eventPkg.row).assertEqual(1);
            expect(eventPkg.size > 0).assertTrue();
            expect(eventPkg.data.length).assertEqual(1);
            expect(eventPkg.data[0].length > 0).assertTrue();
            hiAppEventV9.removeWatcher(watcher);
            console.info('HiAppEventWatcherTest010 end');
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest011
     * @tc.desc: invalid watcher.holder test.
     * @tc.type: FUNC
     * @tc.require: issueI5NTOD
     */
    it('HiAppEventWatcherTest011', 0, function () {
        console.info('HiAppEventWatcherTest011 start');
        let watcher = {
            name: "watcher",
        };
        let holder = hiAppEventV9.addWatcher(watcher);
        expect(holder != null).assertTrue()

        // size not passed in
        try {
            holder.setSize()
        } catch (err) {
            let expectErr = createError3("size");
            assertErrorEqual(err, expectErr)
        }

        // invalid size type
        function holderSetSizeTest(holder, size, expectErr) {
            try {
                holder.setSize(size)
            } catch (err) {
                assertErrorEqual(err, expectErr)
            }
        }
        let expectErr = createError2("size", "number");
        holderSetSizeTest(holder, null, expectErr);
        holderSetSizeTest(holder, {}, expectErr);

        // invalid size value
        expectErr = createError(11104001, "Invalid size value.");
        holderSetSizeTest(holder, -1, expectErr);
        holderSetSizeTest(holder, -100, expectErr);

        hiAppEventV9.removeWatcher(watcher)
        console.info('HiAppEventWatcherTest011 end')
    });
});