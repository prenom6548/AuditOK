#include "KMJsonHelper.h"
#include <gtest/gtest.h>

// 测试 getInt64Value 方法
TEST(KMJsonHelperTest, GetInt64ValueTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData = {
        { "key1", 12345 },
        { "key2", 67890 }
    };
    KMJsonHelper jsonHelper(jsonData);

    // 测试获取存在的 key 的值
    EXPECT_EQ(jsonHelper.getInt64Value("key1", 0), 12345);
    EXPECT_EQ(jsonHelper.getInt64Value("key2", 0), 67890);

    // 测试获取不存在的 key 的默认值
    EXPECT_EQ(jsonHelper.getInt64Value("nonexistent_key", 0), 0);
}

// 测试 getIntValue 方法
TEST(KMJsonHelperTest, GetIntValueTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData = {
        { "key1", 12345 },
        { "key2", 67890 }
    };
    KMJsonHelper jsonHelper(jsonData);

    // 测试获取存在的 key 的值
    EXPECT_EQ(jsonHelper.getIntValue("key1", 0), 12345);
    EXPECT_EQ(jsonHelper.getIntValue("key2", 0), 67890);

    // 测试获取不存在的 key 的默认值
    EXPECT_EQ(jsonHelper.getIntValue("nonexistent_key", 0), 0);
}

// 测试 setBoolValue 方法
TEST(KMJsonHelperTest, SetBoolValueTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData;
    KMJsonHelper jsonHelper(jsonData);

    // 设置 key 为 "key1"，值为 true
    jsonHelper.setBoolValue("key1", true, true);
    EXPECT_EQ(jsonHelper.getBoolValue("key1", false), true);

    // 设置 key 为 "key2"，值为 false
    jsonHelper.setBoolValue("key2", false, true);
    EXPECT_EQ(jsonHelper.getBoolValue("key2", true), false);
}

// 测试 setStringListValue 方法
TEST(KMJsonHelperTest, SetStringListValueTest)
{
    nlohmann::json jsonData;
    KMJsonHelper jsonHelper(jsonData);

    // 测试设置非空字符串列表
    std::vector<std::string> list = { "value1", "value2", "value3" };
    jsonHelper.setStringListValue("key1", list);
    auto result = jsonHelper.getStringListValue("key1");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "value1");
    EXPECT_EQ(result[1], "value2");
    EXPECT_EQ(result[2], "value3");

    // 测试设置空列表
    std::vector<std::string> emptyList = {};
    jsonHelper.setStringListValue("key2", emptyList);
    EXPECT_EQ(jsonHelper.getStringListValue("key2").size(), 0); // 应该返回空列表

    // 测试设置空列表时 key 不存在
    jsonHelper.setStringListValue("key3", emptyList);
    EXPECT_EQ(jsonHelper.getStringListValue("key3").size(), 0); // 应该返回空列表

    // 测试设置非空列表时 key3 存在
    jsonHelper.setStringListValue("key3", list);
    result = jsonHelper.getStringListValue("key3");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "value1");
}

// 测试 setStringMapValue 方法
TEST(KMJsonHelperTest, SetStringMapValueTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData;
    KMJsonHelper jsonHelper(jsonData);

    // 测试设置非空 map
    std::map<std::string, std::string> stringMap = { { "key1", "value1" }, { "key2", "value2" } };
    jsonHelper.setStringMapValue("key1", stringMap);
    auto result = jsonHelper.getStringMapValue("key1");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["key1"], "value1");
    EXPECT_EQ(result["key2"], "value2");

    // 测试设置空 map，不应改变 JSON
    std::map<std::string, std::string> emptyMap;
    jsonHelper.setStringMapValue("key2", emptyMap);
    EXPECT_EQ(jsonHelper.getStringMapValue("key2").size(), 0);

    // 测试设置非空 map 时 key 已经存在，应该更新值
    std::map<std::string, std::string> newMap = { { "key3", "newValue" } };
    jsonHelper.setStringMapValue("key1", newMap);
    result = jsonHelper.getStringMapValue("key1");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["key3"], "newValue");
}

// 测试 setStringValue 方法
TEST(KMJsonHelperTest, SetStringValueTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData;
    KMJsonHelper jsonHelper(jsonData);

    // 测试设置非空字符串值
    jsonHelper.setStringValue("key1", "value1");
    EXPECT_EQ(jsonHelper.getStringValue("key1", ""), "value1");

    // 测试设置空字符串，不应改变 JSON
    jsonHelper.setStringValue("key2", "");
    EXPECT_EQ(jsonHelper.getStringValue("key2", ""), "");
}

// 测试 setUInt64Value 方法
TEST(KMJsonHelperTest, SetUInt64ValueTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData;
    KMJsonHelper jsonHelper(jsonData);

    // 设置 key 为 "key1"，值为一个非零的 uint64_t 数值
    jsonHelper.setUInt64Value("key1", 123456789);
    EXPECT_EQ(jsonHelper.getUInt64Value("key1", 0), 123456789);
}

// 测试 add 方法
TEST(KMJsonHelperTest, AddMethodTest)
{
    // 创建一个包含 JSON 数据的 KMJsonHelper 对象
    nlohmann::json jsonData;
    KMJsonHelper jsonHelper(jsonData);

    // 创建一个 map 并添加数据
    std::map<std::string, std::string> testMap = { { "key1", "value1" }, { "key2", "value2" } };
    jsonHelper.add(testMap);

    // 验证 map 是否成功添加
    EXPECT_EQ(jsonHelper.getStringValue("key1", ""), "value1");
    EXPECT_EQ(jsonHelper.getStringValue("key2", ""), "value2");
}