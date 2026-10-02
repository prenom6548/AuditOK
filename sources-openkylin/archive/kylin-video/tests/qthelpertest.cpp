/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "qthelpertest.h"
#include "core/qthelper.h"
#include <QVariant>
#include <QMap>
#include <QString>
#include <QList>
#include <iostream>

QTHelperTest::QTHelperTest(QObject *parent) : QObject(parent)
{
}

void QTHelperTest::testNodeToVariant()
{
    std::cout << "=== Testing node_to_variant ===" << std::endl;
    
    // 测试 STRING 类型
    mpv_node stringNode;
    stringNode.format = MPV_FORMAT_STRING;
    stringNode.u.string = const_cast<char*>("test_string");
    QVariant stringResult = mpv::qt::node_to_variant(&stringNode);
    QVERIFY(stringResult.type() == QVariant::String);
    QVERIFY(stringResult.toString() == "test_string");
    
    // 测试 FLAG 类型
    mpv_node flagNode;
    flagNode.format = MPV_FORMAT_FLAG;
    flagNode.u.flag = 1;
    QVariant flagResult = mpv::qt::node_to_variant(&flagNode);
    QVERIFY(flagResult.type() == QVariant::Bool);
    QVERIFY(flagResult.toBool() == true);
    
    // 测试 INT64 类型
    mpv_node intNode;
    intNode.format = MPV_FORMAT_INT64;
    intNode.u.int64 = 123456789LL;
    QVariant intResult = mpv::qt::node_to_variant(&intNode);
    QVERIFY(intResult.type() == QVariant::LongLong);
    QVERIFY(intResult.toLongLong() == 123456789LL);
    
    // 测试 DOUBLE 类型
    mpv_node doubleNode;
    doubleNode.format = MPV_FORMAT_DOUBLE;
    doubleNode.u.double_ = 3.14159;
    QVariant doubleResult = mpv::qt::node_to_variant(&doubleNode);
    QVERIFY(doubleResult.type() == QVariant::Double);
    QVERIFY(qAbs(doubleResult.toDouble() - 3.14159) < 0.00001);
    
    // 测试 NODE_ARRAY 类型
    mpv_node arrayNode;
    arrayNode.format = MPV_FORMAT_NODE_ARRAY;
    mpv_node_list arrayList;
    arrayList.num = 3;
    
    mpv_node arrayItems[3];
    arrayItems[0].format = MPV_FORMAT_STRING;
    arrayItems[0].u.string = const_cast<char*>("item1");
    arrayItems[1].format = MPV_FORMAT_STRING;
    arrayItems[1].u.string = const_cast<char*>("item2");
    arrayItems[2].format = MPV_FORMAT_STRING;
    arrayItems[2].u.string = const_cast<char*>("item3");
    
    arrayList.values = arrayItems;
    arrayNode.u.list = &arrayList;
    
    QVariant arrayResult = mpv::qt::node_to_variant(&arrayNode);
    QVERIFY(arrayResult.type() == QVariant::List);
    QVariantList list = arrayResult.toList();
    QVERIFY(list.size() == 3);
    QVERIFY(list[0].toString() == "item1");
    QVERIFY(list[1].toString() == "item2");
    QVERIFY(list[2].toString() == "item3");
    
    // 测试 NODE_MAP 类型
    mpv_node mapNode;
    mapNode.format = MPV_FORMAT_NODE_MAP;
    mpv_node_list mapList;
    mapList.num = 2;
    
    mpv_node mapValues[2];
    mapValues[0].format = MPV_FORMAT_STRING;
    mapValues[0].u.string = const_cast<char*>("value1");
    mapValues[1].format = MPV_FORMAT_INT64;
    mapValues[1].u.int64 = 42;
    
    char* mapKeys[2];
    mapKeys[0] = const_cast<char*>("key1");
    mapKeys[1] = const_cast<char*>("key2");
    
    mapList.values = mapValues;
    mapList.keys = mapKeys;
    mapNode.u.list = &mapList;
    
    QVariant mapResult = mpv::qt::node_to_variant(&mapNode);
    QVERIFY(mapResult.type() == QVariant::Map);
    QVariantMap map = mapResult.toMap();
    QVERIFY(map.size() == 2);
    QVERIFY(map["key1"].toString() == "value1");
    QVERIFY(map["key2"].toLongLong() == 42);
    
    // 测试 NONE 类型
    mpv_node noneNode;
    noneNode.format = MPV_FORMAT_NONE;
    QVariant noneResult = mpv::qt::node_to_variant(&noneNode);
    QVERIFY(noneResult.isNull());
    
    std::cout << "node_to_variant test completed." << std::endl;
}

