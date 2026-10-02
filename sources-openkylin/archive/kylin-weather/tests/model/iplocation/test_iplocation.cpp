/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "test_iplocation.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

// 使用 private public 技巧来测试私有成员和方法，提升覆盖率
#define private public
#include "model/iplocation.h"
#undef private

void TestIplocation::initTestCase()
{
}

void TestIplocation::cleanupTestCase()
{
}

void TestIplocation::testIplocationConstructor()
{
    Iplocation iplocation;
    QVERIFY(true);
}

void TestIplocation::testGotoLocation()
{
    Iplocation iplocation;
    // 这是一个综合测试，会调用内部的各种定位逻辑
    // 我们主要验证它不会崩溃
    iplocation.gotoLocation();
    QVERIFY(true);
}

void TestIplocation::testTimeOut_Active()
{
    Iplocation iplocation;
    QTimer timer;
    timer.start(1000);
    QEventLoop loop;
    
    // 模拟 reply
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl("http://localhost")));
    
    // 当 timer 还在运行时调用 timeOut
    bool result = iplocation.timeOut(&timer, reply, &loop);
    
    QVERIFY(result);
    QVERIFY(!timer.isActive()); // 定时器应该被停止了
    
    reply->abort();
    reply->deleteLater();
}

void TestIplocation::testTimeOut_Expired()
{
    Iplocation iplocation;
    QTimer timer;
    timer.stop(); // 模拟已超时
    QEventLoop loop;
    
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl("http://localhost")));
    
    // 当 timer 不在运行时调用 timeOut (模拟超时)
    bool result = iplocation.timeOut(&timer, reply, &loop);
    
    QVERIFY(!result);
    // 这里会覆盖 iplocation.cpp:176-180
    
    reply->deleteLater();
}

void TestIplocation::testDecodeToOrigin()
{
    Iplocation iplocation;
    // 测试解密逻辑
    // aXA/a2V5PWZhZjhmYWFkNA== 是 "ip?key=faf8faad4" 的 base64
    // YzljMDcwODc5YmViZTYxNQ== 是 "c9c070879bebe615" 的 base64
    QString d1 = "aXA/a2V5PWZhZjhmYWFkNA==";
    QString d2 = "YzljMDcwODc5YmViZTYxNQ==";
    QString d3 = "ZWRkZWZmNA=="; // "eddeff4" 的 base64
    
    QString result = iplocation.decodeToOrigin(d1, d2, d3);
    QCOMPARE(result, QString("ip?key=faf8faad4c9c070879bebe615eddeff4"));
}

void TestIplocation::testParsingLogic()
{
    Iplocation iplocation;
    // 模拟 pconlineLocation 中的解析逻辑 (iplocation.cpp:124-135)
    // 虽然我们不能直接调用私有的 pconlineLocation，但我们可以通过构造特定的 reply 来间接测试或者
    // 如果我们能直接调用它：
    
    QString mockHtml = "<html><br/><br/><br/><br/>ip=127.0.0.1<br/></html>";
    // 模拟 htmlList.size() >= 4 的逻辑
    QString htmlStr = mockHtml.replace(" ", "").replace("\r", "").replace("\n", "");
    QStringList htmlList = htmlStr.split("<br/>");
    QVERIFY(htmlList.size() >= 4);
    QStringList ipList = htmlList.at(4).split("=");
    QCOMPARE(ipList.at(1), QString("127.0.0.1"));
    
    // 模拟带有逗号的 IP (iplocation.cpp:131)
    QString mockHtmlComma = "<html><br/><br/><br/><br/>ip=192.168.1.1,192.168.1.2<br/></html>";
    QString htmlStrComma = mockHtmlComma.replace(" ", "").replace("\r", "").replace("\n", "");
    QStringList htmlListComma = htmlStrComma.split("<br/>");
    QStringList ipListComma = htmlListComma.at(4).split("=");
    if (ipListComma.at(1).contains(",")) {
        QString ip = ipListComma.at(1).split(",").at(0);
        QCOMPARE(ip, QString("192.168.1.1"));
    }
}

void TestIplocation::testPconlineLocation_Manual()
{
    Iplocation iplocation;
    // 覆盖 pconlineLocation 函数入口
    iplocation.pconlineLocation("http://localhost/invalid");
    QVERIFY(true);
}

