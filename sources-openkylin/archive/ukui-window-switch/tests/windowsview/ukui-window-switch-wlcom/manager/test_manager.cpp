/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "test_manager.h"
#include "manager.h"
#include <QTest>
#include <QQuickView>

TestManager::TestManager(QObject *parent)
    : QObject(parent)
{
}

TestManager::~TestManager()
{
    cleanup();
}

void TestManager::cleanup()
{
    QTest::qWait(100);
}

void TestManager::testConstructor()
{
    manager m;
    QVERIFY(m.m_view != nullptr);
}

void TestManager::testManagerViewInitialized()
{
    manager m;
    QVERIFY(m.m_view != nullptr);
    QVERIFY(m.m_view.get() != nullptr);
    QVERIFY(qobject_cast<QQuickView*>(m.m_view.get()) != nullptr);
}

void TestManager::testManagerViewGeometry()
{
    manager m;
    QRect expectedGeometry(0, 0, 1000, 1000);
    m.m_view->setGeometry(expectedGeometry);
}

void TestManager::testManagerViewVisibility()
{
    manager m;
    QVERIFY(m.m_view->isVisible() == true);
}

#include "test_manager.moc"
