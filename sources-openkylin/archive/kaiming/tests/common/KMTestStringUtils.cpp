/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 *
 * See the LICENSE file for more details.
 */

#include <gtest/gtest.h>
#include "common/KMStringUtils.h"

TEST(KMStringUtilsTest, StartsWith)
{
    EXPECT_TRUE(KMStringUtils::startsWith("hello world", "hello"));
    EXPECT_FALSE(KMStringUtils::startsWith("hello world", "world"));
}

TEST(KMStringUtilsTest, EndsWith)
{
    EXPECT_TRUE(KMStringUtils::endsWith("hello world", "world"));
    EXPECT_FALSE(KMStringUtils::endsWith("hello world", "hello"));
}

TEST(KMStringUtilsTest, StartsWithForRef)
{
    EXPECT_TRUE(KMStringUtils::startsWithForRef("runtime-manager", "runtime"));
    EXPECT_FALSE(KMStringUtils::startsWithForRef("application", "manager"));
}

TEST(KMStringUtilsTest, EndsWithForRef)
{
    EXPECT_TRUE(KMStringUtils::endsWithForRef("config.Debug", ".Debug"));
    EXPECT_FALSE(KMStringUtils::endsWithForRef("config.Debug", ".Release"));
}

TEST(KMStringUtilsTest, SplitString)
{
    std::string input = "a,b,c";
    std::vector<std::string> result = KMStringUtils::splitString(input, ",");
    std::vector<std::string> expected = { "a", "b", "c" };
    EXPECT_EQ(result, expected);

    EXPECT_EQ(KMStringUtils::splitString("", ","), std::vector<std::string>{});
    EXPECT_EQ(KMStringUtils::splitString("a--b--c", "--"), std::vector<std::string>({ "a", "b", "c" }));
}

TEST(KMStringUtilsTest, IsNumeric)
{
    EXPECT_TRUE(KMStringUtils::isNumeric("123456"));
    EXPECT_FALSE(KMStringUtils::isNumeric("123abc"));
}

TEST(KMStringUtilsTest, IsValidPrerelease)
{
    EXPECT_FALSE(KMStringUtils::isValidPrerelease("alpha.1"));
    EXPECT_FALSE(KMStringUtils::isValidPrerelease("!invalid@")); // 非法字符
}

TEST(KMStringUtilsTest, Strstrip)
{
    EXPECT_EQ(KMStringUtils::strstrip("   hello world   "), "hello world");
    EXPECT_EQ(KMStringUtils::strstrip("test"), "test");
}

TEST(KMStringUtilsTest, JoinVector)
{
    std::vector<std::string> input = { "a", "b", "c" };
    std::string result = KMStringUtils::join(input, "-");
    EXPECT_EQ(result, "a-b-c");

    EXPECT_EQ(KMStringUtils::join(std::vector<std::string>{}, ","), "");
    EXPECT_EQ(KMStringUtils::join(std::vector<std::string>{ "only" }, ","), "only");
}

TEST(KMStringUtilsTest, JoinCharArray)
{
    const char *arr1[] = { "x", "y", "z", nullptr };
    EXPECT_EQ(KMStringUtils::join(arr1, "|"), "x|y|z");

    const char *arr2[] = { nullptr };
    EXPECT_EQ(KMStringUtils::join(arr2, ","), "");

    EXPECT_EQ(KMStringUtils::join(nullptr, ","), "");
}

TEST(KMStringUtilsTest, JoinList)
{
    std::list<std::string> input = { "1", "2", "3" };
    EXPECT_EQ(KMStringUtils::join(input, ":"), "1:2:3");

    EXPECT_EQ(KMStringUtils::join(std::list<std::string>{}, ";"), "");
    EXPECT_EQ(KMStringUtils::join(std::list<std::string>{ "single" }, ";"), "single");
}

TEST(KMStringUtilsTest, ContainsVectorString)
{
    std::vector<std::string> data = { "apple", "banana", "cherry" };
    EXPECT_TRUE(KMStringUtils::contains(data, "banana"));
    EXPECT_FALSE(KMStringUtils::contains(data, "grape"));
}

