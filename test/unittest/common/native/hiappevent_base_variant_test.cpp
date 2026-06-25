/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include <variant>
#include <vector>

#include "hiappevent_base.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

namespace {
class HiAppEventBaseVariantTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};
}

/**
 * @tc.name: AppEventParamValue_VariantTypeIndex001
 * @tc.desc: check the variant index matches AppEventParamType for scalar types.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParamValue_VariantTypeIndex001, TestSize.Level0)
{
    AppEventParamValue value = std::monostate{};
    EXPECT_EQ(value.index(), AppEventParamType::EMPTY);

    value = true;
    EXPECT_EQ(value.index(), AppEventParamType::BOOL);

    value = 'a';
    EXPECT_EQ(value.index(), AppEventParamType::CHAR);

    value = static_cast<int16_t>(10);
    EXPECT_EQ(value.index(), AppEventParamType::SHORT);

    value = 42;
    EXPECT_EQ(value.index(), AppEventParamType::INTEGER);

    value = static_cast<int64_t>(100);
    EXPECT_EQ(value.index(), AppEventParamType::LONGLONG);

    value = 1.0f;
    EXPECT_EQ(value.index(), AppEventParamType::FLOAT);

    value = 3.14;
    EXPECT_EQ(value.index(), AppEventParamType::DOUBLE);

    value = std::string("hello");
    EXPECT_EQ(value.index(), AppEventParamType::STRING);
}

/**
 * @tc.name: AppEventParamValue_VariantTypeIndex002
 * @tc.desc: check the variant index matches AppEventParamType for vector types.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParamValue_VariantTypeIndex002, TestSize.Level0)
{
    AppEventParamValue value = std::vector<bool>{true, false};
    EXPECT_EQ(value.index(), AppEventParamType::BVECTOR);

    value = std::vector<char>{'a', 'b'};
    EXPECT_EQ(value.index(), AppEventParamType::CVECTOR);

    value = std::vector<int16_t>{1, 2};
    EXPECT_EQ(value.index(), AppEventParamType::SHVECTOR);

    value = std::vector<int>{10, 20};
    EXPECT_EQ(value.index(), AppEventParamType::IVECTOR);

    value = std::vector<int64_t>{100, 200};
    EXPECT_EQ(value.index(), AppEventParamType::LLVECTOR);

    value = std::vector<float>{1.0f, 2.0f};
    EXPECT_EQ(value.index(), AppEventParamType::FVECTOR);

    value = std::vector<double>{3.14, 2.71};
    EXPECT_EQ(value.index(), AppEventParamType::DVECTOR);

    value = std::vector<std::string>{"a", "b"};
    EXPECT_EQ(value.index(), AppEventParamType::STRVECTOR);
}

/**
 * @tc.name: AppEventParamValue_GetIf001
 * @tc.desc: check std::get_if extracts the correct int value and returns nullptr for wrong type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParamValue_GetIf001, TestSize.Level0)
{
    AppEventParamValue value = 42;
    auto* ptr = std::get_if<int>(&value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);

    auto* strPtr = std::get_if<std::string>(&value);
    EXPECT_EQ(strPtr, nullptr);
}

/**
 * @tc.name: AppEventParamValue_GetIf002
 * @tc.desc: check std::get_if extracts the correct string value and returns nullptr for wrong type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParamValue_GetIf002, TestSize.Level0)
{
    AppEventParamValue value = std::string("testStr");
    auto* ptr = std::get_if<std::string>(&value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "testStr");

    auto* intPtr = std::get_if<int>(&value);
    EXPECT_EQ(intPtr, nullptr);
}

/**
 * @tc.name: AppEventParamValue_GetIf003
 * @tc.desc: check std::get_if extracts monostate and returns nullptr for int type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParamValue_GetIf003, TestSize.Level0)
{
    AppEventParamValue value = std::monostate{};
    auto* ptr = std::get_if<std::monostate>(&value);
    EXPECT_NE(ptr, nullptr);

    auto* intPtr = std::get_if<int>(&value);
    EXPECT_EQ(intPtr, nullptr);
}

/**
 * @tc.name: AppEventParamValue_GetIf004
 * @tc.desc: check std::get_if extracts vector content and returns nullptr for wrong vector type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParamValue_GetIf004, TestSize.Level0)
{
    AppEventParamValue value = std::vector<int>{10, 20, 30};
    auto* ptr = std::get_if<std::vector<int>>(&value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->size(), 3u);
    EXPECT_EQ((*ptr)[0], 10);
    EXPECT_EQ((*ptr)[1], 20);
    EXPECT_EQ((*ptr)[2], 30);

    auto* strVecPtr = std::get_if<std::vector<std::string>>(&value);
    EXPECT_EQ(strVecPtr, nullptr);
}

/**
 * @tc.name: AppEventParam_Construct001
 * @tc.desc: check AppEventParam construction with int type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Construct001, TestSize.Level0)
{
    AppEventParam param("testKey", 42);
    EXPECT_EQ(param.name, "testKey");
    EXPECT_EQ(param.value.index(), AppEventParamType::INTEGER);
    auto* ptr = std::get_if<int>(&param.value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

/**
 * @tc.name: AppEventParam_Construct002
 * @tc.desc: check AppEventParam construction with string type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Construct002, TestSize.Level0)
{
    AppEventParam param("strKey", std::string("testValue"));
    EXPECT_EQ(param.name, "strKey");
    EXPECT_EQ(param.value.index(), AppEventParamType::STRING);
    auto* ptr = std::get_if<std::string>(&param.value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "testValue");
}

/**
 * @tc.name: AppEventParam_Construct003
 * @tc.desc: check AppEventParam construction with monostate (EMPTY) type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Construct003, TestSize.Level0)
{
    AppEventParam param("emptyKey", std::monostate{});
    EXPECT_EQ(param.name, "emptyKey");
    EXPECT_EQ(param.value.index(), AppEventParamType::EMPTY);
}

/**
 * @tc.name: AppEventParam_Construct004
 * @tc.desc: check AppEventParam construction with bool type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Construct004, TestSize.Level0)
{
    AppEventParam param("boolKey", true);
    EXPECT_EQ(param.name, "boolKey");
    EXPECT_EQ(param.value.index(), AppEventParamType::BOOL);
    auto* ptr = std::get_if<bool>(&param.value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, true);
}

/**
 * @tc.name: AppEventParam_Construct005
 * @tc.desc: check AppEventParam construction with int vector type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Construct005, TestSize.Level0)
{
    AppEventParam param("vecKey", std::vector<int>{1, 2, 3});
    EXPECT_EQ(param.name, "vecKey");
    EXPECT_EQ(param.value.index(), AppEventParamType::IVECTOR);
    auto* ptr = std::get_if<std::vector<int>>(&param.value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->size(), 3u);
}

/**
 * @tc.name: AppEventParam_Copy001
 * @tc.desc: check AppEventParam copy constructor with string type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Copy001, TestSize.Level0)
{
    AppEventParam param1("origKey", std::string("origValue"));
    AppEventParam param2(param1);
    EXPECT_EQ(param2.name, "origKey");
    EXPECT_EQ(param2.value.index(), AppEventParamType::STRING);
    auto* ptr = std::get_if<std::string>(&param2.value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, "origValue");
}

/**
 * @tc.name: AppEventParam_Copy002
 * @tc.desc: check AppEventParam copy constructor with string vector type value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventParam_Copy002, TestSize.Level0)
{
    AppEventParam param1("vecKey", std::vector<std::string>{"a", "b"});
    AppEventParam param2(param1);
    EXPECT_EQ(param2.name, "vecKey");
    EXPECT_EQ(param2.value.index(), AppEventParamType::STRVECTOR);
    auto* ptr = std::get_if<std::vector<std::string>>(&param2.value);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->size(), 2u);
    EXPECT_EQ((*ptr)[0], "a");
    EXPECT_EQ((*ptr)[1], "b");
}

/**
 * @tc.name: AppEventPack_AddParam_Bool001
 * @tc.desc: check AddParam with bool value stores correct variant type and value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Bool001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("boolKey", true);
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "boolKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::BOOL);
            auto* ptr = std::get_if<bool>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(*ptr, true);
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_Int001
 * @tc.desc: check AddParam with int value stores correct variant type and value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Int001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("intKey", 42);
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "intKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::INTEGER);
            auto* ptr = std::get_if<int>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(*ptr, 42);
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_Int64_001
 * @tc.desc: check AddParam with int64_t value stores correct variant type and value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Int64_001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("int64Key", static_cast<int64_t>(999));
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "int64Key") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::LONGLONG);
            auto* ptr = std::get_if<int64_t>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(*ptr, static_cast<int64_t>(999));
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_String001
 * @tc.desc: check AddParam with string value stores correct variant type and value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_String001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("strKey", std::string("testStr"));
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "strKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::STRING);
            auto* ptr = std::get_if<std::string>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(*ptr, "testStr");
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_Float001
 * @tc.desc: check AddParam with float value stores correct variant type and value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Float001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("floatKey", 1.5f);
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "floatKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::FLOAT);
            auto* ptr = std::get_if<float>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_FLOAT_EQ(*ptr, 1.5f);
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_Double001
 * @tc.desc: check AddParam with double value stores correct variant type and value.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Double001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("doubleKey", 3.14);
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "doubleKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::DOUBLE);
            auto* ptr = std::get_if<double>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_DOUBLE_EQ(*ptr, 3.14);
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_VecInt001
 * @tc.desc: check AddParam with int vector stores correct variant type and size.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_VecInt001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    std::vector<int> vec = {10, 20, 30};
    pack.AddParam("vecIntKey", vec);
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "vecIntKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::IVECTOR);
            auto* ptr = std::get_if<std::vector<int>>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(ptr->size(), 3u);
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_VecStr001
 * @tc.desc: check AddParam with string vector stores correct variant type and element values.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_VecStr001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    std::vector<std::string> vec = {"a", "b", "c"};
    pack.AddParam("vecStrKey", vec);
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "vecStrKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::STRVECTOR);
            auto* ptr = std::get_if<std::vector<std::string>>(&param.value);
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(ptr->size(), 3u);
            EXPECT_EQ((*ptr)[0], "a");
            EXPECT_EQ((*ptr)[1], "b");
            EXPECT_EQ((*ptr)[2], "c");
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_Empty001
 * @tc.desc: check AddParam with no value stores EMPTY variant type.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Empty001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("emptyKey");
    auto params = pack.GetBaseParams();
    bool found = false;
    for (const auto& param : params) {
        if (param.name == "emptyKey") {
            found = true;
            EXPECT_EQ(param.value.index(), AppEventParamType::EMPTY);
        }
    }
    EXPECT_TRUE(found);
}

/**
 * @tc.name: AppEventPack_AddParam_Mixed001
 * @tc.desc: check AddParam with mixed types stores correct variant indices for each param.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_AddParam_Mixed001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("boolKey", true);
    pack.AddParam("intKey", 42);
    pack.AddParam("strKey", std::string("hello"));
    pack.AddParam("vecKey", std::vector<int>{1, 2, 3});

    auto params = pack.GetBaseParams();
    EXPECT_EQ(params.size(), 4u);

    std::map<std::string, size_t> typeMap;
    for (const auto& param : params) {
        typeMap[param.name] = param.value.index();
    }
    EXPECT_EQ(typeMap["boolKey"], AppEventParamType::BOOL);
    EXPECT_EQ(typeMap["intKey"], AppEventParamType::INTEGER);
    EXPECT_EQ(typeMap["strKey"], AppEventParamType::STRING);
    EXPECT_EQ(typeMap["vecKey"], AppEventParamType::IVECTOR);
}

/**
 * @tc.name: AppEventPack_GetParamStr001
 * @tc.desc: check GetParamStr serializes int and string params into JSON string.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_GetParamStr001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("intKey", 42);
    pack.AddParam("strKey", std::string("hello"));
    std::string paramStr = pack.GetParamStr();
    EXPECT_FALSE(paramStr.empty());
    EXPECT_NE(paramStr.find("intKey"), std::string::npos);
    EXPECT_NE(paramStr.find("strKey"), std::string::npos);
}

/**
 * @tc.name: AppEventPack_GetCustomParams001
 * @tc.desc: check GetCustomParams extracts type field from variant index correctly.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventBaseVariantTest, AppEventPack_GetCustomParams001, TestSize.Level0)
{
    AppEventPack pack("testDomain", "testName", 1);
    pack.AddParam("strParam", std::string("value1"));
    pack.AddParam("intParam", 100);

    std::vector<CustomEventParam> customParams;
    pack.GetCustomParams(customParams);
    EXPECT_GE(customParams.size(), 2u);

    for (const auto& cp : customParams) {
        if (cp.key == "strParam") {
            EXPECT_EQ(cp.type, AppEventParamType::STRING);
        } else if (cp.key == "intParam") {
            EXPECT_EQ(cp.type, AppEventParamType::INTEGER);
        }
    }
}
