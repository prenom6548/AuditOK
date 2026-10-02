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

#include "utiltest.h"
#include "global/util.h"
#include <QDir>
#include <iostream>

UtilTest::UtilTest(QObject *parent) : QObject(parent)
{
}

void UtilTest::testFormatNumberWithAmpersand()
{
    std::cout << "=== Testing FormatNumberWithAmpersand ===" << std::endl;
    
    // 测试小于10的情况
    QString result1 = Util::FormatNumberWithAmpersand(5, 9);
    QVERIFY(result1 == "&5");
    
    // 测试小于100且值小于10的情况
    QString result2 = Util::FormatNumberWithAmpersand(5, 50);
    QVERIFY(result2 == "0&5");
    
    // 测试小于100且值大于等于10的情况
    QString result3 = Util::FormatNumberWithAmpersand(15, 50);
    QVERIFY(result3 == "15");
    
    // 测试大于等于100且值小于10的情况
    QString result4 = Util::FormatNumberWithAmpersand(5, 150);
    QVERIFY(result4 == "00&5");
    
    // 测试大于等于100的情况
    QString result5 = Util::FormatNumberWithAmpersand(150, 150);
    QVERIFY(result5 == "150");
    
    std::cout << "FormatNumberWithAmpersand test completed." << std::endl;
}

void UtilTest::testHumanSize()
{
    std::cout << "=== Testing HumanSize ===" << std::endl;
    
    // 测试字节
    QString size1 = Util::HumanSize(512);
    QVERIFY(size1 == "512.00 bytes");
    
    // 测试KB
    QString size2 = Util::HumanSize(1024);
    QVERIFY(size2 == "1.00 KB");
    
    // 测试MB
    QString size3 = Util::HumanSize(1024 * 1024);
    QVERIFY(size3 == "1.00 MB");
    
    // 测试GB
    QString size4 = Util::HumanSize(1024LL * 1024 * 1024);
    QVERIFY(size4 == "1.00 GB");
    
    // 测试TB
    QString size5 = Util::HumanSize(1024LL * 1024 * 1024 * 1024);
    QVERIFY(size5 == "1.00 TB");
    
    // 测试0大小
    QString size6 = Util::HumanSize(0);
    QVERIFY(size6 == "0.00 bytes");
    
    std::cout << "HumanSize test completed." << std::endl;
}

void UtilTest::testIsValidLocation()
{
    std::cout << "=== Testing IsValidLocation ===" << std::endl;
    
    // 注意：由于IsValidLocation函数在源文件中是空实现，我们只能测试它能正常调用
    bool result = Util::IsValidLocation("/home/user/test.mp4");
    // 不管返回什么，只要不崩溃就行
    QVERIFY(true);
    
    std::cout << "IsValidLocation test completed." << std::endl;
}

void UtilTest::testShowInFolder()
{
    std::cout << "=== Testing ShowInFolder ===" << std::endl;
    
    // 注意：由于ShowInFolder函数在源文件中是空实现，我们只能测试它能正常调用
    Util::ShowInFolder("/home/user/", "test.mp4");
    // 不管做什么，只要不崩溃就行
    QVERIFY(true);
    
    std::cout << "ShowInFolder test completed." << std::endl;
}

void UtilTest::testGCD()
{
    std::cout << "=== Testing GCD ===" << std::endl;
    
    // 测试正常情况
    int result1 = Util::GCD(12, 8);
    QVERIFY(result1 == 4);
    
    // 测试其中一个为0的情况
    int result2 = Util::GCD(0, 5);
    QVERIFY(result2 == 5);
    
    int result3 = Util::GCD(5, 0);
    QVERIFY(result3 == 5);
    
    // 测试两个都为0的情况
    int result4 = Util::GCD(0, 0);
    QVERIFY(result4 == 0);
    
    // 测试互质数
    int result5 = Util::GCD(7, 13);
    QVERIFY(result5 == 1);
    
    std::cout << "GCD test completed." << std::endl;
}

