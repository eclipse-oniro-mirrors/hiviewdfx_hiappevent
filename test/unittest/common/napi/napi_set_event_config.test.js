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

    function createError(code, message) {
        return { code: code.toString(), message: message };
    }

    function createError2(name, type) {
        return { code: "401", message: "Parameter error. The type of " + name + " must be " + type + "." };
    }


    function assertErrorEqual(actualErr, expectErr) {
        if (expectErr) {
            expect(actualErr.code).assertEqual(expectErr.code)
            expect(actualErr.message).assertEqual(expectErr.message)
        } else {
            expect(actualErr).assertNull();
        }
    }

    function setEventConfigTest(name, configInfo, expectErr, done) {
        hiAppEvent.setEventConfig(name, configInfo).then((data) => {
            expect(data).assertEqual(0);
            done();
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
            done();
        });
    }

    function setEventConfigTestCatch(name, configInfo, expectErr, done) {
        try {
            setEventConfigTest(name, configInfo, expectErr, done);
        } catch (err) {
            assertErrorEqual(err, expectErr);
            done();
        }
    }

    function configEventPolicyTest(policy, expectErr, done) {
        hiAppEvent.configEventPolicy(policy).then(() => {
            expect(true).assertEqual(true);
            done();
        }).catch((err) => {
            assertErrorEqual(err, expectErr);
            done();
        });
    }

    /**
     * @tc.name: SetEventConfigTest001
     * @tc.desc: Test the SetEventConfig interface.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest001', 0, async function (done) {
        console.info('SetEventConfigTest001 start');
        let configInfo1 = {  // default, collect stack and trace
            "log_type": "0",
        };
        let configInfo2 = {  // collect stack
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "100",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo3 = {  // collect trace
            "log_type": "2",
        };
        setEventConfigTest("MAIN_THREAD_JANK", configInfo1, null, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo2, null, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo3, null, done);
        console.info('SetEventConfigTest001 end');
    });

    /**
     * @tc.name: SetEventConfigTest002
     * @tc.desc: Test the SetEventConfig interface with invalid name.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest002', 0, async function (done) {
        console.info('SetEventConfigTest002 start');
        let configInfo = {
            "log_type": "0",
        };
        let expectErr1 = createError2("name", "string");
        setEventConfigTestCatch(0, configInfo, expectErr1, done);
        setEventConfigTestCatch(true, configInfo, expectErr1, done);

        let expectErr2 = createError(401, "Invalid param value for event config.");
        setEventConfigTest("", configInfo, expectErr2, done);
        setEventConfigTest(null, configInfo, expectErr2, done);
        setEventConfigTest("INVALID_NAME", configInfo, expectErr2, done);
        console.info('SetEventConfigTest002 end');
    });

    /**
     * @tc.name: SetEventConfigTest003
     * @tc.desc: Test the SetEventConfig interface with invalid config type.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest003', 0, async function (done) {
        console.info('SetEventConfigTest003 start');
        let expectErr = createError2("value", "object");
        setEventConfigTestCatch("MAIN_THREAD_JANK", 0, expectErr, done);
        setEventConfigTestCatch("MAIN_THREAD_JANK", "", expectErr, done);
        setEventConfigTestCatch("MAIN_THREAD_JANK", true, expectErr, done);
        setEventConfigTestCatch("MAIN_THREAD_JANK", null, expectErr, done);
        console.info('SetEventConfigTest003 end');
    });

    /**
     * @tc.name: SetEventConfigTest004
     * @tc.desc: Test the SetEventConfig interface when the config is empty.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest004', 0, async function (done) {
        console.info('SetEventConfigTest004 start');
        let configInfo = {};
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("MAIN_THREAD_JANK", configInfo, expectErr, done);
        console.info('SetEventConfigTest004 end');
    });

    /**
     * @tc.name: SetEventConfigTest005
     * @tc.desc: Error code 401 is returned when the config log_type is invalid.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest005', 0, async function (done) {
        console.info('SetEventConfigTest005 start');
        let configInfo1 = {
            "log_type": "-1",
        };
        let configInfo2 = {
            "log_type": "abc",
        };
        let configInfo3 = {
            "log_type": "",
        };
        let configInfo4 = {
            "log_type": null,
        };
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("MAIN_THREAD_JANK", configInfo1, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo2, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo3, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo4, expectErr, done);
        console.info('SetEventConfigTest005 end');
    });

    /**
     * @tc.name: SetEventConfigTest006
     * @tc.desc: Error code 401 is returned when the config log_type=1, but item number is not 5.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest006', 0, async function (done) {
        console.info('SetEventConfigTest006 start');
        let configInfo = {
            "log_type": "1",
            "sample_interval": "100",
            "sample_count": "21",
        };
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("MAIN_THREAD_JANK", configInfo, expectErr, done);
        console.info('SetEventConfigTest006 end');
    });

    /**
     * @tc.name: SetEventConfigTest007
     * @tc.desc: Error code 401 is returned when the value param item value is invalid.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
    */
    it('SetEventConfigTest007', 0, async function (done) {
        console.info('SetEventConfigTest007 start');
        let configInfo1 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "-1",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo2 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "49",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo3 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "50",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo4 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "92233720368547758079223372036854775807",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo5 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "aa",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("MAIN_THREAD_JANK", configInfo1, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo2, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo3, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo4, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo5, expectErr, done);
        console.info('SetEventConfigTest007 end');
    });
    
    /**
     * @tc.name: SetEventConfigTest008
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest008', 0, async function (done) {
        console.info('SetEventConfigTest008 start');
        let configInfo1 = {
            "extend_pc_lr_printing": true,
        };
        let configInfo2 = {
            "log_file_cutoff_sz_bytes": 10,
        };
        let configInfo3 = {
            "simplify_vma_printing": true,
        };
        let configInfo4 = {
            "extend_pc_lr_printing": true,
            "log_file_cutoff_sz_bytes": 10,
            "simplify_vma_printing": true,
        };
        setEventConfigTest("APP_CRASH", configInfo1, null, done);
        setEventConfigTest("APP_CRASH", configInfo2, null, done);
        setEventConfigTest("APP_CRASH", configInfo3, null, done);
        setEventConfigTest("APP_CRASH", configInfo4, null, done);
        console.info('SetEventConfigTest008 end');
    });
 
    /**
     * @tc.name: SetEventConfigTest009
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH with invalid param type.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest009', 0, async function (done) {
        console.info('SetEventConfigTest009 start');
        let configInfo1 = {
            "extend_pc_lr_printing": 10,
        };
        let configInfo2 = {
            "extend_pc_lr_printing": "123abc",
        };
        let configInfo3 = {
            "extend_pc_lr_printing": null,
        };
        let configInfo4 = {
            "log_file_cutoff_sz_bytes": true,
        }
        let configInfo5 = {
            "log_file_cutoff_sz_bytes": "123abc",
        }
        let configInfo6 = {
            "log_file_cutoff_sz_bytes": null,
        }
        let configInfo7 = {
            "simplify_vma_printing": 10,
        }
        let configInfo8 = {
            "simplify_vma_printing": "123abc",
        }
        let configInfo9 = {
            "simplify_vma_printing": null,
        }
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("APP_CRASH", configInfo1, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo2, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo3, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo4, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo5, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo6, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo7, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo8, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo9, expectErr, done);
        console.info('SetEventConfigTest009 end');
    });
 
    /**
     * @tc.name: SetEventConfigTest010
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH with not all params are valid.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest010', 0, async function (done) {
        console.info('SetEventConfigTest010 start');
        let configInfo1 = {
            "extend_pc_lr_printing": true,
            "log_file_cutoff_sz_bytes": 10,
            "simplify_vma_printing": "123abc",
        };
        setEventConfigTest("APP_CRASH", configInfo1, null, done);
        console.info('SetEventConfigTest010 end');
    });
 
    /**
     * @tc.name: SetEventConfigTest011
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH with all params are invalid.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest011', 0, async function (done) {
        console.info('SetEventConfigTest011 start');
        let configInfo1 = {
            "extend_pc_lr_printing": 10,
            "log_file_cutoff_sz_bytes": true,
            "simplify_vma_printing": 10,
        };
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("APP_CRASH", configInfo1, expectErr, done);
        console.info('SetEventConfigTest011 end');
    });
 
    /**
     * @tc.name: SetEventConfigTest012
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH with numeric param.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest012', 0, async function (done) {
        console.info('SetEventConfigTest012 start');
        let configInfo1 = {
            "log_file_cutoff_sz_bytes": -1,
        };
        let configInfo2 = {
            "log_file_cutoff_sz_bytes": 0,
        };
        let configInfo3 = {
            "log_file_cutoff_sz_bytes": 5 * 1024 * 1024,
        };
        let configInfo4 = {
            "log_file_cutoff_sz_bytes": 5 * 1024 * 1024 + 1,
        };
        let configInfo5 = {
            "log_file_cutoff_sz_bytes": 123.456,
        };
        let configInfo6 = {
            "log_file_cutoff_sz_bytes": "123",
        };
        let configInfo7 = {
            "log_file_cutoff_sz_bytes": "123.456",
        };
        let configInfo8 = {
            "log_file_cutoff_sz_bytes": "123.456.789",
        };
        let configInfo9 = {
            "log_file_cutoff_sz_bytes": "",
        };
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("APP_CRASH", configInfo1, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo2, null, done);
        setEventConfigTest("APP_CRASH", configInfo3, null, done);
        setEventConfigTest("APP_CRASH", configInfo4, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo5, null, done);
        setEventConfigTest("APP_CRASH", configInfo6, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo7, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo8, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo9, expectErr, done);
        console.info('SetEventConfigTest012 end');
    });
 
    /**
     * @tc.name: SetEventConfigTest013
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH with invalid param item.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest013', 0, async function (done) {
        console.info('SetEventConfigTest013 start');
        let configInfo1 = {
            "extend_pc_lr_printing": true,
            "testKey": "testValue",
        };
        setEventConfigTest("APP_CRASH", configInfo1, null, done);
        console.info('SetEventConfigTest013 end');
    });

    /**
     * @tc.name: SetEventConfigTest014
     * @tc.desc: Test the SetEventConfig interface for APP_CRASH with boolean param.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest014', 0, async function (done) {
        console.info('SetEventConfigTest014 start');
        let configInfo1 = {
            "extend_pc_lr_printing": true,
        };
        let configInfo2 = {
            "extend_pc_lr_printing": "true",
        };
        let configInfo3 = {
            "simplify_vma_printing": true,
        };
        let configInfo4 = {
            "simplify_vma_printing": "true",
        };
 
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("APP_CRASH", configInfo1, null, done);
        setEventConfigTest("APP_CRASH", configInfo2, expectErr, done);
        setEventConfigTest("APP_CRASH", configInfo3, null, done);
        setEventConfigTest("APP_CRASH", configInfo4, expectErr, done);
        console.info('SetEventConfigTest014 end');
    });
 
    /**
     * @tc.name: SetEventConfigTest015
     * @tc.desc: Test the SetEventConfig interface for MAIN_THREAD_JANK with string param.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('SetEventConfigTest015', 0, async function (done) {
        console.info('SetEventConfigTest015 start');
        let configInfo1 = {
            "log_type": "0",
        };
        let configInfo2 = {
            "log_type": 0,
        };
        let configInfo3 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "100",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo4 = {
            "log_type": "1",
            "ignore_startup_time": 10,
            "sample_interval": "100",
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo5 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": 100,
            "sample_count": "21",
            "report_times_per_app": "3"
        };
        let configInfo6 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "100",
            "sample_count": 21,
            "report_times_per_app": "3"
        };
        let configInfo7 = {
            "log_type": "1",
            "ignore_startup_time": "10",
            "sample_interval": "100",
            "sample_count": "21",
            "report_times_per_app": 3
        };
 
        let expectErr = createError(401, "Invalid param value for event config.");
        setEventConfigTest("MAIN_THREAD_JANK", configInfo1, null, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo2, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo3, null, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo4, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo5, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo6, expectErr, done);
        setEventConfigTest("MAIN_THREAD_JANK", configInfo7, expectErr, done);
        console.info('SetEventConfigTest015 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest001
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest001', 0, async function (done) {
        console.info('ConfigEventPolicyTest001 start');
        let policy = {
            'mainThreadJankPolicy': {
              "logType": 1,
              "ignoreStartupTime": 11,
              "sampleInterval": 100,
              "sampleCount": 21,
              "reportTimesPerApp": 3,
              "autoStopSampling": true
            }
        }
        configEventPolicyTest(policy, null, done);   
        console.info('ConfigEventPolicyTest001 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest002
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest002', 0, async function (done) {
        console.info('ConfigEventPolicyTest002 start');
        let policy = {
            'mainThreadJankPolicy': {
              "logType": "1",
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);   
        console.info('ConfigEventPolicyTest002 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest003
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest003', 0, async function (done) {
        console.info('ConfigEventPolicyTest003 start');
        let policy = {
            'mainThreadJankPolicy': {
              "ignoreStartupTime": "11",
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);   
        console.info('ConfigEventPolicyTest003 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest004
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest004', 0, async function (done) {
        console.info('ConfigEventPolicyTest004 start');
        let policy = {
            'mainThreadJankPolicy': {
              "sampleInterval": "100",
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);   
        console.info('ConfigEventPolicyTest004 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest005
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest005', 0, async function (done) {
        console.info('ConfigEventPolicyTest005 start');
        let policy = {
            'mainThreadJankPolicy': {
              "sampleCount": "21",
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);   
        console.info('ConfigEventPolicyTest005 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest006
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest006', 0, async function (done) {
        console.info('ConfigEventPolicyTest006 start');
        let policy = {
            'mainThreadJankPolicy': {
              "reportTimesPerApp": "3",
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);   
        console.info('ConfigEventPolicyTest006 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest007
     * @tc.desc: Test the ConfigEventPolicy interface for MAIN_THREAD_JANK with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest007', 0, async function (done) {
        console.info('ConfigEventPolicyTest007 start');
        let policy = {
            'mainThreadJankPolicy': {
              "autoStopSampling": "true",
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);   
        console.info('ConfigEventPolicyTest007 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest008
     * @tc.desc: Test the ConfigEventPolicy interface for CPU_USAGE_HIGH.
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest008', 0, async function (done) {
        console.info('ConfigEventPolicyTest008 start');
        let policy = {
            "foregroundLoadThreshold": 11,  // [0, 100]
            "backgroundLoadThreshold": 22,
            "threadLoadThreshold": 33,
            "perfLogCaptureCount": 44,  // [-1, 100]
            "threadLoadInterval": 55,  // [5,60]
        }
        configEventPolicyTest(policy, null, done);   
        console.info('ConfigEventPolicyTest008 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest009
     * @tc.desc: Test the ConfigEventPolicy interface for CPU_USAGE_HIGH with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest009', 0, async function (done) {
        console.info('ConfigEventPolicyTest009 start');
        let policy = {
            "foregroundLoadThreshold": "11",
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);
        console.info('ConfigEventPolicyTest009 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest010
     * @tc.desc: Test the ConfigEventPolicy interface for CPU_USAGE_HIGH with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest010', 0, async function (done) {
        console.info('ConfigEventPolicyTest010 start');
        let policy = {
            "backgroundLoadThreshold": "22",
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);
        console.info('ConfigEventPolicyTest010 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest011
     * @tc.desc: Test the ConfigEventPolicy interface for CPU_USAGE_HIGH with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest011', 0, async function (done) {
        console.info('ConfigEventPolicyTest011 start');
        let policy = {
            "threadLoadThreshold": "33",
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);
        console.info('ConfigEventPolicyTest011 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest012
     * @tc.desc: Test the ConfigEventPolicy interface for CPU_USAGE_HIGH with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest012', 0, async function (done) {
        console.info('ConfigEventPolicyTest012 start');
        let policy = {
            "perfLogCaptureCount": "44",
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);
        console.info('ConfigEventPolicyTest012 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest013
     * @tc.desc: Test the ConfigEventPolicy interface for CPU_USAGE_HIGH with invalid policy item type
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest013', 0, async function (done) {
        console.info('ConfigEventPolicyTest013 start');
        let policy = {
            "threadLoadInterval": "55"
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);
        console.info('ConfigEventPolicyTest013 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest014
     * @tc.desc: Test the ConfigEventPolicy interface for empty policy struct
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest014', 0, async function (done) {
        console.info('ConfigEventPolicyTest014 start');
        let policy = {}
        configEventPolicyTest(policy, null, done);
        policy = {
            'mainThreadJankPolicy': {}
        }
        configEventPolicyTest(policy, null, done);
        console.info('ConfigEventPolicyTest014 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest015
     * @tc.desc: Test the ConfigEventPolicy interface for config Multiple policy at the same time
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest015', 0, async function (done) {
        console.info('ConfigEventPolicyTest015 start');
        let policy = {
            'mainThreadJankPolicy': {
              "logType": 0,
            },
            'cpuUsageHighPolicy': {
              "foregroundLoadThreshold": 11,
            }
        }
        configEventPolicyTest(policy, null, done);
        console.info('ConfigEventPolicyTest015 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest016
     * @tc.desc: Test the ConfigEventPolicy interface for config Multiple policy at the same time with a param invalid
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest016', 0, async function (done) {
        console.info('ConfigEventPolicyTest016 start');
        let policy = {
            'mainThreadJankPolicy': {
              "logType": "0",
            },
            'cpuUsageHighPolicy': {
              "foregroundLoadThreshold": 11,
            }
        }
        let expectErr = createError(401, "Invalid param value type for event policy.");
        configEventPolicyTest(policy, expectErr, done);
        console.info('ConfigEventPolicyTest016 end');
    });

    /**
     * @tc.name: ConfigEventPolicyTest017
     * @tc.desc: Test the ConfigEventPolicy interface for config Multiple policy at the same time with one failed
     * @tc.type: FUNC
     * @tc.require: issueI8U2VO
     */
    it('ConfigEventPolicyTest017', 0, async function (done) {
        console.info('ConfigEventPolicyTest017 start');
        let policy = {
            'mainThreadJankPolicy': {
              "logType": 3,
            },
            'cpuUsageHighPolicy': {
              "foregroundLoadThreshold": 11,
            }
        }
        hiAppEvent.configEventPolicy(policy).then(() => {
            expect(false).assertEqual(true);
            done();
        }).catch(() => {
            expect(true).assertEqual(true);
            done();
        });
        console.info('ConfigEventPolicyTest017 end');
    });
});