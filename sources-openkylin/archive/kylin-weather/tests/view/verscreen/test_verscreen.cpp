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
#include "test_verscreen.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QApplication>
#include <QWidget>
#include "../../../view/verscreen/verscreen.h"

TestVerscreen::TestVerscreen()
    : m_verscreen(nullptr)
{
}

void TestVerscreen::initTestCase()
{
    // 在所有测试之前执行一次
}

void TestVerscreen::cleanupTestCase()
{
    // 在所有测试之后执行一次
}

void TestVerscreen::init()
{
    // 在每个测试之前执行
    m_verscreen = new verscreen();
}

void TestVerscreen::cleanup()
{
    // 在每个测试之后执行
    if (m_verscreen) {
        delete m_verscreen;
        m_verscreen = nullptr;
    }
}

void TestVerscreen::testConstructor()
{
    // 测试构造函数是否成功创建对象
    QVERIFY(m_verscreen != nullptr);
    
    // 测试对象类型是否正确
    QCOMPARE(m_verscreen->metaObject()->className(), "verscreen");
}

void TestVerscreen::testDefaultProperties()
{
    // 测试默认尺寸是否正确设置
    QCOMPARE(m_verscreen->width(), 900);
    QCOMPARE(m_verscreen->height(), 620);
    
    // 测试默认可见性
    QVERIFY(!m_verscreen->isVisible());
    
    // 测试父对象是否正确设置
    QVERIFY(m_verscreen->parent() == nullptr);
}

QTEST_MAIN(TestVerscreen)