/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QtTest>

#include "../daemon/src/resource.h"

class TestResource : public QObject
{
    Q_OBJECT

public:
    TestResource();
    ~TestResource();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试资源枚举和字符串相互转换
    void testResourceEnumToString();
    void testStringToResourceEnum();

    // 测试紧急程度枚举和整数相互转换
    void testResourceUrgencyEnumToInt();
    void testIntToResourceUrgencyEnum();

    // 测试紧急程度比较运算符
    void testResourceUrgencyComparison();

    // 测试边界情况
    void testInvalidResourceEnumToString();
    void testInvalidStringToResourceEnum();
    void testInvalidIntToResourceUrgencyEnum();
    void testInvalidResourceUrgencyEnumToInt();

private:
    // 辅助函数和数据
};

TestResource::TestResource()
{
}

TestResource::~TestResource()
{
}

void TestResource::initTestCase()
{
}

void TestResource::cleanupTestCase()
{
}

void TestResource::init()
{
}

void TestResource::cleanup()
{
}

void TestResource::testResourceEnumToString()
{
    // 测试所有有效的资源枚举转字符串
    QCOMPARE(resource::resourceEnumToString(resource::Resource::CPU), std::string("CPU"));
    QCOMPARE(resource::resourceEnumToString(resource::Resource::IO), std::string("IO"));
    QCOMPARE(resource::resourceEnumToString(resource::Resource::Memory), std::string("Memory"));
}

void TestResource::testStringToResourceEnum()
{
    // 测试所有有效的字符串转资源枚举
    QCOMPARE(resource::stringToResourceEnum("CPU"), resource::Resource::CPU);
    QCOMPARE(resource::stringToResourceEnum("IO"), resource::Resource::IO);
    QCOMPARE(resource::stringToResourceEnum("Memory"), resource::Resource::Memory);
}

void TestResource::testResourceUrgencyEnumToInt()
{
    // 测试所有有效的紧急程度枚举转整数
    QCOMPARE(resource::resourceUrgencyEnumToInt(resource::ResourceUrgency::Low), 0);
    QCOMPARE(resource::resourceUrgencyEnumToInt(resource::ResourceUrgency::Medium), 1);
    QCOMPARE(resource::resourceUrgencyEnumToInt(resource::ResourceUrgency::High), 2);
}

void TestResource::testIntToResourceUrgencyEnum()
{
    // 测试所有有效的整数转紧急程度枚举
    QCOMPARE(resource::intToResourceUrgencyEnum(0), resource::ResourceUrgency::Low);
    QCOMPARE(resource::intToResourceUrgencyEnum(1), resource::ResourceUrgency::Medium);
    QCOMPARE(resource::intToResourceUrgencyEnum(2), resource::ResourceUrgency::High);
}

void TestResource::testResourceUrgencyComparison()
{
    // 测试紧急程度比较运算符
    QVERIFY(resource::ResourceUrgency::Low < resource::ResourceUrgency::Medium);
    QVERIFY(resource::ResourceUrgency::Medium < resource::ResourceUrgency::High);
    QVERIFY(!(resource::ResourceUrgency::High < resource::ResourceUrgency::Medium));
    QVERIFY(!(resource::ResourceUrgency::Medium < resource::ResourceUrgency::Low));
}

void TestResource::testInvalidResourceEnumToString()
{
    // 测试无效的资源枚举转字符串
    QCOMPARE(resource::resourceEnumToString(resource::Resource::Unknown), std::string(""));
    // 测试超出枚举范围的值
    QCOMPARE(resource::resourceEnumToString(static_cast<resource::Resource>(99)), std::string(""));
}

void TestResource::testInvalidStringToResourceEnum()
{
    // 测试无效的字符串转资源枚举
    QCOMPARE(resource::stringToResourceEnum("Invalid"), resource::Resource::Unknown);
    QCOMPARE(resource::stringToResourceEnum(""), resource::Resource::Unknown);
}

void TestResource::testInvalidIntToResourceUrgencyEnum()
{
    // 测试无效的整数转紧急程度枚举
    QCOMPARE(resource::intToResourceUrgencyEnum(-1), resource::ResourceUrgency::Unknown);
    QCOMPARE(resource::intToResourceUrgencyEnum(99), resource::ResourceUrgency::Unknown);
}

void TestResource::testInvalidResourceUrgencyEnumToInt()
{
    // 测试无效的紧急程度枚举转整数
    QCOMPARE(resource::resourceUrgencyEnumToInt(resource::ResourceUrgency::Unknown), -1);
    // 测试超出枚举范围的值
    QCOMPARE(resource::resourceUrgencyEnumToInt(static_cast<resource::ResourceUrgency>(99)), -1);
}

QTEST_MAIN(TestResource)
#include "tst_resource.moc"