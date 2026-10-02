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
#include "test_errorreact.h"
#include <QSignalSpy>
#include <QtTest>

TestErrorReact::TestErrorReact()
{
}

void TestErrorReact::initTestCase()
{
    m_errorReact = ErrorReact::getInstance();
}

void TestErrorReact::cleanupTestCase()
{
    // Cleanup if needed
}

void TestErrorReact::testGetInstance()
{
    ErrorReact* instance1 = ErrorReact::getInstance();
    ErrorReact* instance2 = ErrorReact::getInstance();
    
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QCOMPARE(instance1, instance2); // Should be the same instance
}

void TestErrorReact::testNetworkFeedBack_data()
{
    QTest::addColumn<bool>("isOnline");
    QTest::addColumn<QString>("errorString");
    QTest::addColumn<QString>("expectedMessage");

    QTest::newRow("无网络") << false << QString("无网络") << QString("No internet connection. Please check your connection and try again.");
    QTest::newRow("其他网络错误") << false << QString("连接超时") << QString("Network connection error. Please check your network or the website address and try again.");
}

void TestErrorReact::testNetworkFeedBack()
{
    QFETCH(bool, isOnline);
    QFETCH(QString, errorString);
    QFETCH(QString, expectedMessage);

    QSignalSpy spy(m_errorReact, &ErrorReact::netUiShow);
    m_errorReact->networkFeedBack(isOnline, errorString);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), expectedMessage);
}

void TestErrorReact::testServerFeedBack_data()
{
    QTest::addColumn<bool>("isGetData");
    QTest::addColumn<QString>("stateCode");
    QTest::addColumn<QString>("expectedMessage");

    QTest::newRow("服务器错误1") << false << QString("500") << QString("Abnormal access address!");
    QTest::newRow("服务器错误2") << false << QString("404") << QString("Abnormal access address!");
}

void TestErrorReact::testServerFeedBack()
{
    QFETCH(bool, isGetData);
    QFETCH(QString, stateCode);
    QFETCH(QString, expectedMessage);

    QSignalSpy spy(m_errorReact, &ErrorReact::serverUiShow);
    m_errorReact->serverFeedBack(isGetData, stateCode);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), expectedMessage);
}

void TestErrorReact::testDataFeedBack_data()
{
    QTest::addColumn<bool>("isDataError");
    QTest::addColumn<QString>("dataError");
    QTest::addColumn<QString>("expectedMessage");

    QTest::newRow("数据解析错误1") << true << QString("JSON解析失败") << QString("Data parsing error!");
    QTest::newRow("数据解析错误2") << true << QString("XML解析失败") << QString("Data parsing error!");
}

void TestErrorReact::testDataFeedBack()
{
    QFETCH(bool, isDataError);
    QFETCH(QString, dataError);
    QFETCH(QString, expectedMessage);

    QSignalSpy spy(m_errorReact, &ErrorReact::dataUiShow);
    m_errorReact->dataFeedBack(isDataError, dataError);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), expectedMessage);
}

void TestErrorReact::testNetworkTimeout_data()
{
    QTest::addColumn<QString>("timeout");
    QTest::addColumn<QString>("expectedMessage");

    QTest::newRow("主界面超时") << QString("true") << QString("Network Timeout!");
    QTest::newRow("搜索超时") << QString("false") << QString("Network Timeout!");
}

void TestErrorReact::testNetworkTimeout()
{
    QFETCH(QString, timeout);
    QFETCH(QString, expectedMessage);

    QSignalSpy spy(m_errorReact, &ErrorReact::netTimeout);
    m_errorReact->networkTimeout(timeout);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), expectedMessage);
}

void TestErrorReact::testGeoFail_data()
{
    QTest::addColumn<QString>("geoError");
    QTest::addColumn<QString>("expectedMessage");

    QTest::newRow("定位失败") << QString("GPS unavailable") << QString("Seek failed!");
    QTest::newRow("定位超时") << QString("Timeout") << QString("Seek failed!");
}

void TestErrorReact::testGeoFail()
{
    QFETCH(QString, geoError);
    QFETCH(QString, expectedMessage);

    QSignalSpy spy(m_errorReact, &ErrorReact::geoUiShow);
    m_errorReact->geoFail(geoError);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), expectedMessage);
}

void TestErrorReact::testCollectAddError()
{
    QString testMessage = "添加城市失败";
    // There's no signal emitted for collectAddError, so we just verify it doesn't crash
    m_errorReact->collectAddError(testMessage);
    
    // There's no signal emitted for collectAddError, so we just verify it doesn't crash
    QVERIFY(true);
}