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
import { fileIo as fs } from '@ohos.file.fs';

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('HiAppEventJsTest', function () {
    let isConfigFileExist = false;
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all test cases
         */
        console.info('HiAppEventJsTest beforeAll called')
        isConfigFileExist = fs.accessSync("/system/etc/hiappevent/processor.json")
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

    function validProcessorTest(processor, done) {
        let processorId = hiAppEvent.addProcessor(processor);
        expect(processorId).assertLarger(0);
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

    function assertAddProcessorResult(processor1, processor2, isEqual) {
        let processorId1 = hiAppEvent.addProcessor(processor1);
        expect(processorId1).assertLarger(0);
        let processorId2 = hiAppEvent.addProcessor(processor2);
        expect(processorId2).assertLarger(0);
        if (isEqual) {
            expect(processorId1 == processorId2).assertTrue();
        } else {
            expect(processorId1 != processorId2).assertTrue();
        }
        hiAppEvent.removeProcessor(processorId1);
        hiAppEvent.removeProcessor(processorId2);
    }

    /**
     * @tc.name: AddProcessorTest001
     * @tc.desc: Add processor when all configs were set.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest001', 0, async function (done) {
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
            },
            configName: "test_configName"
        };
        validProcessorTest(processor1, done);
    });

    /**
     * @tc.name: AddProcessorTest002
     * @tc.desc: Add processor when param is not processor types.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest002', 0, function () {
        let expectErr = createError1("config", "Processor");
        invalidProcessorTest("str", expectErr);
        invalidProcessorTest(0, expectErr);
        invalidProcessorTest(true, expectErr);
        invalidProcessorTest(null, expectErr);
        invalidProcessorTest(undefined, expectErr);
    });

    /**
     * @tc.name: AddProcessorTest003_1
     * @tc.desc: Add processor when param name with type error.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest003_1', 0, function () {
        let expectErr1 = createError1("name", "string");
        let processor1 = {
            name: 0,
        };
        invalidProcessorTest(processor1, expectErr1);

        let processor2 = {
            name: true,
        };
        invalidProcessorTest(processor2, expectErr1);

        let processor3 = {
            name: {},
        };
        invalidProcessorTest(processor3, expectErr1);

        let processor4 = {
            name: null,
        };
        invalidProcessorTest(processor4, expectErr1);

        let processor5 = {
            name: undefined,
        };
        invalidProcessorTest(processor5, expectErr1);
    });

    /**
     * @tc.name: AddProcessorTest003_2
     * @tc.desc: Add processor when param name with invalid format.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest003_2', 0, function () {
        let expectErr = createError2("Invalid processor name.");
        let processor1 = {
            name: "",
        };
        invalidProcessorTest(processor1, expectErr);

        let processor2 = {
            name: "xxx***",
        };
        invalidProcessorTest(processor2, expectErr);

        let processor3 = {
            name: "123_processor",
        };
        invalidProcessorTest(processor3, expectErr);

        const maxLen = 256;
        let processor4 = {
            name: 'a'.repeat(maxLen + 1),
        };
        invalidProcessorTest(processor4, expectErr);
    });

    /**
     * @tc.name: AddProcessorTest004
     * @tc.desc: Add processor for test param debugMode.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest004', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            debugMode: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            debugMode: 0,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            debugMode: true,
        };
        assertAddProcessorResult(processor2, processor3, false);
    });

    /**
     * @tc.name: AddProcessorTest005
     * @tc.desc: Add processor for test param routeInfo.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest005', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            routeInfo: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            routeInfo: 0,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            routeInfo: "test_routeInfo",
        };
        assertAddProcessorResult(processor2, processor3, false);

        const maxLen = 8 * 1024 + 1;  // 8KB + 1
        let processor4 = {
            name: "test_processor",
            routeInfo: 'a'.repeat(maxLen + 1),
        };        
        assertAddProcessorResult(processor1, processor4, true);
    });

    /**
     * @tc.name: AddProcessorTest006
     * @tc.desc: Add processor for test param appId.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest006', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            appId: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            appId: 0,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            appId: "test_appId",
        };
        assertAddProcessorResult(processor2, processor3, false);

        const maxLen = 8 * 1024 + 1;  // 8KB + 1
        let processor4 = {
            name: "test_processor",
            appId: 'a'.repeat(maxLen + 1),
        };
        assertAddProcessorResult(processor1, processor4, true);
    });

    /**
     * @tc.name: AddProcessorTest007
     * @tc.desc: Add processor for test param onStartReport.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest007', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            appId: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            onStartReport: 0,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            onStartReport: true,
        };
        assertAddProcessorResult(processor2, processor3, false);
    });

    /**
     * @tc.name: AddProcessorTest008
     * @tc.desc: Add processor for test param onBackgroundReport.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest008', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            onBackgroundReport: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            onBackgroundReport: 0,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            onBackgroundReport: true,
        };
        assertAddProcessorResult(processor2, processor3, false);
    });

    /**
     * @tc.name: AddProcessorTest009
     * @tc.desc: Add processor for test param periodReport.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest009', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            periodReport: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            periodReport: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            periodReport: -1,
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            periodReport: 1,
        };
        assertAddProcessorResult(processor3, processor4, false);
    });

    /**
     * @tc.name: AddProcessorTest010
     * @tc.desc: Add processor for test param batchReport.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest010', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            batchReport: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            batchReport: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            batchReport: 0,
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            batchReport: 1001,
        };
        assertAddProcessorResult(processor3, processor4, true);

        let processor5 = {
            name: "test_processor",
            batchReport: 1,
        };
        assertAddProcessorResult(processor4, processor5, false);
    });

    /**
     * @tc.name: AddProcessorTest011
     * @tc.desc: Add processor for test param userIds.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest011', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            userIds: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            userIds: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            userIds: [123, 456],
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            userIds: ["id1", "id2", null],
        };
        assertAddProcessorResult(processor3, processor4, true);
    });

    /**
     * @tc.name: AddProcessorTest012
     * @tc.desc: Add processor for test param userProperties.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest012', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            userProperties: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            userProperties: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            userProperties: [123, 456],
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            userProperties: ["prop1", "prop2", null],
        };
        assertAddProcessorResult(processor3, processor4, true);
    });

    /**
     * @tc.name: AddProcessorTest013_1
     * @tc.desc: Add processor for test param eventConfigs with invalid type.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest013_1', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            eventConfigs: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            eventConfigs: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            eventConfigs: [ {domain: true,} ],
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: true} ],
        };
        assertAddProcessorResult(processor3, processor4, true);

        let processor5 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: "test_name", isRealTime: 0} ],
        };
        assertAddProcessorResult(processor4, processor5, true);

        let processor6 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: "test_name", isRealTime: true} ],
        };
        assertAddProcessorResult(processor5, processor6, false);
    });

    /**
     * @tc.name: AddProcessorTest013_2
     * @tc.desc: Add processor for test param eventConfigs with invalid format.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest013_2', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain*", name: "test_name", isRealTime: true} ],
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: "test_name*", isRealTime: true} ],
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            eventConfigs: [ {isRealTime: true} ],
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: "test_name", isRealTime: true} ],
        };
        assertAddProcessorResult(processor3, processor4, false);
    });

    /**
     * @tc.name: AddProcessorTest013_3
     * @tc.desc: Add processor for test param eventConfigs with valid format.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest013_3', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain"} ],
        };
        assertAddProcessorResult(processor0, processor1, false);

        let processor2 = {
            name: "test_processor",
            eventConfigs: [ {name: "test_name"} ],
        };
        assertAddProcessorResult(processor1, processor2, false);

        let processor3 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", isRealTime: true} ],
        };
        assertAddProcessorResult(processor2, processor3, false);

        let processor4 = {
            name: "test_processor",
            eventConfigs: [ {name: "test_name", isRealTime: true} ],
        };
        assertAddProcessorResult(processor3, processor4, false);

        let processor5 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: "", isRealTime: true} ],
        };
        assertAddProcessorResult(processor4, processor5, false);

        let processor6 = {
            name: "test_processor",
            eventConfigs: [ {domain: "", name: "test_name", isRealTime: true} ],
        };
        assertAddProcessorResult(processor5, processor6, false);

        let processor7 = {
            name: "test_processor",
            eventConfigs: [ {domain: "test_domain", name: "test_name", isRealTime: true} ],
        };
        assertAddProcessorResult(processor6, processor7, false);
        assertAddProcessorResult(processor7, processor0, false);
    });

    /**
     * @tc.name: AddProcessorTest014_1
     * @tc.desc: Add processor for test param configId.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest014_1', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            configId: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            configId: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            configId: -1,
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            configId: 0,
        };
        assertAddProcessorResult(processor3, processor4, true);

        let processor5 = {
            name: "test_processor",
            configId: 1,
        };
        assertAddProcessorResult(processor4, processor5, false);
    });


    /**
     * @tc.name: AddProcessorTest014_2
     * @tc.desc: Add processor for test param configId with the same configId and name.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest014_2', 0, function () {
        let processor1 = {
            name: "test_processor",
            configId: 1,
        };
        let processor2 = {
            name: "test_processor",
            configId: 1,
            debugMode: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            configId: 0,
            debugMode: true,
        };
        assertAddProcessorResult(processor2, processor3, false);
    });

    /**
     * @tc.name: AddProcessorTest015_1
     * @tc.desc: Add processor for test param customConfigs with invalid type.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest015_1', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        let processor1 = {
            name: "test_processor",
            customConfigs: null,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            customConfigs: true,
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            customConfigs: {},
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            customConfigs: {"test_key": true},
        };
        assertAddProcessorResult(processor3, processor4, true);

        let processor5 = {
            name: "test_processor",
            customConfigs: {"test_key": "test_value"},
        };
        assertAddProcessorResult(processor4, processor5, false);
    });

    /**
     * @tc.name: AddProcessorTest015_2
     * @tc.desc: Add processor for test param customConfigs with invalid format.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest015_2', 0, function () {
        let processor0 = {
            name: "test_processor",
        };
        const elementSize = 32 + 1;
        let bigEventConfigs = [];
        for (let i = 0; i < elementSize; i++) {
            bigEventConfigs.push({"test_key": "test_value"});
        }
        let processor1 = {
            name: "test_processor",
            customConfigs: bigEventConfigs,
        };
        assertAddProcessorResult(processor0, processor1, true);

        let processor2 = {
            name: "test_processor",
            customConfigs: {"": "test_value"},
        };
        assertAddProcessorResult(processor1, processor2, true);

        let processor3 = {
            name: "test_processor",
            customConfigs: {"test_key*": "test_value"},
        };
        assertAddProcessorResult(processor2, processor3, true);

        let processor4 = {
            name: "test_processor",
            customConfigs: {"abc012345678901234567890123456789": "test_value"},
        };
        assertAddProcessorResult(processor3, processor4, true);

        let processor5 = {
            name: "test_processor",
            customConfigs: {"test_key": 'a'.repeat(1024 + 1)},
        };
        assertAddProcessorResult(processor4, processor5, true);

        let processor6 = {
            name: "test_processor",
            customConfigs: {"test_key": "test_value"},
        };       
        assertAddProcessorResult(processor5, processor6, false);
    });

    /**
     * @tc.name: AddProcessorTest016_1
     * @tc.desc: Add processor with valid configName.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_1', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: "SDK_OCG"
            }
            assertAddProcessorResult(processor1, processor2, false);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_2
     * @tc.desc: Add processor with undefined configName.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_2', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: "undefined_name"
            }
            assertAddProcessorResult(processor1, processor2, false);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_3
     * @tc.desc: Add processor with invalid configName is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_3', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: ""
            }
            assertAddProcessorResult(processor1, processor2, true);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_4
     * @tc.desc: Add processor with invalid configName is null.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_4', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: null
            }
            assertAddProcessorResult(processor1, processor2, true);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_5
     * @tc.desc: Add processor with invalid configName is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_5', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: 0
            }
            assertAddProcessorResult(processor1, processor2, true);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_6
     * @tc.desc: Add processor with invalid configName has special char.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_6', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: "xxx***"
            }
            assertAddProcessorResult(processor1, processor2, true);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_7
     * @tc.desc: Add processor with invalid configName beginner is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_7', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: "123_processor"
            }
            assertAddProcessorResult(processor1, processor2, true);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorTest016_8
     * @tc.desc: Add processor with invalid configName length is over range.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorTest016_8', 0, function () {
        if (isConfigFileExist) {
            let processor1 = {
                name: "test_processor",
            }
            let processor2 = {
                name: "test_processor",
                configName: 'a'.repeat(256 + 1)
            }
            assertAddProcessorResult(processor1, processor2, true);
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest001
     * @tc.desc: test addProcessorFromConfig without configName when use correctly.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest001', 0, function () {
        if (isConfigFileExist) {
            hiAppEvent.addProcessorFromConfig("ha_app_event").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                expect(err).assertNull();
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest002
     * @tc.desc: test addProcessorFromConfig with configName when use correctly.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest002', 0, function () {
        if (isConfigFileExist) {
            hiAppEvent.addProcessorFromConfig("ha_app_event", "SDK_OCG").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                expect(err).assertNull();
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_1
     * @tc.desc: test addProcessorFromConfig when processorName is undefined.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_1', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("undefined").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_2
     * @tc.desc: test addProcessorFromConfig when processorName is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_2', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_3
     * @tc.desc: test addProcessorFromConfig when processorName is null.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_3', 0, function () {
        if (isConfigFileExist) {
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
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_4
     * @tc.desc: test addProcessorFromConfig when processorName is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_4', 0, function () {
        if (isConfigFileExist) {
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
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_5
     * @tc.desc: test addProcessorFromConfig when processorName is over range.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_5', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            let processorName = 'a'.repeat(256 +1);
            hiAppEvent.addProcessorFromConfig(processorName).then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_6
     * @tc.desc: test addProcessorFromConfig when processorName has special char.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_6', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("xxx***").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest003_7
     * @tc.desc: test addProcessorFromConfig when processorName beginner is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest003_7', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("123_processor").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_1
     * @tc.desc: test addProcessorFromConfig when configName is undefined.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_1', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("ha_app_event", "undefined").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_2
     * @tc.desc: test addProcessorFromConfig when configName is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_2', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("ha_app_event", "").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_3
     * @tc.desc: test addProcessorFromConfig when configName is null.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_3', 0, function () {
        if (isConfigFileExist) {
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
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_4
     * @tc.desc: test addProcessorFromConfig when configName is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_4', 0, function () {
        if (isConfigFileExist) {
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
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_5
     * @tc.desc: test addProcessorFromConfig when configName is over range.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_5', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            let configName = 'a'.repeat(256 +1);
            hiAppEvent.addProcessorFromConfig("ha_app_event", configName).then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_6
     * @tc.desc: test addProcessorFromConfig when configName has special char.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_6', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("ha_app_event", "xxx***").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });

    /**
     * @tc.name: AddProcessorFromConfigTest004_7
     * @tc.desc: test addProcessorFromConfig when configName beginner is num.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('AddProcessorFromConfigTest004_7', 0, function () {
        if (isConfigFileExist) {
            let expectErr = createError3("Invalid param value for add processor from config.");
            hiAppEvent.addProcessorFromConfig("ha_app_event", "123_processor").then((processorId) => {
                expect(processorId).assertLarger(0);
                hiAppEvent.removeProcessor(processorId);
            }).catch((err) => {
                assertErrorEqual(err, expectErr);
            })
        } else {
            console.info("processor config file is not exist.");
        }
    });
});