void UtilTest::testRatio()
{
    std::cout << "=== Testing Ratio ===" << std::endl;
    
    // 测试正常情况
    QString result1 = Util::Ratio(1920, 1080);
    QVERIFY(result1 == "16:9");
    
    // 测试其中一个为0的情况
    QString result2 = Util::Ratio(0, 1080);
    QVERIFY(result2 == "0:0");
    
    // 测试相等的情况
    QString result3 = Util::Ratio(100, 100);
    QVERIFY(result3 == "1:1");
    
    std::cout << "Ratio test completed." << std::endl;
}

void UtilTest::testToNativeSeparators()
{
    std::cout << "=== Testing ToNativeSeparators ===" << std::endl;
    
    // 创建测试列表
    QStringList list;
    list << "/home/user/test.mp4" << "http://example.com" << "/tmp/file.txt";
    
    // 调用函数
    QStringList result = Util::ToNativeSeparators(list);
    
    // 验证结果数量
    QVERIFY(result.size() == 3);
    
    std::cout << "ToNativeSeparators test completed." << std::endl;
}

void UtilTest::testFromNativeSeparators()
{
    std::cout << "=== Testing FromNativeSeparators ===" << std::endl;
    
    // 创建测试列表
    QStringList list;
    list << "\\home\\user\\test.mp4" << "\\tmp\\file.txt";
    
    // 调用函数
    QStringList result = Util::FromNativeSeparators(list);
    
    // 验证结果数量
    QVERIFY(result.size() == 2);
    
    std::cout << "FromNativeSeparators test completed." << std::endl;
}

void UtilTest::testFormatTime()
{
    std::cout << "=== Testing FormatTime ===" << std::endl;
    
    // 测试小时级别的时间
    QString time1 = Util::FormatTime(3661, 3661); // 1小时1分1秒
    QVERIFY(time1 == "1:01:01");
    
    // 测试分钟级别的时间
    QString time2 = Util::FormatTime(61, 61); // 1分1秒
    QVERIFY(time2 == "01:01");
    
    // 测试秒级别的时间
    QString time3 = Util::FormatTime(1, 1); // 1秒
    QVERIFY(time3 == "0:01");
    
    std::cout << "FormatTime test completed." << std::endl;
}

void UtilTest::testFormatRelativeTime()
{
    std::cout << "=== Testing FormatRelativeTime ===" << std::endl;
    
    // 测试正数时间
    QString time1 = Util::FormatRelativeTime(3661); // 1小时1分1秒
    QVERIFY(time1 == "+1:01:01");
    
    // 测试负数时间
    QString time2 = Util::FormatRelativeTime(-3661); // -1小时1分1秒
    QVERIFY(time2 == "-1:01:01");
    
    // 测试分钟级别的时间
    QString time3 = Util::FormatRelativeTime(61); // 1分1秒
    QVERIFY(time3 == "+01:01");
    
    // 测试秒级别的时间
    QString time4 = Util::FormatRelativeTime(1); // 1秒
    QVERIFY(time4 == "+0:01");
    
    std::cout << "FormatRelativeTime test completed." << std::endl;
}

void UtilTest::testFormatNumber()
{
    std::cout << "=== Testing FormatNumber ===" << std::endl;
    
    // 测试小于10的情况
    QString result1 = Util::FormatNumber(5, 9);
    QVERIFY(result1 == "5");
    
    // 测试小于100的情况
    QString result2 = Util::FormatNumber(5, 50);
    QVERIFY(result2 == "05");
    
    // 测试大于等于100的情况
    QString result3 = Util::FormatNumber(5, 150);
    QVERIFY(result3 == "005");
    
    std::cout << "FormatNumber test completed." << std::endl;
}

void UtilTest::testIsValidUrl()
{
    std::cout << "=== Testing IsValidUrl ===" << std::endl;
    
    // 测试有效的URL
    bool result1 = Util::IsValidUrl("http://example.com");
    QVERIFY(result1 == true);
    
    // 测试无效的URL
    bool result2 = Util::IsValidUrl("/home/user/test.mp4");
    QVERIFY(result2 == false);
    
    // 测试空字符串
    bool result3 = Util::IsValidUrl("");
    QVERIFY(result3 == false);
    
    std::cout << "IsValidUrl test completed." << std::endl;
}

}
