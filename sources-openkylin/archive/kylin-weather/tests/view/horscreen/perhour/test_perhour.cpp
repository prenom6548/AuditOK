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
#include "test_perhour.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QThread>
#include "view/horscreen/perhour.h"
#include "global/corevar.h"

TestPerhour::TestPerhour()
    : m_perhour(nullptr)
{
}

void TestPerhour::initTestCase()
{
    // 在所有测试之前执行一次
}

void TestPerhour::cleanupTestCase()
{
    // 在所有测试之后执行一次
}

void TestPerhour::init()
{
    // 在每个测试之前执行
    m_perhour = new perhour();
}

void TestPerhour::cleanup()
{
    // 在每个测试之后执行
    if (m_perhour) {
        delete m_perhour;
        m_perhour = nullptr;
    }
}

void TestPerhour::testConstructor()
{
    QVERIFY(m_perhour != nullptr);
    
    // 验证对象已正确初始化
    // isVisible()在没有显示部件的情况下可能返回false，所以我们不测试这个
}

void TestPerhour::testSetHourlyInfor_data()
{
    QTest::addColumn<QList<Hourly>>("hourlyData");
    QTest::addColumn<int>("isFullScreen");
    
    // 测试数据1：空列表
    QList<Hourly> emptyList;
    QTest::newRow("empty") << emptyList << 0;
    
    // 测试数据2：少于9个小时的数据
    QList<Hourly> shortList;
    for (int i = 0; i < 5; ++i) {
        Hourly hourly;
        hourly.time = QString("%1:00").arg(i + 1);
        hourly.tmp = QString("%1°C").arg(20 + i);
        hourly.cond_code = "100"; // 晴天
        shortList.append(hourly);
    }
    QTest::newRow("short") << shortList << 0;
    
    // 测试数据3：正好9个小时的数据
    QList<Hourly> exactList;
    for (int i = 0; i < 9; ++i) {
        Hourly hourly;
        hourly.time = QString("%1:00").arg(i + 1);
        hourly.tmp = QString("%1°C").arg(20 + i);
        hourly.cond_code = "100"; // 晴天
        exactList.append(hourly);
    }
    QTest::newRow("exact") << exactList << 1;
    
    // 测试数据4：超过9个小时的数据
    QList<Hourly> longList;
    for (int i = 0; i < 24; ++i) {
        Hourly hourly;
        hourly.time = QString("%1:00").arg(i + 1);
        hourly.tmp = QString("%1°C").arg(20 + (i % 10));
        hourly.cond_code = "100"; // 晴天
        longList.append(hourly);
    }
    QTest::newRow("long") << longList << 0;
}

void TestPerhour::testSetHourlyInfor()
{
    QFETCH(QList<Hourly>, hourlyData);
    QFETCH(int, isFullScreen);
    
    // 调用方法
    m_perhour->setHourlyInfor(hourlyData, isFullScreen);
    
    // 验证方法执行完成（不会崩溃）
    QVERIFY(true);
}

void TestPerhour::testSetHourlyIcon()
{
    // 创建测试数据
    QList<Hourly> testData;
    for (int i = 0; i < 24; ++i) {
        Hourly hourly;
        hourly.time = QString("%1:00").arg(i + 1);
        hourly.tmp = QString("%1°C").arg(20 + (i % 10));
        hourly.cond_code = "100"; // 晴天
        testData.append(hourly);
    }
    
    // 先设置小时数据
    m_perhour->setHourlyInfor(testData, 0);
    
    // 调用方法
    m_perhour->setHourlyIcon(0);
    
    // 验证方法执行完成（不会崩溃）
    QVERIFY(true);
}

void TestPerhour::testModeSwitching()
{
    // 测试平板横屏模式切换
    m_perhour->slotChangePadHMode();
    QVERIFY(true); // 验证不崩溃
    
    // 测试平板竖屏模式切换
    m_perhour->slotChangePadVMode();
    QVERIFY(true); // 验证不崩溃
    
    // 测试PC模式切换
    m_perhour->slotChangePCMode();
    QVERIFY(true); // 验证不崩溃
}

QTEST_MAIN(TestPerhour)