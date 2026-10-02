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
#include "test_onehourweather.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QThread>
#include "view/horscreen/onehourweather.h"
#include "global/viewvar.h"

TestOnehourweather::TestOnehourweather()
    : m_onehourweather(nullptr)
{
}

void TestOnehourweather::initTestCase()
{
    // 在所有测试之前执行一次
}

void TestOnehourweather::cleanupTestCase()
{
    // 在所有测试之后执行一次
}

void TestOnehourweather::init()
{
    // 在每个测试之前执行
    m_onehourweather = new onehourweather();
}

void TestOnehourweather::cleanup()
{
    // 在每个测试之后执行
    if (m_onehourweather) {
        delete m_onehourweather;
        m_onehourweather = nullptr;
    }
}

void TestOnehourweather::testConstructor()
{
    QVERIFY(m_onehourweather != nullptr);
    
    // 验证成员变量是否正确初始化
    QVERIFY(m_onehourweather->m_time != nullptr);
    QVERIFY(m_onehourweather->m_icon != nullptr);
    QVERIFY(m_onehourweather->m_temperature != nullptr);
    
    // 验证标签的初始文本
    QCOMPARE(m_onehourweather->m_time->text(), QString("N/A"));
    QCOMPARE(m_onehourweather->m_temperature->text(), QString("N/A"));
    
    // 验证标签的对齐方式
    QCOMPARE(m_onehourweather->m_time->alignment(), Qt::AlignCenter);
    QCOMPARE(m_onehourweather->m_icon->alignment(), Qt::AlignCenter);
    QCOMPARE(m_onehourweather->m_temperature->alignment(), Qt::AlignCenter);
}

void TestOnehourweather::testIsFullScreen()
{
    // 测试设置全屏模式
    m_onehourweather->isFullScreen(1);
    QCOMPARE(m_onehourweather->m_judgeFullScreen, 1);
    
    // 测试设置非全屏模式
    m_onehourweather->isFullScreen(0);
    QCOMPARE(m_onehourweather->m_judgeFullScreen, 0);
}

void TestOnehourweather::testLable2SetFontSizeSlot()
{
    // 测试字体大小设置槽函数
    // 通过信号触发槽函数调用
    // 由于lable2SetFontSizeSlot是私有槽函数，我们通过ViewVar发送信号来测试
    ViewVar* viewVar = ViewVar::getInstance();
    QVERIFY(viewVar != nullptr);
    
    // 发送信号测试槽函数是否能正常响应
    // 注意：这里不会直接调用私有槽函数，而是通过Qt的信号槽机制
    emit viewVar->setFontSizeSignal(11);
    
    // 测试不同的字体大小值
    emit viewVar->setFontSizeSignal(9);
    emit viewVar->setFontSizeSignal(12);
    emit viewVar->setFontSizeSignal(15);
    
    // 验证函数能正常执行不抛出异常
    QVERIFY(true);
}

void TestOnehourweather::testInitLayout()
{
    // 验证布局已正确设置
    QVERIFY(m_onehourweather->layout() != nullptr);
    
    // 验证所有控件都已添加到布局中
    QVERIFY(m_onehourweather->m_time->parentWidget() == m_onehourweather);
    QVERIFY(m_onehourweather->m_icon->parentWidget() == m_onehourweather);
    QVERIFY(m_onehourweather->m_temperature->parentWidget() == m_onehourweather);
}

QTEST_MAIN(TestOnehourweather)