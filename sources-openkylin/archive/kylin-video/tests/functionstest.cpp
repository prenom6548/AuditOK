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

#include "functionstest.h"
#include "global/functions.h"
#include <QDir>
#include <QFile>
#include <QProcess>
#include <iostream>

FunctionsTest::FunctionsTest(QObject *parent) : QObject(parent)
{
}

void FunctionsTest::testUrl()
{
    std::cout << "=== Testing Url ===" << std::endl;
    
    // 测试相对路径转换为绝对路径
    QString relativePath = "test.txt";
    QString absolutePath = Functions::Url(relativePath);
    QVERIFY(absolutePath.startsWith("file://"));
    QVERIFY(absolutePath.endsWith("test.txt"));
    
    // 测试已经是绝对路径的情况
    QString absoluteUrl = "file:///home/user/test.txt";
    QString result = Functions::Url(absoluteUrl);
    QVERIFY(result == absoluteUrl);
    
    // 测试带有反斜杠的路径
    QString backslashPath = "test\\file.txt";
    QString backslashResult = Functions::Url(backslashPath);
    QVERIFY(backslashResult.startsWith("file://"));
    
    // 测试带路径的情况
    QString path = "/home/user/";
    QString fileName = "test.txt";
    QString pathResult = Functions::Url(fileName, path);
    QVERIFY(pathResult == "file:///home/user/test.txt");
    
    std::cout << "Url test completed." << std::endl;
}

void FunctionsTest::testExecuteCMD()
{
    std::cout << "=== Testing executeCMD ===" << std::endl;
    
    // 测试执行简单命令
    QString result;
    int ret = Functions::executeCMD("echo 'hello world'", result);
    QVERIFY(ret == 0);
    QVERIFY(result.trimmed() == "hello world");
    
    // 测试执行不存在的命令
    QString errorResult;
    int errorRet = Functions::executeCMD("nonexistentcommand12345", errorResult);
    // 即使命令不存在，executeCMD也返回0，但结果应该是空的或错误信息
    QVERIFY(errorRet == 0);
    
    std::cout << "executeCMD test completed." << std::endl;
}

void FunctionsTest::testFileExt()
{
    std::cout << "=== Testing fileExt ===" << std::endl;
    
    // 测试正常的文件路径
    QString filePath1 = "/home/user/test.mp4";
    QString ext1 = Functions::fileExt(filePath1);
    QVERIFY(ext1 == "mp4");
    
    // 测试没有扩展名的文件
    QString filePath2 = "/home/user/test";
    QString ext2 = Functions::fileExt(filePath2);
    QVERIFY(ext2.isEmpty());
    
    // 测试以点结尾的文件
    QString filePath3 = "/home/user/test.";
    QString ext3 = Functions::fileExt(filePath3);
    QVERIFY(ext3.isEmpty());
    
    // 测试多个点的文件名
    QString filePath4 = "/home/user/test.file.mp4";
    QString ext4 = Functions::fileExt(filePath4);
    QVERIFY(ext4 == "mp4");
    
    // 测试只有扩展名的文件
    QString filePath5 = ".mp4";
    QString ext5 = Functions::fileExt(filePath5);
    QVERIFY(ext5 == "mp4");
    
    std::cout << "fileExt test completed." << std::endl;
}

void FunctionsTest::testTimeToStr()
{
    std::cout << "=== Testing timeToStr ===" << std::endl;
    
    // 测试正常的时间转换
    QString time1 = Functions::timeToStr(3661.0); // 1小时1分1秒
    QVERIFY(time1 == "01:01:01");
    
    // 测试带小数的时间转换
    QString time2 = Functions::timeToStr(3661.5, true); // 1小时1分1.5秒
    QVERIFY(time2 == "01:01:01.5");
    
    // 测试小于1秒的时间
    QString time3 = Functions::timeToStr(0.5);
    QVERIFY(time3 == "00:00:00");
    
    // 测试负数时间
    QString time4 = Functions::timeToStr(-10.0);
    QVERIFY(time4 == "--:--:--");
    
    // 测试0时间
    QString time5 = Functions::timeToStr(0.0);
    QVERIFY(time5 == "--:--:--");
    
    // 测试较大的时间
    QString time6 = Functions::timeToStr(7200.0); // 2小时
    QVERIFY(time6 == "02:00:00");
    
    std::cout << "timeToStr test completed." << std::endl;
}

