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

    function createError2(name, type) {
        return { code: "401", message: "Parameter error. The type of " + name + " must be " + type + "." };
    }

    function createError4(name) {
        return { code: "401", message: "Parameter error. The " + name + " parameter is invalid." };
    }

    function assertErrorEqual(actualErr, expectErr) {
        if (expectErr) {
            expect(actualErr.code).assertEqual(expectErr.code)
            expect(actualErr.message).assertEqual(expectErr.message)
        } else {
            expect(actualErr).assertNull();
        }
    }

    const USER_ID_NAME = 'testUserIdName';
    const USER_ID_VALUE = 'testUserIdValue';
    const USER_PROP_NAME = 'testUserPropName';
    const USER_PROP_VALUE = 'testUserPropValue';
    const INVALID_USER_ID_NAMES = [-1, '', null, undefined, '%test', `testUserIdName` + 'a'.repeat(256)];
    const INVALID_USER_ID_VALUES = [-1, '', null, undefined, `testUserIdValue` + 'a'.repeat(256)];
    const INVALID_USER_PROP_NAMES = [-1, '', null, undefined, '%test', `testUserPropName` + 'a'.repeat(256)];
    const INVALID_USER_PROP_VALUES = [-1, '', null, undefined, `testUserPropValue` + 'a'.repeat(1024)];
    const CLEAR_USER_INFO_VALUES = ['', null, undefined];

    /**
     * @tc.number: HiAppEventUserIdsTest001_1
     * @tc.name: HiAppEventUserIdsTest
     * @tc.desc: Correctly set and get the user ids function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserIdsTest001_1', 0, function () {
        console.info('HiAppEventUserIdsTest001_1 start');

        let expectErr = createError2("userId", "string");

        try {
            console.info(`setUserId name: ${USER_ID_NAME}, value: ${USER_ID_VALUE}`);
            hiAppEventV9.setUserId(USER_ID_NAME, USER_ID_VALUE);

            console.info(`getUserId name: ${USER_ID_NAME}`);
            let res = hiAppEventV9.getUserId(USER_ID_NAME);
            expect(res != "").assertTrue();
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }

        console.info('HiAppEventUserIdsTest001_1 end');
    });

    /**
     * @tc.number: HiAppEventUserIdsTest001_2
     * @tc.name: HiAppEventUserIdsTest
     * @tc.desc: Correctly clear and get the user ids function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
     it('HiAppEventUserIdsTest001_2', 0, function () {
        console.info('HiAppEventUserIdsTest001_2 start');

        let expectErr = createError2("userId", "string");

        try {
            console.info(`setUserId name: ${USER_ID_NAME}, value: ${USER_ID_VALUE}`);
            hiAppEventV9.setUserId(USER_ID_NAME, USER_ID_VALUE);

            hiAppEventV9.clearData();

            console.info(`getUserId name: ${USER_ID_NAME}`);
            let res = hiAppEventV9.getUserId(USER_ID_NAME);
            expect(res == "").assertTrue();
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }

        console.info('HiAppEventUserIdsTest001_2 end');
    });

    /**
     * @tc.number: HiAppEventUserIdsTest002_1
     * @tc.name: HiAppEventUserIdsTest
     * @tc.desc: Incorrectly set the user ids function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserIdsTest002_1', 0, function () {
        console.info('HiAppEventUserIdsTest002_1 start');

        for (let userIdName of INVALID_USER_ID_NAMES) {
            let expectErr = typeof userIdName == 'string' ? createError4("name") : createError2("name", "string");
            try {
                console.info(`setUserId name: ${userIdName}, value: ${USER_ID_VALUE}`);
                hiAppEventV9.setUserId(userIdName, USER_ID_VALUE);
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }
        for (let userIdVal of INVALID_USER_ID_VALUES) {
            let expectErr = typeof userIdVal == 'string' ? createError4("value") : createError2("userId", "string");
            try {
                console.info(`setUserId name: ${USER_ID_NAME}, value: ${userIdVal}`);
                hiAppEventV9.setUserId(USER_ID_NAME, userIdVal);
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }

        console.info('HiAppEventUserIdsTest002_1 end');
    });

    /**
     * @tc.number: HiAppEventUserIdsTest003_1
     * @tc.name: HiAppEventUserIdsTest
     * @tc.desc: Incorrectly get the user ids function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserIdsTest003_1', 0, function () {
        console.info('HiAppEventUserIdsTest003_1 start');

        console.info(`setUserId name: ${USER_ID_NAME}, value: ${USER_ID_VALUE}`);
        hiAppEventV9.setUserId(USER_ID_NAME, USER_ID_VALUE);

        for (let userIdName of INVALID_USER_ID_NAMES) {
            let expectErr = typeof userIdName == 'string' ? createError4("name") : createError2("name", "string");
            try {
                console.info(`getUserId name: ${userIdName}`);
                let res = hiAppEventV9.getUserId(userIdName);
                expect(res == "").assertTrue();
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }

        console.info('HiAppEventUserIdsTest003_1 end');
    });

    /**
     * @tc.number: HiAppEventUserIdsTest004_1
     * @tc.name: HiAppEventUserIdsTest
     * @tc.desc: Correctly replace and get the user ids function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserIdsTest004_1', 0, function () {
        console.info('HiAppEventUserIdsTest004_1 start');

        let expectErr = createError2("userId", "string");

        try {
            console.info(`setUserId name: ${USER_ID_NAME}, value: ${USER_ID_VALUE}`);
            hiAppEventV9.setUserId(USER_ID_NAME, USER_ID_VALUE);
            console.info(`setUserId name: ${USER_ID_NAME}, value: ${'testUserIdValue1'}`);
            hiAppEventV9.setUserId(USER_ID_NAME, "testUserIdValue1");

            console.info(`getUserId name: ${USER_ID_NAME}`);
            let res = hiAppEventV9.getUserId(USER_ID_NAME);
            expect(res == "testUserIdValue1").assertTrue();
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }

        console.info('HiAppEventUserIdsTest004_1 end');
    });

    /**
     * @tc.number: HiAppEventUserIdsTest005_1
     * @tc.name: HiAppEventUserIdsTest
     * @tc.desc: Correctly remove and get the user ids function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserIdsTest005_1', 0, function () {
        console.info('HiAppEventUserIdsTest005_1 start');

        let expectErr = createError2("userId", "string");

        for (let userIdVal of CLEAR_USER_INFO_VALUES) {
            try {
                console.info(`setUserId name: ${USER_ID_NAME}, value: ${USER_ID_VALUE}`);
                hiAppEventV9.setUserId(USER_ID_NAME, USER_ID_VALUE);
                console.info(`setUserId name: ${USER_ID_NAME}, value: ${userIdVal}`);
                hiAppEventV9.setUserId(USER_ID_NAME, userIdVal);

                console.info(`getUserId name: ${USER_ID_NAME}`);
                let res = hiAppEventV9.getUserId(USER_ID_NAME);
                expect(res == "").assertTrue();
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }

        console.info('HiAppEventUserIdsTest005_1 end');
    });

    /**
     * @tc.number: HiAppEventUserPropertyTest001_1
     * @tc.name: HiAppEventUserPropertyTest
     * @tc.desc: Correctly set and get the user properties function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserPropertyTest001_1', 0, function () {
        console.info('HiAppEventUserPropertyTest001_1 start');

        let expectErr = createError2("userProperties", "string");

        try {
            console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${USER_PROP_VALUE}`);
            hiAppEventV9.setUserProperty(USER_PROP_NAME, USER_PROP_VALUE);

            console.info(`getUserProperty name: ${USER_PROP_NAME}`);
            let res = hiAppEventV9.getUserProperty(USER_PROP_NAME);
            expect(res != "").assertTrue();
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }

        console.info('HiAppEventUserPropertyTest001_1 end');
    });

    /**
     * @tc.number: HiAppEventUserPropertyTest001_2
     * @tc.name: HiAppEventUserPropertyTest
     * @tc.desc: Correctly clear and get the user properties function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserPropertyTest001_2', 0, function () {
        console.info('HiAppEventUserPropertyTest001_2 start');

        let expectErr = createError2("userProperties", "string");

        try {
            console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${USER_PROP_VALUE}`);
            hiAppEventV9.setUserProperty(USER_PROP_NAME, USER_PROP_VALUE);

            hiAppEventV9.clearData();

            console.info(`getUserProperty name: ${USER_PROP_NAME}`);
            let res = hiAppEventV9.getUserProperty(USER_PROP_NAME);
            expect(res == "").assertTrue();
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }

        console.info('HiAppEventUserPropertyTest001_2 end');
    });

    /**
     * @tc.number: HiAppEventUserPropertyTest002_1
     * @tc.name: HiAppEventUserPropertyTest
     * @tc.desc: Incorrectly set the user properties function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserPropertyTest002_1', 0, function () {
        console.info('HiAppEventUserPropertyTest002_1 start');

        for (let userPropertyName of INVALID_USER_PROP_NAMES) {
            let expectErr = typeof userPropertyName == 'string' ? createError4("name") : createError2("name", "string");
            try {
                console.info(`setUserProperty name: ${userPropertyName}, value: ${USER_PROP_VALUE}`);
                hiAppEventV9.setUserProperty(userPropertyName, USER_PROP_VALUE);
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }
        for (let userPropertyVal of INVALID_USER_PROP_VALUES) {
            let expectErr = typeof userPropertyVal == 'string' ? createError4("value")
            : createError2("user property", "string");
            try {
                console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${userPropertyVal}`);
                hiAppEventV9.setUserProperty(USER_PROP_NAME, userPropertyVal);
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }

        console.info('HiAppEventUserPropertyTest002_1 end');
    });

    /**
     * @tc.number: HiAppEventUserPropertyTest003_1
     * @tc.name: HiAppEventUserPropertyTest
     * @tc.desc: Incorrectly set the user properties function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserPropertyTest003_1', 0, function () {
        console.info('HiAppEventUserPropertyTest003_1 start');

        console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${USER_PROP_VALUE}`);
        hiAppEventV9.setUserProperty(USER_PROP_NAME, USER_PROP_VALUE);

        for (let userPropertyName of INVALID_USER_PROP_NAMES) {
            let expectErr = typeof userPropertyName == 'string' ? createError4("name") : createError2("name", "string");
            try {
                console.info(`getUserProperty name: ${USER_PROP_NAME}`);
                let res = hiAppEventV9.getUserProperty(userPropertyName);
                expect(res == "").assertTrue();
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }

        console.info('HiAppEventUserPropertyTest003_1 end');
    });

    /**
     * @tc.number: HiAppEventUserPropertyTest004_1
     * @tc.name: HiAppEventUserPropertyTest
     * @tc.desc: Correctly replace and get the user properties function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserPropertyTest004_1', 0, function () {
        console.info('HiAppEventUserPropertyTest004_1 start');

        let expectErr = createError2("userProperties", "string");

        try {
            console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${USER_PROP_VALUE}`);
            hiAppEventV9.setUserProperty(USER_PROP_NAME, USER_PROP_VALUE);
            console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${'testUserProperty1'}`);
            hiAppEventV9.setUserProperty(USER_PROP_NAME, "testUserProperty1");

            console.info(`getUserProperty name: ${USER_PROP_NAME}`);
            let res = hiAppEventV9.getUserProperty(USER_PROP_NAME);
            expect(res == "testUserProperty1").assertTrue();
        } catch (err) {
            assertErrorEqual(err, expectErr);
        }

        console.info('HiAppEventUserPropertyTest004_1 end');
    });

    /**
     * @tc.number: HiAppEventUserPropertyTest005_1
     * @tc.name: HiAppEventUserPropertyTest
     * @tc.desc: Correctly remove and get the user properties function.
     * @tc.type: FUNC
     * @tc.require: issueI8G4M9
     */
    it('HiAppEventUserPropertyTest005_1', 0, function () {
        console.info('HiAppEventUserPropertyTest005_1 start');

        let expectErr = createError2("user property", "string");

        for (let userPropertyVal of CLEAR_USER_INFO_VALUES) {
            try {
                console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${USER_PROP_VALUE}`);
                hiAppEventV9.setUserProperty(USER_PROP_NAME, USER_PROP_VALUE);
                console.info(`setUserProperty name: ${USER_PROP_NAME}, value: ${userPropertyVal}`);
                hiAppEventV9.setUserProperty(USER_PROP_NAME, userPropertyVal);

                console.info(`getUserProperty name: ${USER_PROP_NAME}`);
                let res = hiAppEventV9.getUserProperty(USER_PROP_NAME);
                expect(res == "").assertTrue();
            } catch (err) {
                assertErrorEqual(err, expectErr);
            }
        }

        console.info('HiAppEventUserPropertyTest005_1 end');
    });
});