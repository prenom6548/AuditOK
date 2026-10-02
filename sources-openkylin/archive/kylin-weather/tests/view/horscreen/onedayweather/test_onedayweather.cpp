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
#include "test_onedayweather.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QThread>
#include "view/horscreen/onedayweather.h"
#include "global/corevar.h"

TestOnedayweather::TestOnedayweather()
    : m_onedayweather(nullptr)
{
}

void TestOnedayweather::initTestCase()
{
    // 在所有测试之前执行一次
}

void TestOnedayweather::cleanupTestCase()
{
    // 在所有测试之后执行一次
}

void TestOnedayweather::init()
{
    // 在每个测试之前执行
    m_onedayweather = new onedayweather();
}

void TestOnedayweather::cleanup()
{
    // 在每个测试之后执行
    if (m_onedayweather) {
        delete m_onedayweather;
        m_onedayweather = nullptr;
    }
}

void TestOnedayweather::testConstructor()
{
    QVERIFY(m_onedayweather != nullptr);
    
    // 验证成员变量是否正确初始化
    QVERIFY(m_onedayweather->m_dayOfWeek != nullptr);
    QVERIFY(m_onedayweather->m_date != nullptr);
    QVERIFY(m_onedayweather->m_weatherIcon != nullptr);
    QVERIFY(m_onedayweather->m_temp != nullptr);
    QVERIFY(m_onedayweather->m_weather != nullptr);
    QVERIFY(m_onedayweather->m_wind != nullptr);
    
    // 验证标签的初始文本
    QCOMPARE(m_onedayweather->m_dayOfWeek->text(), QString("N/A"));
    QCOMPARE(m_onedayweather->m_date->text(), QString("N/A"));
    QCOMPARE(m_onedayweather->m_temp->text(), QString("N/A"));
    QCOMPARE(m_onedayweather->m_weather->text(), QString("N/A"));
    QCOMPARE(m_onedayweather->m_wind->text(), QString("N/A"));
}

void TestOnedayweather::testIsFullScreen()
{
    // 测试设置全屏模式
    m_onedayweather->isFullScreen(1);
    QCOMPARE(m_onedayweather->m_judgeFullScreen, 1);
    
    // 测试设置非全屏模式
    m_onedayweather->isFullScreen(0);
    QCOMPARE(m_onedayweather->m_judgeFullScreen, 0);
}

void TestOnedayweather::testInitLayout()
{
    // 验证布局已正确设置
    QVERIFY(m_onedayweather->layout() != nullptr);
    
    // 验证所有控件都已添加到布局中
    QVERIFY(m_onedayweather->m_dayOfWeek->parentWidget() == m_onedayweather);
    QVERIFY(m_onedayweather->m_date->parentWidget() == m_onedayweather);
    QVERIFY(m_onedayweather->m_weatherIcon->parentWidget() == m_onedayweather);
    QVERIFY(m_onedayweather->m_temp->parentWidget() == m_onedayweather);
    QVERIFY(m_onedayweather->m_weather->parentWidget() == m_onedayweather);
    QVERIFY(m_onedayweather->m_wind->parentWidget() == m_onedayweather);
}

QTEST_MAIN(TestOnedayweather)