void FunctionsTest::testGetUrlType()
{
    std::cout << "=== Testing getUrlType ===" << std::endl;
    
    // 测试正常的URL
    QString url1 = "http://example.com";
    QString type1 = Functions::getUrlType(url1);
    QVERIFY(type1 == "http");
    
    // 测试没有协议的路径
    QString url2 = "/home/user/test.mp4";
    QString type2 = Functions::getUrlType(url2);
    QVERIFY(type2.isEmpty());
    
    // 测试以冒号开头的路径
    QString url3 = ":/test.mp4";
    QString type3 = Functions::getUrlType(url3);
    QVERIFY(type3 == "");
    
    // 测试空字符串
    QString url4 = "";
    QString type4 = Functions::getUrlType(url4);
    QVERIFY(type4.isEmpty());
    
    std::cout << "getUrlType test completed." << std::endl;
}

void FunctionsTest::testFilePath()
{
    std::cout << "=== Testing filePath ===" << std::endl;
    
    // 测试正常的文件路径
    QString filePath1 = "/home/user/test.mp4";
    QString path1 = Functions::filePath(filePath1);
    QVERIFY(path1 == "/home/user/");
    
    // 测试没有路径的文件名
    QString filePath2 = "test.mp4";
    QString path2 = Functions::filePath(filePath2);
    QVERIFY(path2 == "");
    
    // 测试根目录下的文件
    QString filePath3 = "/test.mp4";
    QString path3 = Functions::filePath(filePath3);
    QVERIFY(path3 == "/");
    
    std::cout << "filePath test completed." << std::endl;
}

void FunctionsTest::testHumanSize()
{
    std::cout << "=== Testing humanSize ===" << std::endl;
    
    // 测试字节
    QString size1 = Functions::humanSize(512);
    QVERIFY(size1 == "512.00 bytes");
    
    // 测试KB
    QString size2 = Functions::humanSize(1024);
    QVERIFY(size2 == "1.00 KB");
    
    // 测试MB
    QString size3 = Functions::humanSize(1024 * 1024);
    QVERIFY(size3 == "1.00 MB");
    
    // 测试GB
    QString size4 = Functions::humanSize(1024LL * 1024 * 1024);
    QVERIFY(size4 == "1.00 GB");
    
    // 测试TB
    QString size5 = Functions::humanSize(1024LL * 1024 * 1024 * 1024);
    QVERIFY(size5 == "1.00 TB");
    
    // 测试0大小
    QString size6 = Functions::humanSize(0);
    QVERIFY(size6 == "0.00 bytes");
    
    std::cout << "humanSize test completed." << std::endl;
}

void FunctionsTest::testIsVirtualSuppotGpu()
{
    std::cout << "=== Testing isVirtualSuppotGpu ===" << std::endl;
    
    // 这个函数依赖系统环境，我们只能测试它能正常调用
    bool result = Functions::isVirtualSuppotGpu();
    // 不管返回什么，只要不崩溃就行
    QVERIFY(true);
    
    std::cout << "isVirtualSuppotGpu test completed." << std::endl;
}

void FunctionsTest::testIsKirinCpu()
{
    std::cout << "=== Testing isKirinCpu ===" << std::endl;
    
    // 这个函数依赖系统环境，我们只能测试它能正常调用
    bool result = Functions::isKirinCpu();
    // 不管返回什么，只要不崩溃就行
    QVERIFY(true);
    
    std::cout << "isKirinCpu test completed." << std::endl;
}

void FunctionsTest::testIsGreatwallDevice()
{
    std::cout << "=== Testing isGreatwallDevice ===" << std::endl;
    
    // 这个函数依赖系统环境，我们只能测试它能正常调用
    bool result = Functions::isGreatwallDevice();
    // 不管返回什么，只要不崩溃就行
    QVERIFY(true);
    
    std::cout << "isGreatwallDevice test completed." << std::endl;
}

void FunctionsTest::testGetVersion()
{
    std::cout << "=== Testing getVersion ===" << std::endl;
    
    // 测试获取版本号
    QString version = Functions::getVersion();
    // 不管返回什么，只要不崩溃就行
    QVERIFY(!version.isNull());

    Functions::processComand("pwd");
    
    std::cout << "getVersion test completed." << std::endl;
}
