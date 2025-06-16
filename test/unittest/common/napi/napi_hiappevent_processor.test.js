/*
 * Copyright (C) 2023-2025 Huawei Device Co., Ltd.
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

    function createError3(message) {
        return { code: "11105001", message: message };
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
            ],
            configId: 1,
            customConfigs: {
                "str_key": "str_value"
            }
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

    /**
     * @tc.number: HiAppEventProcessorTest003
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with same processorId.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest003', 0, function () {
        let config = {
            name: "test_processor",
            configId: 1,
        };
        let processorId1 = hiAppEvent.addProcessor(config);
        expect(processorId1).assertLarger(0);

        config.customConfigs = {
            "str_key": "str_value"
        };
        let processorId2 = hiAppEvent.addProcessor(config);
        expect(processorId2).assertLarger(0);
        expect(processorId2 == processorId1).assertTrue();

        config.configId = 0;
        let processorId3 = hiAppEvent.addProcessor(config);
        expect(processorId3).assertLarger(0);
        expect(processorId3 != processorId2).assertTrue();

        hiAppEvent.removeProcessor(processorId1);
        hiAppEvent.removeProcessor(processorId2);
        hiAppEvent.removeProcessor(processorId3);
    });

    /**
     * @tc.number: HiAppEventProcessorTest004_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configId.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest004_1', 0, function () {
        let expectConfig = {
            name: "test_processor",
            configId: 0,
        }
        let processorId = hiAppEvent.addProcessor(expectConfig);
        expect(processorId).assertLarger(0);

        let config = {
            name: "test_processor",
            configId: -1,
        };
        let processorId1 = hiAppEvent.addProcessor(config);
        expect(processorId1).assertLarger(0);
        expect(processorId1 == processorId).assertTrue();

        hiAppEvent.removeProcessor(processorId);
    });

    /**
     * @tc.number: HiAppEventProcessorTest004_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid customConfigs.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest004_2', 0, function () {
        let expectConfig = {
            name: "test_processor",
            customConfigs: {},
        }
        let processorId = hiAppEvent.addProcessor(expectConfig);
        expect(processorId).assertLarger(0);

        let config = {
            name: "test_processor",
            customConfigs: {
                "": "str_value"
            },
        };
        let processorId1 = hiAppEvent.addProcessor(config);
        expect(processorId1).assertLarger(0);
        expect(processorId1 == processorId).assertTrue();

        config.customConfigs = {
            "***key": "str_value"
        };
        let processorId2 = hiAppEvent.addProcessor(config);
        expect(processorId2).assertLarger(0);
        expect(processorId2 == processorId).assertTrue();

        config.customConfigs = {
            "abc012345678901234567890123456789": "str_value"
        };
        let processorId3 = hiAppEvent.addProcessor(config);
        expect(processorId3).assertLarger(0);
        expect(processorId3 == processorId).assertTrue();

        config.customConfigs = {
            "str_key": "a".repeat(1024 + 1)
        };
        let processorId4 = hiAppEvent.addProcessor(config);
        expect(processorId4).assertLarger(0);
        expect(processorId4 == processorId).assertTrue();

        hiAppEvent.removeProcessor(processorId);
    });

    /**
     * @tc.number: HiAppEventProcessorTest005
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with valid configName.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest005', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: "SDK_OCG"
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 != processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
        hiAppEvent.removeProcessor(processorId2);
    });

    /**
     * @tc.number: HiAppEventProcessorTest006
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with undefined configName.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest006', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: "undefined_name"
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 != processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest007_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configName is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest007_1', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: ""
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 == processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest007_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configName is null.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest007_2', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: null
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 == processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest007_3
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configName is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest007_3', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: 0
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 == processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest007_4
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configName has special char.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest007_4', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: "xxx***"
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 == processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest007_5
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configName beginner is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest007_5', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        let processor2 = {
            name: "test_processor",
            configName: "123_processor"
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 == processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest007_6
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: Add processor with invalid configName length is over range.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest007_6', 0, function () {
        let processor1 = {
            name: "test_processor",
        }
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);

        const maxLen = 256;
        let processor2 = {
            name: "test_processor",
            configName: 'a'.repeat(maxLen + 1)
        }
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);

        expect(processorId1 == processorId2).assertTrue();
        hiAppEvent.removeProcessor(processorId1);
    });

    /**
     * @tc.number: HiAppEventProcessorTest008_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig without configName when use correctly.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest008_1', 0, function () {
        hiAppEvent.addProcessorFromConfig("ha_app_event").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            expect(err).assertNull();
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest008_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig with configName when use correctly.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest008_2', 0, function () {
        hiAppEvent.addProcessorFromConfig("ha_app_event", "SDK_OCG").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            expect(err).assertNull();
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest009_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when processorName is undefined.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest009_1', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        hiAppEvent.addProcessorFromConfig("undefined").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest009_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when processorName is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest009_2', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        hiAppEvent.addProcessorFromConfig("").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest009_3
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when processorName is null.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest009_3', 0, function () {
        let expectErr = "Parameter error. The type of processorName must be string.";
        try {
            hiAppEvent.addProcessorFromConfig(null).then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                expect(err).assertNull();
            })
        } catch (err) {
            expect(err.message).assertEqual(expectErr);
        }
    });

    /**
     * @tc.number: HiAppEventProcessorTest009_4
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when processorName is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest009_4', 0, function () {
        let expectErr = "Parameter error. The type of processorName must be string.";
        try {
            hiAppEvent.addProcessorFromConfig(0).then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                expect(err).assertNull();
            })
        } catch (err) {
            expect(err.message).assertEqual(expectErr);
        }
    });

    /**
     * @tc.number: HiAppEventProcessorTest009_5
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when processorName is over range.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest009_5', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        let processorName = 'a'.repeat(256 +1);
        hiAppEvent.addProcessorFromConfig(processorName).then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest009_6
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when processorName has special char.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
        it('HiAppEventProcessorTest009_6', 0, function () {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("xxx***").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        });

        /**
         * @tc.number: HiAppEventProcessorTest009_7
         * @tc.name: HiAppEventProcessorTest
         * @tc.desc: test addProcessorFromConfig when processorName beginner is num.
         * @tc.type: FUNC
         * @tc.require: issueI8U2VO
         */
        it('HiAppEventProcessorTest009_7', 0, function () {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("123_processor").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        });

    /**
     * @tc.number: HiAppEventProcessorTest010_1
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName is undefined.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_1', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        hiAppEvent.addProcessorFromConfig("ha_app_event", "undefined").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest010_2
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_2', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        hiAppEvent.addProcessorFromConfig("ha_app_event", "").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest010_3
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName is null.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_3', 0, function () {
        let expectErr = "Parameter error. The type of configName must be string.";
        try {
            hiAppEvent.addProcessorFromConfig("ha_app_event", null).then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                expect(err).assertNull();
            })
        } catch (err) {
            expect(err.message).assertEqual(expectErr);
        }
    });

    /**
     * @tc.number: HiAppEventProcessorTest010_4
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_4', 0, function () {
        let expectErr = "Parameter error. The type of configName must be string.";
        try {
            hiAppEvent.addProcessorFromConfig("ha_app_event", 0).then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                expect(err).assertNull();
            })
        } catch (err) {
            expect(err.message).assertEqual(expectErr);
        }
    });

    /**
     * @tc.number: HiAppEventProcessorTest010_5
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName is over range.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_5', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        let configName = 'a'.repeat(256 +1);
        hiAppEvent.addProcessorFromConfig("ha_app_event", configName).then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest010_6
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName has special char.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_6', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        hiAppEvent.addProcessorFromConfig("ha_app_event", "xxx***").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });

    /**
     * @tc.number: HiAppEventProcessorTest010_7
     * @tc.name: HiAppEventProcessorTest
     * @tc.desc: test addProcessorFromConfig when configName beginner is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('HiAppEventProcessorTest010_7', 0, function () {
        let expectErr = createError3("Invalid param value for add processor from config.");
        hiAppEvent.addProcessorFromConfig("ha_app_event", "123_processor").then((processorId) => {
            expect(processorId).assertLarger(0);
            hiAppEvent.removeProcessor(processorId);
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
        })
    });
});