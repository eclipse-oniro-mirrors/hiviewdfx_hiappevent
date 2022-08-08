/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

    /**
     * @tc.name: HiAppEventJsTest001
     * @tc.desc: Test the write interface using callback.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest001', 0, async function (done) {
        console.info('HiAppEventJsTest001 start');
        let name = "name_test1";
        let type = hiAppEvent.EventType.BEHAVIOR;
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
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(0);
            done();
            console.info('HiAppEventJsTest001_1 end');
        });

        let domain = "domain_test1";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(0);
            done();
            console.info('HiAppEventJsTest001_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest002
     * @tc.desc: Test the write interface using promise.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest002', 0, async function (done) {
        console.info('HiAppEventJsTest002 start');
        let name = "name_test2";
        let type = hiAppEvent.EventType.FAULT;
        let params = {};
        hiAppEvent.write(name, type, params).then((value) => {
            let result = value;
            expect(result).assertEqual(0);
            done()
            console.info('HiAppEventJsTest002_1 end');
        }).catch((err) => {
            let result = err.code;
            expect(result).assertEqual(0);
            done()
            console.info('HiAppEventJsTest002_2 end');
        });


        let domain = "domain_test2";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo).then((value) => {
            let result = value;
            expect(result).assertEqual(0);
            done()
            console.info('HiAppEventJsTest002_3 end');
        }).catch((err) => {
            let result = err.code;
            expect(result).assertEqual(0);
            done()
            console.info('HiAppEventJsTest002_4 end');
        });
        expect(result).assertEqual(0);
    });

    /**
     * @tc.name: HiAppEventJsTest003
     * @tc.desc: Error code 1 is returned when the event has an invalid key name.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest003', 0, async function (done) {
        console.info('HiAppEventJsTest003 start');
        let name = "name_test3";
        let type = hiAppEvent.EventType.STATISTIC;
        let params = {
            "**":"ha",
            "key_int":1,
            "HH22":"ha",
            "key_str":"str",
            "":"empty",
            "aa_":"underscore"
        };
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(1)
            done()
            console.info('HiAppEventJsTest003_1 end');
        });

        let domain = "domain_test3";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(1)
            done()
            console.info('HiAppEventJsTest003_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest004
     * @tc.desc: Error code 3 is returned when the event has an invalid value type.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest004', 0, async function (done) {
        console.info('HiAppEventJsTest004 start');
        let name = "name_test4";
        let type = hiAppEvent.EventType.SECURITY;
        let params = {
            key_1_invalid: {},
            key_2_invalid: null,
            key_str: "str"
        };
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(3)
            done()
            console.info('HiAppEventJsTest004_1 end');
        });

        let domain = "domain_test4";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(3)
            done()
            console.info('HiAppEventJsTest004_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest005
     * @tc.desc: Error code 4 is returned when the event has an invalid string length.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest005', 0, async function (done) {
        console.info('HiAppEventJsTest005 start');
        let longStr = "a".repeat(8 * 1024);
        let invalidStr = "a".repeat(8 * 1024 + 1);
        let name = "name_test5";
        let type = hiAppEvent.EventType.SECURITY;
        let params = {
            key_long: longStr,
            key_i_long: invalidStr,
            key_long_arr: ["ha", longStr],
            key_i_long_arr: ["ha", invalidStr],
            key_str: "str"
        };
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(4)
            done()
            console.info('HiAppEventJsTest005_1 end');
        });

        let domain = "domain_test5";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(4)
            done()
            console.info('HiAppEventJsTest005_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest006
     * @tc.desc: Error code 5 is returned when the event has too many params.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest006', 0, async function (done) {
        console.info('HiAppEventJsTest006 start');
        let name = "name_test6";
        let type = hiAppEvent.EventType.SECURITY;
        let params = {};
        for (var i = 1; i <= 33; i++) {
            params["key" + i] = "value" + i;
        }
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(5)
            done()
            console.info('HiAppEventJsTest006_1 end');
        });

        let domain = "domain_test6";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(5)
            done()
            console.info('HiAppEventJsTest006_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest007
     * @tc.desc: Error code 6 is returned when there is an array with too many elements.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest007', 0, async function (done) {
        console.info('HiAppEventJsTest007 start');
        let longArr = new Array(100).fill(1);
        let iLongArr = new Array(101).fill("a");
        let name = "name_test7";
        let type = hiAppEvent.EventType.SECURITY;
        let params = {
            key_long_arr: longArr,
            key_i_long_arr: iLongArr,
            key_str: "str"
        };
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(6)
            done()
            console.info('HiAppEventJsTest007_1 end');
        });

        let domain = "domain_test7";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(6)
            done()
            console.info('HiAppEventJsTest007_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest008
     * @tc.desc: Error code 7 is returned when there is an array with inconsistent or illegal parameter types.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest008', 0, async function (done) {
        console.info('HiAppEventJsTest008 start');
        let name = "name_test8";
        let type = hiAppEvent.EventType.SECURITY;
        let params = {
            key_arr_null: [null, null],
            key_arr_obj: [{}],
            key_arr_not_same1:[true, "ha"],
            key_arr_not_same2:[123, "ha"],
            key_str: "str"
        };
        hiAppEvent.write(name, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(7)
            done()
            console.info('HiAppEventJsTest008_1 end');
        });

        let domain = "domain_test8";
        let eventInfo = {
            domain: domain,
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(7)
            done()
            console.info('HiAppEventJsTest008_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest009
     * @tc.desc: Error code -1 is returned when the event has invalid event name.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest009', 0, async function (done) {
        console.info('HiAppEventJsTest009 start');
        let type = hiAppEvent.EventType.STATISTIC;
        let params = {};
        hiAppEvent.write("verify_test_1.**1", type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-1)
            done()
            console.info('HiAppEventJsTest009_1 end');
        });

        hiAppEvent.write("VVV", type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-1)
            done()
            console.info('HiAppEventJsTest009_2 end');
        });

        let eventInfo = {
            domain: "domain_test9",
            name: "",
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-1)
            done()
            console.info('HiAppEventJsTest009_3 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest010
     * @tc.desc: Error code -2 is returned when the event has invalid eventName type, eventType type, keyValues type.
     * @tc.type: FUNC
     */
    it('HiAppEventJsTest010', 0, async function (done) {
        console.info('HiAppEventJsTest010 start');
        let type = hiAppEvent.EventType.STATISTIC;
        let params = {};
        hiAppEvent.write(null, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-2)
            done()
            console.info('HiAppEventJsTest010_1 end');
        });
        hiAppEvent.write(123, type, params, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-2)
            done()
            console.info('HiAppEventJsTest010_2 end');
        });

        let domain = "domain_test10";
        let name = "name_test10";
        let eventInfo1 = {
            domain: domain,
            name: name,
            eventType: "invalid type",
            params: params
        };
        hiAppEvent.write(eventInfo1, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-2)
            done()
            console.info('HiAppEventJsTest010_3 end');
        });

        let eventInfo2 = {
            domain: domain,
            name: name,
            eventType: null,
            params: params
        };
        hiAppEvent.write(eventInfo2, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-2)
            done()
            console.info('HiAppEventJsTest010_4 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest011
     * @tc.desc: Error code -3 is returned when the event has invalid num of args.
     * @tc.type: FUNC
     */
     it('HiAppEventJsTest011', 0, async function (done) {
        console.info('HiAppEventJsTest011 start');
        hiAppEvent.write().then((value) => {
            let result = value;
            expect(result).assertEqual(-3);
            done()
            console.info('HiAppEventJsTest011_1 end');
        }).catch((err) => {
            let result = err.code;
            expect(result).assertEqual(-3);
            done()
            console.info('HiAppEventJsTest011_2 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsTest012
     * @tc.desc: Error code -4 is returned when the event has invalid event domain.
     * @tc.type: FUNC
     */
     it('HiAppEventJsTest012', 0, async function (done) {
        console.info('HiAppEventJsTest012 start');
        let name = "domain_test12";
        let type = hiAppEvent.EventType.STATISTIC;
        let params = {};
        let eventInfo1 = {
            domain: "domain***",
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo1, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-4)
            done()
            console.info('HiAppEventJsTest012_1 end');
        });

        let eventInfo2 = {
            domain: "domainTest",
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo2, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-4)
            done()
            console.info('HiAppEventJsTest012_2 end');
        });

        let eventInfo3 = {
            domain: "",
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo3, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-4)
            done()
            console.info('HiAppEventJsTest012_3 end');
        });

        let eventInfo4 = {
            domain: "a".repeat(17),
            name: name,
            eventType: type,
            params: params
        };
        hiAppEvent.write(eventInfo4, (err ,value) => {
            let result = err ? err.code : value;
            expect(result).assertEqual(-4)
            done()
            console.info('HiAppEventJsTest012_4 end');
        });
    });

    /**
     * @tc.name: HiAppEventJsPresetTest001
     * @tc.desc: Test preset events and preset parameters.
     * @tc.type: FUNC
     */
    it('HiAppEventJsPresetTest001', 0, async function (done) {
        console.info('HiAppEventJsPresetTest001 start');
        hiAppEvent.write(hiAppEvent.Event.USER_LOGIN, hiAppEvent.EventType.FAULT,
            {
                [hiAppEvent.Param.USER_ID]:"123456"
            },
            (err ,value) => {
                let result = err ? err.code : value;
                expect(result).assertEqual(0)
                done()
                console.info('HiAppEventJsPresetTest001_1 end');
            }
        );

        hiAppEvent.write(hiAppEvent.Event.USER_LOGOUT, hiAppEvent.EventType.STATISTIC,
            {
                [hiAppEvent.Param.USER_ID]:"123456"
            },
            (err ,value) => {
                let result = err ? err.code : value;
                expect(result).assertEqual(0)
                done()
                console.info('HiAppEventJsPresetTest001_2 end');
            }
        );

        hiAppEvent.write(hiAppEvent.Event.DISTRIBUTED_SERVICE_START, hiAppEvent.EventType.SECURITY,
            {
                [hiAppEvent.Param.DISTRIBUTED_SERVICE_NAME]:"test_service",
                [hiAppEvent.Param.DISTRIBUTED_SERVICE_INSTANCE_ID]:"123",
            },
            (err ,value) => {
                let result = err ? err.code : value;
                expect(result).assertEqual(0)
                done()
                console.info('HiAppEventJsPresetTest001_3 end');
            }
        );
        console.info('HiAppEventJsPresetTest001 end');
    });

    /**
     * @tc.name: HiAppEventConfigureTest001
     * @tc.desc: Error code -99 is returned when the logging function is disabled.
     * @tc.type: FUNC
     */
    it('HiAppEventConfigureTest001', 0, async function (done) {
        console.info('HiAppEventConfigureTest001 start');
        let res = hiAppEvent.configure({
            disable: true
        });
        expect(res).assertTrue();

        hiAppEvent.write("base_test13", hiAppEvent.EventType.SECURITY,
            {
                "key_str": "str",
            },
            (err ,value) => {
                let result = err ? err.code : value;
                expect(result).assertEqual(-99)
                done()
                console.info('HiAppEventConfigureTest001 end');
            }
        );
    });

    /**
     * @tc.name: HiAppEventConfigureTest002
     * @tc.desc: Correctly configure the event logging function.
     * @tc.type: FUNC
     */
    it('HiAppEventConfigureTest002', 0, function () {
        console.info('HiAppEventConfigureTest002 start');
        let result = false;

        result = hiAppEvent.configure({
            disable: true,
            maxStorage: "100m"
        });
        expect(result).assertTrue()

        result = hiAppEvent.configure({
            disable: false,
            maxStorage: "10G"
        });
        expect(result).assertTrue()

        result = hiAppEvent.configure({
            disable: false,
            maxStorage: "10M"
        });
        expect(result).assertTrue()

        console.info('HiAppEventConfigureTest002 end');
    });

    /**
     * @tc.name: HiAppEventConfigureTest003
     * @tc.desc: Incorrectly configure the event logging function.
     * @tc.type: FUNC
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

        console.info('HiAppEventConfigureTest003 end');
    });

    /**
     * @tc.name: HiAppEventClearTest001
     * @tc.desc: clear the local data.
     * @tc.type: FUNC
     */
     it('HiAppEventClearTest001', 0, async function (done) {
        console.info('HiAppEventClearTest001 start');

        // 1. clear data
        let result = hiAppEvent.clearData();
        expect(result).assertEqual(undefined);

        // 2. write event after clear data
        let eventInfo = {
            domain: "test_domain",
            name: "clear_test1",
            eventType: hiAppEvent.EventType.FAULT,
            params: {}
        };
        hiAppEvent.write(eventInfo, (err ,value) => {
            let res = err ? err.code : value;
            expect(res).assertEqual(0);
            done();
        });

        console.info('HiAppEventClearTest001 end');
    });
});