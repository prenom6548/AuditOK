/*
 * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */
#include <stdio.h>
#include <string.h>

#include <gtest/gtest.h>
#include "../cpp_stub/src/stub.h"
#include "../../utils/kyutils.h"
#include "../../utils/cstring-extension.h"

TEST(TestSuite1,kdkVolumeBaseCharacterConvert_1)
{
    int ret = 0;

    // 调用字符型数据单位转换接口
    char  origin_data [20] = "10000.24MB";
    char result_data [50] = {0};
    ret = kdkVolumeBaseCharacterConvert(origin_data, KDK_GIGABYTE, result_data);
    printf("%s\n", result_data);
}

TEST(TestSuite2, kdkVolumeBaseNumericalConvert_1)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_KILOBYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite3, kdkVolumeBaseNumericalConvert_2)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_MEGABYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite4, kdkVolumeBaseNumericalConvert_3)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_GIGABYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite5, kdkVolumeBaseNumericalConvert_4)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_TERABYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite6, kdkVolumeBaseNumericalConvert_5)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_PETABYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite7, kdkVolumeBaseNumericalConvert_6)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_EXABYTE, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite8, kdkVolumeBaseNumericalConvert_7)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_KILO, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite9, kdkVolumeBaseNumericalConvert_8)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_MEGA, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite10, kdkVolumeBaseNumericalConvert_9)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_GIGA, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite11, kdkVolumeBaseNumericalConvert_10)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_TERA, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite12, kdkVolumeBaseNumericalConvert_11)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_PETA, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

TEST(TestSuite13, kdkVolumeBaseNumericalConvert_12)
{
    int ret = 0;

    // 调用数字型数据单位转换接口
    // 此例亦可说明，在低进制不足以向高进制转换时，进制保持不变
    double origin_numberical = 100.24;
    double result_numberical;
    ret = kdkVolumeBaseNumericalConvert(origin_numberical, KDK_EXA, KDK_GIGABYTE, &result_numberical);
    printf("%.2f\n", result_numberical);
}

// 测试 strstrip 函数
TEST(CStringExtensionTest, StrStrip) {
    char str[] = "  Hello World!  ";
    strstrip(str, ' ');
    EXPECT_STREQ(str, "Hello World!");
}

// 测试 strstripspace 函数
TEST(CStringExtensionTest, StrStripSpace) {
    char str[] = "\t  Hello World!\n  ";
    strstripspace(str);
    EXPECT_STREQ(str, "Hello World!");
}

// 测试 strstripblank 函数
TEST(CStringExtensionTest, StrStripBlank) {
    char str[] = "  \tHello World!\t  ";
    strstripblank(str);
    EXPECT_STREQ(str, "Hello World!");
}

// 测试 strskipspace 函数
TEST(CStringExtensionTest, StrSkipSpace) {
    const char* str = "   Hello World!";
    const char* result = strskipspace(str);
    EXPECT_STREQ(result, "Hello World!");
}

// 测试 strskipblank 函数
TEST(CStringExtensionTest, StrSkipBlank) {
    const char* str = "  \tHello World!";
    const char* result = strskipblank(str);
    EXPECT_STREQ(result, "Hello World!");
}

// 测试 str2upper 函数
TEST(CStringExtensionTest, Str2Upper) {
    char str[] = "Hello World!";
    str2upper(str);
    EXPECT_STREQ(str, "HELLO WORLD!");
}

// 测试 str2lower 函数
TEST(CStringExtensionTest, Str2Lower) {
    char str[] = "HELLO WORLD!";
    str2lower(str);
    EXPECT_STREQ(str, "hello world!");
}

// 测试 strfirstof 函数
TEST(CStringExtensionTest, StrFirstOf) {
    const char* str = "Hello World!";
    EXPECT_EQ(strfirstof(str, 'W'), 6);
    EXPECT_EQ(strfirstof(str, 'z'), -1);
}

// 测试 strlastof 函数
TEST(CStringExtensionTest, StrLastOf) {
    const char* str = "Hello World!";
    EXPECT_EQ(strlastof(str, 'o'), 7);
    EXPECT_EQ(strlastof(str, 'z'), -1);
}

// 测试 strstartswith 函数
TEST(CStringExtensionTest, StrStartsWith) {
    const char* str = "Hello World!";
    EXPECT_EQ(strstartswith(str, "Hello"), 0);
    EXPECT_EQ(strstartswith(str, "World"), 1);
}

// 测试 strstartswith_nocase 函数
TEST(CStringExtensionTest, StrStartsWithNoCase) {
    const char* str = "Hello World!";
    EXPECT_EQ(strstartswith_nocase(str, "hello"), 0);
    EXPECT_EQ(strstartswith_nocase(str, "WORLD"), 1);
}

// 测试 strendwith 函数
TEST(CStringExtensionTest, StrEndWith) {
    const char* str = "Hello World!";
    EXPECT_EQ(strendwith(str, "World!"), 0);
    EXPECT_EQ(strendwith(str, "Hello"), 1);
}

// 测试 strcounts 函数
TEST(CStringExtensionTest, StrCounts) {
    const char* str = "Hello World!";
    EXPECT_EQ(strcounts(str, 'l'), 3);
    EXPECT_EQ(strcounts(str, 'z'), 0);
}

// 测试 strsplit 函数
TEST(CStringExtensionTest, StrSplit) {
    char str[] = "Hello,World,Test";
    char** result = strsplit(str, ',');
    EXPECT_STREQ(result[0], "Hello");
    EXPECT_STREQ(result[1], "World");
    EXPECT_STREQ(result[2], "Test");
    free(result);
}

// 测试 strisdigit 函数
TEST(CStringExtensionTest, StrIsDigit) {
    char str1[] = "12345";
    char str2[] = "123a45";
    EXPECT_TRUE(strisdigit(str1));
    EXPECT_FALSE(strisdigit(str2));
}

// 测试 strisspace 函数
TEST(CStringExtensionTest, StrIsSpace) {
    char str1[] = "   ";
    char str2[] = "  a  ";
    EXPECT_TRUE(strisspace(str1));
    EXPECT_FALSE(strisspace(str2));
}

// 测试 striswhitespace 函数
TEST(CStringExtensionTest, StrIsWhitespace) {
    char str1[] = "   ";
    char str2[] = "";
    EXPECT_TRUE(striswhitespace(str1));
    EXPECT_TRUE(striswhitespace(str2));
}

// 测试 strisletter 函数
TEST(CStringExtensionTest, StrIsLetter) {
    char str1[] = "abc123";
    char str2[] = "123";
    EXPECT_TRUE(strisletter(str1));
    EXPECT_FALSE(strisletter(str2));
}

// 测试 strisideograph 函数
TEST(CStringExtensionTest, StrIsIdeograph) {
    char str1[] = "你好";
    char str2[] = "Hello";
    EXPECT_TRUE(strisideograph(str1));
    EXPECT_FALSE(strisideograph(str2));
}

// 测试 strisrtl 函数
TEST(CStringExtensionTest, StrIsRTL) {
    char str1[] = "مرحبا";
    char str2[] = "Hello";
    EXPECT_TRUE(strisrtl(str1));
    EXPECT_FALSE(strisrtl(str2));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}