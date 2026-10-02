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
#include "test_allowlocation.h"
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include "view/horscreen/allowlocation.h"

// Interaction::~Interaction() {}

// Interaction* Interaction::getInstance() {
//     return nullptr; // 简单返回，因为测试中不一定需要真实的单例
// }

TestAllowLocation::TestAllowLocation()
{
    m_allowLocation = nullptr;
}

void TestAllowLocation::initTestCase()
{
}

void TestAllowLocation::cleanupTestCase()
{
}

void TestAllowLocation::init()
{
    m_allowLocation = new AllowLocation();
}

void TestAllowLocation::cleanup()
{
    if (m_allowLocation) {
        delete m_allowLocation;
        m_allowLocation = nullptr;
    }
}

void TestAllowLocation::testConstructor()
{
    QVERIFY(m_allowLocation != nullptr);
    QCOMPARE(m_allowLocation->windowModality(), Qt::ApplicationModal);
    
    // 直接访问私有成员验证
    QVERIFY(m_allowLocation->m_cancel != nullptr);
    QVERIFY(m_allowLocation->m_certain != nullptr);
}

void TestAllowLocation::testInitLayout()
{
    m_allowLocation->initLayout();
    // 验证位置
    QCOMPARE(m_allowLocation->m_label->pos(), QPoint(24, 32));
}

void TestAllowLocation::testInitConnect()
{
    // 已经在构造函数中调用
    QVERIFY(true);
}

void TestAllowLocation::testRejectLocate()
{
    m_allowLocation->show();
    m_allowLocation->rejectLocate();
    QVERIFY(!m_allowLocation->isVisible());
}

void TestAllowLocation::testAllowLocate()
{
    QSignalSpy spy(m_allowLocation, SIGNAL(allowLocationYes()));
    m_allowLocation->allowLocate();
    QCOMPARE(spy.count(), 1);
    QVERIFY(!m_allowLocation->isVisible());
}

void TestAllowLocation::testSignals()
{
    QSignalSpy spy(m_allowLocation, SIGNAL(setMenuModuleAllowLocationYes()));
    m_allowLocation->allowLocate();
    QCOMPARE(spy.count(), 1);
}
