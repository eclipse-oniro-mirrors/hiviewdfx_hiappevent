/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

    function simpleTrigger(curRow, curSize, holder) {
        console.info("HiAppEventJsTest onTrigger curRow=" + curRow);
        console.info("HiAppEventJsTest onTrigger curSize=" + curSize);
        if (holder == null) {
            console.info("HiAppEventJsTest onTrigger holder is null");
        }
    }

    function simpleReceive(domain, eventArray) {
        console.info('HiAppEventWatcherTest.onReceive start domain=' + domain + ', event size=' +
            eventArray.length);
        for (var key in eventArray) {
            console.info('HiAppEventWatcherTest event_name=' + eventArray[key]['name'] + ', size=' +
                eventArray[key]['appEventInfos'].length);
        }
        console.info('HiAppEventWatcherTest.onReceive end');
    }

    function simpleWriteV9Test() {
        hiAppEventV9.write({
            domain: TEST_DOMAIN,
            name: TEST_NAME,
            eventType: hiAppEventV9.EventType.FAULT,
            params: {}
        }, (err) => {
            expect(err).assertNull()
        });
    }

    function watcherNameTest(name, expectErr) {
        try {
            hiAppEventV9.addWatcher({
                name: name
            });
        } catch (err) {
            assertErrorEqual(err, expectErr)
        }
    }

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

    /**
     * @tc.name: HiAppEventWatcherTest001
     * @tc.desc: test the addWatcher func when the watcher is not passed in.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest001', 0, function () {
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
    });

    /**
     * @tc.name: HiAppEventWatcherTest002
     * @tc.desc: test the addWatcher func when the input param is invalid type.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest002', 0, function () {
        let expectErr = createError2("watcher", "Watcher")
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
        addWatcherTypeTest("name");
        addWatcherTypeTest(true);
        removeWatcherTypeTest(null);
        removeWatcherTypeTest(123);
        removeWatcherTypeTest("name");
        removeWatcherTypeTest(true);
    });

    /**
     * @tc.name: HiAppEventWatcherTest003
     * @tc.desc: test the addWatcher func when watcher name not passed in.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest003', 0, function () {
        try {
            hiAppEventV9.addWatcher({});
        } catch (err) {
            let expectErr = createError3("name")
            assertErrorEqual(err, expectErr)
        }
    });

    /**
     * @tc.name: HiAppEventWatcherTest004
     * @tc.desc: test the addWatcher func when watcher name is invalid type.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest004', 0, function () {
        let expectErr = createError2("name", "string");

        watcherNameTest(null, expectErr);
        watcherNameTest(true, expectErr);
        watcherNameTest(123, expectErr);
        watcherNameTest(["name1", "name2"], expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest005
     * @tc.desc: test the addWatcher func when watcher name is invalid value.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest005', 0, function () {
        const MAX_LEN_OF_WATCHER = 32;
        let expectErr = createError(11102001, "Invalid watcher name. Possible causes: 1. Contain invalid characters; " +
            "2. Length is invalid.");
        watcherNameTest("a".repeat(MAX_LEN_OF_WATCHER + 1), expectErr);
        watcherNameTest("", expectErr);
        watcherNameTest("watcher_***", expectErr);
        watcherNameTest("Watcher_test", null);
        watcherNameTest("_watcher_test", expectErr);
        watcherNameTest("watcher_", expectErr);
        watcherNameTest("123watcher", expectErr);
        watcherNameTest("a".repeat(MAX_LEN_OF_WATCHER - 1) + "_", expectErr);
        watcherNameTest("a", null);
        watcherNameTest("a1", null);
        watcherNameTest("a".repeat(MAX_LEN_OF_WATCHER), null);
    });

    /**
     * @tc.name: HiAppEventWatcherTest006
     * @tc.desc: test the addWatcher func with invalid watcher trigger condition type.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest006', 0, function () {
        let expectErr = createError2("triggerCondition", "TriggerCondition");

        triggerConditionTest(null, expectErr);
        triggerConditionTest(123, expectErr);
        triggerConditionTest("test", expectErr);
        triggerConditionTest(true, expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest007
     * @tc.desc: test the addWatcher func with invalid watcher triggerCondition.row.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest007', 0, function () {
        function rowTest(row, expectErr) {
            triggerConditionTest({ row: row }, expectErr);
        }
        // invalid triggerCondition.row type
        let expectErr = createError2("row", "number");
        rowTest(null, expectErr);
        rowTest("str", expectErr);
        rowTest(true, expectErr);

        // invalid triggerCondition.row value
        expectErr = createError(11102003, "Invalid row value. Possible caused by the row value is less than zero.");
        rowTest(-1, expectErr);
        rowTest(-100, expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest008
     * @tc.desc: test the addWatcher func with invalid watcher triggerCondition.size.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest008', 0, function () {
        function sizeTest(size, expectErr) {
            triggerConditionTest({ size: size }, expectErr);
        }
        // invalid triggerCondition.size type
        let expectErr = createError2("size", "number");
        sizeTest(null, expectErr);
        sizeTest("str", expectErr);
        sizeTest(true, expectErr);

        // invalid triggerCondition.size value
        expectErr = createError(11102004, "Invalid size value. Possible caused by the size value is less than zero.");
        sizeTest(-1, expectErr);
        sizeTest(-100, expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest009
     * @tc.desc: test the addWatcher func with invalid watcher triggerCondition.timeout.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest009', 0, function () {
        function timeoutTest(timeout) {
            triggerConditionTest({ timeout: timeout }, expectErr);
        }
        // invalid triggerCondition.timeout type
        let expectErr = createError2("timeout", "number");
        timeoutTest(null, expectErr);
        timeoutTest("str", expectErr);
        timeoutTest(true, expectErr);
        timeoutTest({}, expectErr);

        // invalid triggerCondition.timeout value
        expectErr = createError(11102005, "Invalid timeout value. Possible caused by the timeout value is less than " +
            "zero.");
        timeoutTest(-1, expectErr);
        timeoutTest(-100, expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest010
     * @tc.desc: test the addWatcher func with invalid watcher appEventFilter type.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest010', 0, function () {
        let expectErr = createError2("appEventFilters", "AppEventFilter[]");
        appEventFiltersTest(null, expectErr);
        appEventFiltersTest({}, expectErr);
        appEventFiltersTest("invalid", expectErr);
        appEventFiltersTest(123, expectErr);
        appEventFiltersTest(true, expectErr);
        appEventFiltersTest([1, 2], expectErr);
        appEventFiltersTest(["str1", "str2"], expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest011
     * @tc.desc: test the addWatcher func with invalid watcher appEventFilter.domain.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest011', 0, function () {
        function appEventFilterTest(filter, expectErr) {
            appEventFiltersTest([filter], expectErr);
        }
        let expectErr = createError3("domain");
        appEventFilterTest({}, expectErr);

        // invalid appEventFilter.domain type
        function domainTest(domain, expectErr) {
            appEventFilterTest({ domain: domain }, expectErr);
        }
        expectErr = createError2("domain", "string");
        domainTest(null, expectErr);
        domainTest(123, expectErr);
        domainTest(true, expectErr);

        // invalid appEventFilter.domain value
        expectErr = createError(11102002, "Invalid filtering event domain. Possible causes: 1. Contain invalid " +
            "characters; 2. Length is invalid.");
        domainTest("**xx", expectErr);
        domainTest("123test", expectErr);
        domainTest("test_", expectErr);
        domainTest("a".repeat(33), expectErr);
        domainTest("", expectErr);
        domainTest("a", null);
        domainTest("a1", null);
        domainTest("Domain_1", null);
    });

    /**
     * @tc.name: HiAppEventWatcherTest012
     * @tc.desc: test the addWatcher func with invalid watcher appEventFilter.eventTypes.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest012', 0, function () {
        function appEventFilterTest(filter, expectErr) {
            appEventFiltersTest([filter], expectErr);
        }
        // invalid appEventFilter.eventTypes type
        function eventTypesTest(eventTypes, expectErr) {
            appEventFilterTest({
                domain: TEST_DOMAIN,
                eventTypes: eventTypes
            }, expectErr);
        }
        let expectErr = createError2("eventTypes", "EventType[]");
        eventTypesTest(null, expectErr);
        eventTypesTest("invalid", expectErr);
        eventTypesTest(true, expectErr);
        eventTypesTest(["invalid"], expectErr);
        eventTypesTest([10, -1], expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest013
     * @tc.desc: test the addWatcher func with invalid watcher onTrigger.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest013', 0, function () {
        function onTriggerTest(onTrigger, expectErr) {
            try {
                hiAppEventV9.addWatcher({
                    name: "watcher",
                    onTrigger: onTrigger
                });
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }
        let expectErr = createError2("onTrigger", "function");
        onTriggerTest(null, expectErr);
        onTriggerTest("invalid", expectErr);
        onTriggerTest(true, expectErr);
        onTriggerTest(123, expectErr);
    });

    /**
     * @tc.name: HiAppEventWatcherTest014
     * @tc.desc: test the addWatcher func when add valid watcher.
     * @tc.type: FUNC
     * @tc.require: issueI5LB4N
     */
    it('HiAppEventWatcherTest014', 0, function () {
        let watcher1 = {
            name: "watcher1",
        };
        let result = hiAppEventV9.addWatcher(watcher1);
        expect(result != null).assertTrue();

        let watcher2 = {
            name: "watcher2",
            triggerCondition: {}
        };
        result = hiAppEventV9.addWatcher(watcher2);
        expect(result != null).assertTrue();

        let watcher3 = {
            name: "watcher3",
            triggerCondition: {
                row: 5
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher3);
        expect(result != null).assertTrue();

        let watcher4 = {
            name: "watcher4",
            triggerCondition: {
                size: 1000
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher4);
        expect(result != null).assertTrue();

        let watcher5 = {
            name: "watcher5",
            triggerCondition: {
                timeOut: 2
            },
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher5);
        expect(result != null).assertTrue();

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
        expect(result != null).assertTrue();

        let watcher7 = {
            name: "watcher7",
            appEventFilters: []
        };
        result = hiAppEventV9.addWatcher(watcher7);
        expect(result != null).assertTrue();

        let watcher8 = {
            name: "watcher8",
            appEventFilters: [
                {domain: "domain_test", eventTypes: []},
                {domain: "default", eventTypes: [hiAppEventV9.EventType.FAULT, hiAppEventV9.EventType.BEHAVIOR]},
            ]
        };
        result = hiAppEventV9.addWatcher(watcher8);
        expect(result != null).assertTrue();

        hiAppEventV9.removeWatcher(watcher1);
        hiAppEventV9.removeWatcher(watcher2);
        hiAppEventV9.removeWatcher(watcher3);
        hiAppEventV9.removeWatcher(watcher4);
        hiAppEventV9.removeWatcher(watcher5);
        hiAppEventV9.removeWatcher(watcher6);
        hiAppEventV9.removeWatcher(watcher7);
        hiAppEventV9.removeWatcher(watcher8);
    });

    /**
     * @tc.name: HiAppEventWatcherTest015
     * @tc.desc: test the addWatcher func with watcher.onTrigger row test.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest015', 0, async function (done) {
        let watcher = {
            name: "watcher_015",
            appEventFilters: [
                { domain: TEST_DOMAIN },
            ],
            triggerCondition: {
                row: 1
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest015.onTrigger start');
                expect(curRow).assertEqual(1);
                expect(curSize > 0).assertTrue();
                expect(holder != null).assertTrue();

                let eventPkg = holder.takeNext();
                expect(eventPkg != null).assertTrue();
                expect(eventPkg.packageId).assertEqual(0);
                expect(eventPkg.row).assertEqual(1);
                expect(eventPkg.size > 0).assertTrue();
                expect(eventPkg.data.length).assertEqual(1);
                expect(eventPkg.data[0].length > 0).assertTrue();
                console.info('HiAppEventWatcherTest015.onTrigger end');
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest016
     * @tc.desc: test the addWatcher func with watcher.onTrigger size test.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest016', 0, async function (done) {
        let watcher = {
            name: "watcher_016",
            appEventFilters: [
                { domain: TEST_DOMAIN },
            ],
            triggerCondition: {
                row: 10,
                size: 200,
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest016.onTrigger start');
                expect(curRow).assertEqual(2);
                expect(curSize >= 200).assertTrue();
                expect(holder != null).assertTrue();

                holder.setSize(curSize);
                let eventPkg = holder.takeNext();
                expect(eventPkg != null).assertTrue();
                expect(eventPkg.packageId).assertEqual(0);
                expect(eventPkg.row).assertEqual(2);
                expect(eventPkg.size >= 200).assertTrue();
                expect(eventPkg.data.length).assertEqual(2);
                expect(eventPkg.data[0].length > 0).assertTrue();
                expect(eventPkg.data[1].length > 0).assertTrue();
                console.info('HiAppEventWatcherTest016.onTrigger end');
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        simpleWriteV9Test();
        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest017
     * @tc.desc: test the addWatcher func with watcher.onTrigger timeout test.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest017', 0, async function (done) {
        let watcher = {
            name: "watcher_017",
            appEventFilters: [
                { domain: TEST_DOMAIN },
            ],
            triggerCondition: {
                timeOut: 1
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest009.onTrigger start');
                expect(curRow).assertEqual(1);
                expect(curSize > 0).assertTrue();
                expect(holder != null).assertTrue();

                let eventPkg = holder.takeNext();
                expect(eventPkg != null).assertTrue();
                expect(eventPkg.packageId).assertEqual(0);
                expect(eventPkg.row).assertEqual(1);
                expect(eventPkg.size > 0).assertTrue();
                expect(eventPkg.data.length).assertEqual(1);
                expect(eventPkg.data[0].length > 0).assertTrue();
                console.info('HiAppEventWatcherTest009.onTrigger end');
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 3000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest018
     * @tc.desc: test the holder.takeNext func.
     * @tc.type: FUNC
     * @tc.require: issueI5NTOD
     */
    it('HiAppEventWatcherTest018', 0, async function (done) {
        let watcher = {
            name: "watcher_018",
        };
        let holder = hiAppEventV9.addWatcher(watcher);
        expect(holder != null).assertTrue();

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
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest019
     * @tc.desc: test the holder.setSize func.
     * @tc.type: FUNC
     * @tc.require: issueI5NTOD
     */
    it('HiAppEventWatcherTest019', 0, function () {
        let watcher = {
            name: "watcher",
        };
        let holder = hiAppEventV9.addWatcher(watcher);
        expect(holder != null).assertTrue();

        // size not passed in
        try {
            holder.setSize();
        } catch (err) {
            let expectErr = createError3("size");
            assertErrorEqual(err, expectErr);
        }

        // invalid size type
        function holderSetSizeTest(holder, size, expectErr) {
            try {
                holder.setSize(size);
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }
        let expectErr = createError2("size", "number");
        holderSetSizeTest(holder, null, expectErr);
        holderSetSizeTest(holder, "str", expectErr);
        holderSetSizeTest(holder, true, expectErr);
        holderSetSizeTest(holder, {}, expectErr);

        // invalid size value
        expectErr = createError(11104001,
            "Invalid size value. Possible caused by the size value is less than or equal to zero.");
        holderSetSizeTest(holder, -1, expectErr);
        holderSetSizeTest(holder, -100, expectErr);

        hiAppEventV9.removeWatcher(watcher);
    });

    /**
     * @tc.name: HiAppEventWatcherTest020
     * @tc.desc: test the holder constructor func.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
     it('HiAppEventWatcherTest020', 0, async function (done) {
        let watcher = {
            name: "watcher",
        };
        hiAppEventV9.addWatcher(watcher);

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
        hiAppEventV9.write({
            domain: TEST_DOMAIN,
            name: TEST_NAME,
            eventType: hiAppEventV9.EventType.FAULT,
            params: params
        }, (err) => {
            expect(err).assertNull();

            let holder = new hiAppEventV9.AppEventPackageHolder("watcher");
            let eventPkg = holder.takeNext();
            console.info("HiAppEventWatcherTest020 (eventPkg != null) is " + eventPkg != null);
            expect(eventPkg != null).assertTrue();
            console.info("eventPkg.data.length is " + eventPkg.data.length);
            expect(eventPkg.data.length == 1).assertTrue();
            let paramJsonStr = JSON.stringify(params);;
            console.info("paramJsonStr = " + paramJsonStr + ", len = " + paramJsonStr.length());
            console.info("eventPkg.data[0] = " + eventPkg.data[0] + ", len = " + eventPkg.data[0].length());
            expect(eventPkg.data[0].includes(paramJsonStr.substr(1, paramJsonStr.length - 2))).assertTrue();
            hiAppEventV9.removeWatcher(watcher);
            done();
        });
    });

    /**
     * @tc.name: HiAppEventWatcherTest021
     * @tc.desc: test the watcher.onTrigger after clear up.
     * @tc.type: FUNC
     * @tc.require: issueI8H07G
     */
     it('HiAppEventWatcherTest021', 0, async function (done) {
        let testRow = 5;
        let watcher = {
            name: "watcher",
            triggerCondition: {
                row: testRow
            },
            onTrigger: function (curRow, curSize, holder) {
                console.info('HiAppEventWatcherTest021.onTrigger start');
                holder.setRow(testRow);
                let eventPkg = holder.takeNext();
                expect(eventPkg.data.length).assertEqual(testRow);
                console.info('HiAppEventWatcherTest021.onTrigger end');
            }
        };
        hiAppEventV9.addWatcher(watcher);

        setTimeout(() => {
            for (var i = 1; i <= 3; i++) {
                simpleWriteV9Test();
            }
        }, 1000);

        setTimeout(() => {
            hiAppEventV9.clearData();
        }, 2000);

        setTimeout(() => {
            for (var i = 1; i <= testRow; i++) {
                simpleWriteV9Test();
            }
        }, 3000);

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 4000);
    });

    

    /**
     * @tc.name: HiAppEventWatcherTest022
     * @tc.desc: test the addWatcher func with watcher.onReceive subscribe OS event.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest022', 0, async function (done) {
        let watcher = {
            name: "watcheros1",
            appEventFilters: [
                {
                    domain: hiAppEventV9.domain.OS,
                    names: [
                        hiAppEventV9.event.APP_CRASH,
                        hiAppEventV9.event.APP_FREEZE,
                        hiAppEventV9.event.APP_LAUNCH,
                        hiAppEventV9.event.SCROLL_JANK,
                        hiAppEventV9.event.CPU_USAGE_HIGH,
                        hiAppEventV9.event.BATTERY_USAGE,
                        hiAppEventV9.event.RESOURCE_OVERLIMIT,
                        hiAppEventV9.event.ADDRESS_SANITIZER
                    ]
                },
            ],
            onReceive: simpleReceive
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest023
     * @tc.desc: test the addWatcher func with watcher.onReceive subscribe app event.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest023', 0, async function (done) {
        let watcher = {
            name: "watcher",
            appEventFilters: [
                {domain: TEST_DOMAIN, names: [TEST_NAME]},
            ],
            onReceive: simpleReceive
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest024
     * @tc.desc: test the addWatcher func with different callback func.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest024', 0, async function (done) {
        let result = true;
        let watcher1 = {
            name: "watcheros1",
            appEventFilters: [
                {
                    domain: hiAppEventV9.domain.OS,
                    names: [hiAppEventV9.event.APP_CRASH, hiAppEventV9.event.APP_FREEZE]
                },
            ],
            onReceive: simpleReceive
        };
        result = hiAppEventV9.addWatcher(watcher1);
        expect(result != null).assertTrue();

        let watcher2 = {
            name: "watcheros2",
            appEventFilters: [
                {
                    domain: hiAppEventV9.domain.OS,
                    names: [hiAppEventV9.event.APP_CRASH, hiAppEventV9.event.APP_FREEZE]
                },
            ],
            onReceive: simpleReceive,
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher2);
        expect(result != null).assertTrue();

        let watcher3 = {
            name: "watcheros3",
            appEventFilters: [
                {
                    domain: hiAppEventV9.domain.OS,
                    names: [hiAppEventV9.event.APP_CRASH, hiAppEventV9.event.APP_FREEZE]
                },
            ],
            onTrigger: simpleTrigger
        };
        result = hiAppEventV9.addWatcher(watcher3);
        expect(result != null).assertTrue();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher1);
            hiAppEventV9.removeWatcher(watcher2);
            hiAppEventV9.removeWatcher(watcher3);
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest025
     * @tc.desc: test watcher onReceive with two consecutive callbacks.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest025', 0, async function (done) {
        let eventName = "";
        let watcher = {
            name: "watcher_025",
            appEventFilters: [
                {domain: "testDomain"},
            ],
            onReceive: (domain, appEventGroups) => {
                for (const eventGroup of appEventGroups) {
                    for (const eventInfo of eventGroup.appEventInfos) {
                        let curName = eventInfo.name;
                        expect(curName != eventName).assertTrue();
                        eventName = curName;   // The name of the events should be different.
                    }
                }
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        hiAppEventV9.write({
            domain: "testDomain",
            name: "name1",
            eventType: hiAppEventV9.EventType.FAULT,
            params: {"key": "value"}
        }, (err) => {
            assertErrorEqual(err, null);
        })
        hiAppEventV9.write({
            domain: "testDomain",
            name: "name2",
            eventType: hiAppEventV9.EventType.FAULT,
            params: {"key": "value"}
        }, (err) => {
            expect(err).assertNull();
        })

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 1000);
    });

    /**
     * @tc.name: HiAppEventWatcherTest026
     * @tc.desc: test watcher onReceive with two consecutive callbacks when Multi-thread concurrent data contention.
     * @tc.type: FUNC
     * @tc.require: issueI5KYYI
     */
    it('HiAppEventWatcherTest026', 0, async function (done) {
        let watcher = {
            name: "watcher_026",
            appEventFilters: [
                {domain: TEST_DOMAIN},
            ],
            onReceive: (domain, appEventGroups) => {
                hiAppEventV9.removeWatcher(watcher);  // it should not cause a crash, otherwise, there is a bug in code.
            }
        };
        let result = hiAppEventV9.addWatcher(watcher);
        expect(result != null).assertTrue();

        simpleWriteV9Test();
        simpleWriteV9Test();

        setTimeout(() => {
            hiAppEventV9.removeWatcher(watcher);
            done();
        }, 1000);
    });
});