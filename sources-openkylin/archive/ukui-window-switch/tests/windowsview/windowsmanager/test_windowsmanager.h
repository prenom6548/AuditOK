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

#ifndef TEST_WINDOWSMANAGER_H
#define TEST_WINDOWSMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRect>
#include <QVariant>

class TestWindowsManager : public QObject
{
    Q_OBJECT
public:
    explicit TestWindowsManager(QObject *parent = nullptr);
    ~TestWindowsManager();

private slots:
    void testConstructor();
    void testDestructor();
    void testGetInstance();
    void testWindows();
    void testWindowIcon();
    void testWindowTitle();
    void testGeometry();
    void testIsMinimized();
    void testMoveAppWindowToDesktop();
    void testActivateAppWindow();
    void testRemoveAppWindow();
    void testSkipSwitcher();
    void testHasModalChildWindow();
    void testWindowDesktops();

private:
    void cleanup();
};

#endif // TEST_WINDOWSMANAGER_H