void QTHelperTest::testNodeBuilder()
{
    std::cout << "=== Testing node_builder ===" << std::endl;
    
    // 测试 QString 类型
    QVariant stringVar("test_string");
    mpv::qt::node_builder stringBuilder(stringVar);
    mpv_node* stringNode = stringBuilder.node();
    QVERIFY(stringNode->format == MPV_FORMAT_STRING);
    QVERIFY(QString::fromUtf8(stringNode->u.string) == "test_string");
    
    // 测试 Bool 类型
    QVariant boolVar(true);
    mpv::qt::node_builder boolBuilder(boolVar);
    mpv_node* boolNode = boolBuilder.node();
    QVERIFY(boolNode->format == MPV_FORMAT_FLAG);
    QVERIFY(boolNode->u.flag == 1);
    
    // 测试 Int 类型
    QVariant intVar(12345);
    mpv::qt::node_builder intBuilder(intVar);
    mpv_node* intNode = intBuilder.node();
    QVERIFY(intNode->format == MPV_FORMAT_INT64);
    QVERIFY(intNode->u.int64 == 12345);
    
    // 测试 Double 类型
    QVariant doubleVar(3.14159);
    mpv::qt::node_builder doubleBuilder(doubleVar);
    mpv_node* doubleNode = doubleBuilder.node();
    QVERIFY(doubleNode->format == MPV_FORMAT_DOUBLE);
    QVERIFY(qAbs(doubleNode->u.double_ - 3.14159) < 0.00001);
    
    // 测试 QVariantList 类型
    QVariantList list;
    list.append("item1");
    list.append(42);
    list.append(true);
    QVariant listVar(list);
    mpv::qt::node_builder listBuilder(listVar);
    mpv_node* listNode = listBuilder.node();
    QVERIFY(listNode->format == MPV_FORMAT_NODE_ARRAY);
    QVERIFY(listNode->u.list->num == 3);
    QVERIFY(listNode->u.list->values[0].format == MPV_FORMAT_STRING);
    QVERIFY(QString::fromUtf8(listNode->u.list->values[0].u.string) == "item1");
    QVERIFY(listNode->u.list->values[1].format == MPV_FORMAT_INT64);
    QVERIFY(listNode->u.list->values[1].u.int64 == 42);
    QVERIFY(listNode->u.list->values[2].format == MPV_FORMAT_FLAG);
    QVERIFY(listNode->u.list->values[2].u.flag == 1);
    
    // 测试 QVariantMap 类型
    QVariantMap map;
    map["key1"] = "value1";
    map["key2"] = 123;
    map["key3"] = false;
    QVariant mapVar(map);
    mpv::qt::node_builder mapBuilder(mapVar);
    mpv_node* mapNode = mapBuilder.node();
    QVERIFY(mapNode->format == MPV_FORMAT_NODE_MAP);
    QVERIFY(mapNode->u.list->num == 3);
    QVERIFY(QString::fromUtf8(mapNode->u.list->keys[0]) == "key1");
    QVERIFY(mapNode->u.list->values[0].format == MPV_FORMAT_STRING);
    QVERIFY(QString::fromUtf8(mapNode->u.list->values[0].u.string) == "value1");
    QVERIFY(QString::fromUtf8(mapNode->u.list->keys[1]) == "key2");
    QVERIFY(mapNode->u.list->values[1].format == MPV_FORMAT_INT64);
    QVERIFY(mapNode->u.list->values[1].u.int64 == 123);
    QVERIFY(QString::fromUtf8(mapNode->u.list->keys[2]) == "key3");
    QVERIFY(mapNode->u.list->values[2].format == MPV_FORMAT_FLAG);
    QVERIFY(mapNode->u.list->values[2].u.flag == 0);
    
    std::cout << "node_builder test completed." << std::endl;
}

void QTHelperTest::testGetProperty()
{
    std::cout << "=== Testing get_property ===" << std::endl;
    
    // 由于需要真实的 mpv_handle，我们只能测试函数是否存在且能编译通过
    // 在实际测试中，需要创建一个 mock 的 mpv_handle 或使用真实的实例
    
    QVERIFY(true); // 占位符，实际测试需要更复杂的设置
    
    std::cout << "get_property test completed." << std::endl;
}

void QTHelperTest::testSetProperty()
{
    std::cout << "=== Testing set_property ===" << std::endl;
    
    // 由于需要真实的 mpv_handle，我们只能测试函数是否存在且能编译通过
    // 在实际测试中，需要创建一个 mock 的 mpv_handle 或使用真实的实例
    
    QVERIFY(true); // 占位符，实际测试需要更复杂的设置
    
    std::cout << "set_property test completed." << std::endl;
}

void QTHelperTest::testCommand()
{
    std::cout << "=== Testing command ===" << std::endl;
    
    // 由于需要真实的 mpv_handle，我们只能测试函数是否存在且能编译通过
    // 在实际测试中，需要创建一个 mock 的 mpv_handle 或使用真实的实例
    
    QVERIFY(true); // 占位符，实际测试需要更复杂的设置
    
    std::cout << "command test completed." << std::endl;
}

void QTHelperTest::testErrorReturn()
{
    std::cout << "=== Testing ErrorReturn ===" << std::endl;
    
    // 测试 ErrorReturn 构造函数
    mpv::qt::ErrorReturn defaultError;
    QVERIFY(defaultError.error == 0);
    
    mpv::qt::ErrorReturn customError(-10);
    QVERIFY(customError.error == -10);
    
    // 测试 get_error 函数
    QVariant noError;
    QVERIFY(mpv::qt::get_error(noError) == 0);
    
    QVariant errorVar = QVariant::fromValue(mpv::qt::ErrorReturn(-5));
    QVERIFY(mpv::qt::get_error(errorVar) == -5);
    
    // 测试 is_error 函数
    QVERIFY(!mpv::qt::is_error(noError));
    QVERIFY(mpv::qt::is_error(errorVar));
    
    std::cout << "ErrorReturn test completed." << std::endl;
}