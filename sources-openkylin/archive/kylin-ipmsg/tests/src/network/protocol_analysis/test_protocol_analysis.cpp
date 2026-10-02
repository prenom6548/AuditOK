/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "test_protocol_analysis.h"
#include <QMap>
#include <QByteArray>

// 测试序列化功能
TEST_F(TestProtocolAnalysis, EncapsulationTest) {
    QMap<QByteArray, QByteArray> testMap;
    testMap.insert("uuid", "123456");
    testMap.insert("type", "msg");
    testMap.insert("content", "hello world");

    QByteArray result = parser->encapsulationHeadBody(testMap);
    
    // 验证结果中包含关键字段
    EXPECT_TRUE(result.contains("uuid:123456"));
    EXPECT_TRUE(result.contains("type:msg"));
    EXPECT_TRUE(result.contains("content:hello world"));
    EXPECT_TRUE(result.contains("\r\n"));
}

// 测试序列化功能 (空 map)
TEST_F(TestProtocolAnalysis, EncapsulationEmptyTest) {
    QMap<QByteArray, QByteArray> testMap;
    QByteArray result = parser->encapsulationHeadBody(testMap);
    EXPECT_TRUE(result.isEmpty());
}

// 测试序列化功能 (包含 body)
TEST_F(TestProtocolAnalysis, EncapsulationWithBodyTest) {
    QMap<QByteArray, QByteArray> testMap;
    testMap.insert("uuid", "123");
    testMap.insert("body", "this is body content");
    
    QByteArray result = parser->encapsulationHeadBody(testMap);
    
    EXPECT_TRUE(result.contains("uuid:123\r\n"));
    EXPECT_TRUE(result.contains("\r\n\r\nthis is body content"));
    // 验证补齐长度 (PACKAGE_SIZE 为 1024 * 100)
    EXPECT_EQ(result.size(), 1024 * 100);
}

// 测试反序列化功能 (正常路径，包含 body)
TEST_F(TestProtocolAnalysis, AnalysisNormalWithBodyTest) {
    QByteArray input = "uuid:654321\r\nbody_size:5\r\n\r\nhello";
    // 确保输入长度足够，模拟实际网络包或至少满足解析逻辑
    QMap<QByteArray, QByteArray> result = parser->analysisHeadBody(input);

    EXPECT_EQ(result.value("uuid"), "654321");
    EXPECT_EQ(result.value("body_size"), "5");
    EXPECT_EQ(result.value("body"), "hello");
}

// 测试反序列化功能 (异常路径：空输入)
TEST_F(TestProtocolAnalysis, AnalysisEmptyTest) {
    QByteArray input = "";
    QMap<QByteArray, QByteArray> result = parser->analysisHeadBody(input);
    EXPECT_TRUE(result.isEmpty());
}

// 测试反序列化功能 (异常路径：格式错误，缺少 \r\n\r\n)
TEST_F(TestProtocolAnalysis, AnalysisMalformedTest) {
    QByteArray input = "key:value\r\n";
    QMap<QByteArray, QByteArray> result = parser->analysisHeadBody(input);
    
    // 由于缺少 \r\n\r\n，函数应该返回空 map
    EXPECT_TRUE(result.isEmpty());
}

// 测试反序列化功能 (复杂输入)
TEST_F(TestProtocolAnalysis, AnalysisComplexTest) {
    QByteArray input = "key1:val1\r\nkey2:val2:with:extra:colons\r\nbody_size:0\r\n\r\n";
    QMap<QByteArray, QByteArray> result = parser->analysisHeadBody(input);

    EXPECT_EQ(result.value("key1"), "val1");
    // 验证多冒号处理逻辑（通常以第一个冒号分隔）
    EXPECT_EQ(result.value("key2"), "val2:with:extra:colons");
    EXPECT_EQ(result.value("body"), "");
}