TEST(KMStringUtilsTest, ContainsSubstring)
{
    EXPECT_TRUE(KMStringUtils::contains("hello world", "world"));
}

TEST(KMStringUtilsTest, ReplaceOptionalString)
{
    std::optional<std::string> str = "foo bar foo";
    KMStringUtils::replace(str, "foo", "baz");
    ASSERT_TRUE(str.has_value());
    EXPECT_EQ(str.value(), "baz bar baz");

    std::optional<std::string> emptyStr;
    KMStringUtils::replace(emptyStr, "a", "b"); // should not crash
    EXPECT_FALSE(emptyStr.has_value());

    std::optional<std::string> noMatch = "hello";
    KMStringUtils::replace(noMatch, "x", "y");
    EXPECT_EQ(noMatch.value(), "hello");
}

TEST(KMStringUtilsTest, ReplaceVectorString)
{
    std::vector<std::string> vec = { "apple pie", "pineapple", "banana" };
    KMStringUtils::replace(vec, "apple", "orange");
    EXPECT_EQ(vec[0], "orange pie");
    EXPECT_EQ(vec[1], "pineorange");
    EXPECT_EQ(vec[2], "banana");
}

TEST(KMStringUtilsTest, ReplaceOptionalVectorSimple)
{
    std::optional<std::vector<std::string>> strs = std::vector<std::string>{ "abc_from_xyz", "from_here" };
    KMStringUtils::replace(strs, "from", "to");

    ASSERT_TRUE(strs.has_value());
    EXPECT_EQ(strs->at(0), "abc_to_xyz");
    EXPECT_EQ(strs->at(1), "to_here");
}

TEST(KMStringUtilsTest, BuildFilename_TwoParts)
{
    std::string result = KMStringUtils::buildFilename("path", "folder", "file.txt");
    EXPECT_EQ(result, "path/folder/file.txt");
}

TEST(KMStringUtilsTest, BuildFilename_ThreeParts)
{
    std::string result = KMStringUtils::buildFilename("path", "folder", "subfolder", "file.txt");
    EXPECT_EQ(result, "path/folder/subfolder/file.txt");
}

TEST(KMStringUtilsTest, BuildFilename_FourParts)
{
    std::string result = KMStringUtils::buildFilename("path", "folder", "subfolder", "extra", "file.txt");
    EXPECT_EQ(result, "path/folder/subfolder/extra/file.txt");
}

TEST(KMStringUtilsTest, SplitStringrByDelimiter)
{
    std::string input = "a,b,c";
    char delimiter = ',';
    std::vector<std::string> result = KMStringUtils::splitStringrByDelimiter(input, delimiter);
    EXPECT_EQ(result.size(), 3);
}

TEST(KMStringUtilsTest, IsValidVersion)
{
    EXPECT_TRUE(KMStringUtils::isValidVersion("1.2.3.4"));
    EXPECT_FALSE(KMStringUtils::isValidVersion("1.2.3"));
}

TEST(KMStringUtilsTest, SplitVersion)
{
    std::vector<int> result = KMStringUtils::splitVersion("1.2.3.4");
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 4);
}

TEST(KMStringUtilsTest, CompareVersions)
{
    // 正常版本号比较
    EXPECT_EQ(KMStringUtils::compareVersions("1.2.3.4", "1.2.3.4"), 0); // 相等

    // 非法版本号
    EXPECT_THROW(KMStringUtils::compareVersions("1.2.3", "1.2.3.4"), std::invalid_argument);
}

TEST(KMStringUtilsTest, ParseType)
{
    // 测试不同的ID类型
    EXPECT_EQ(KMStringUtils::parseType("myapp.Platform"), BASE_TYPE_APP);
    EXPECT_EQ(KMStringUtils::parseType("myapp.base"), BASE_TYPE_APP);
    EXPECT_EQ(KMStringUtils::parseType("myapp.app"), BASE_TYPE_APP);    // 默认类型
    EXPECT_EQ(KMStringUtils::parseType("myapp.custom"), BASE_TYPE_APP); // 默认类型
}