/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 *
 */

#ifndef TEST_KYWCDESKTOPMANAGER_H
#define TEST_KYWCDESKTOPMANAGER_H

#include <QObject>

class KywcDesktopManager;

class TestKywcDesktopManager : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testConstructor();
    void testDestructor();
    void testGetNumberOfDesktops();
    void testGetCurrentDesktop();
    void testAppendDesktop();
    void testSetCurrentDesktop();
    void testGetFullScreenGeometry();
    void testSetDesktopName();
    void testGetOutputUuid();
    void testGetDesktopUuid();
    void testDesktopRemovedSignal();
    void testDesktopAppendedSignal();
    void testDesktopMovedSignal();
    void testCurrentDesktopChangedSignal();
    void testMoveDesktop();
    void testRemoveDesktop();
    void testIsCurrentDesktopShowingDesktop();

private:
    KywcDesktopManager *m_manager = nullptr;
};

#endif // TEST_KYWCDESKTOPMANAGER_H
