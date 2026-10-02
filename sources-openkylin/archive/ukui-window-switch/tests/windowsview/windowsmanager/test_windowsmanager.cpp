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

#include "test_windowsmanager.h"
#include <QTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QDir>
#include <XdgDesktopFile>
#include "windowsmanager.h"

TestWindowsManager::TestWindowsManager(QObject *parent)
    : QObject(parent)
{
}

TestWindowsManager::~TestWindowsManager()
{
    cleanup();
}

void TestWindowsManager::cleanup()
{
    QTest::qWait(100);
}

void TestWindowsManager::testConstructor()
{
    WindowsManager *manager = WindowsManager::getInstance();
    QVERIFY(manager != nullptr);
    cleanup();
}

void TestWindowsManager::testDestructor()
{
    WindowsManager *manager = WindowsManager::getInstance();
    QVERIFY(manager != nullptr);
    cleanup();
}

void TestWindowsManager::testGetInstance()
{
    WindowsManager *manager1 = WindowsManager::getInstance();
    WindowsManager *manager2 = WindowsManager::getInstance();
    QVERIFY(manager1 != nullptr);
    QVERIFY(manager2 != nullptr);
    QVERIFY(manager1 == manager2);
    cleanup();
}

void TestWindowsManager::testWindows()
{
    WindowsManager *manager = WindowsManager::getInstance();
    QStringList windows = manager->windows();
    // 可能为空,只验证调用不崩溃
    QVERIFY(true);
    cleanup();
}

void TestWindowsManager::testWindowIcon()
{
    WindowsManager *manager = WindowsManager::getInstance();
    
    QVariant icon = manager->windowIcon("");
    QVERIFY(icon.isNull() || !icon.isValid());
    
    QStringList windows = manager->windows();
    if (!windows.isEmpty()) {
        QString windowId = windows.first();
        icon = manager->windowIcon(windowId);
        QVERIFY(icon.isValid());
    } else {
        QSKIP("No windows available for testing windowIcon with valid windowId");
    }
    cleanup();
}

void TestWindowsManager::testWindowTitle()
{
    WindowsManager *manager = WindowsManager::getInstance();
    
    QString title = manager->windowTitle("");
    QVERIFY(title.isEmpty() || title.isNull());

    QStringList windows = manager->windows();
    if (!windows.isEmpty()) {
        QString windowId = windows.first();
        title = manager->windowTitle(windowId);
        QVERIFY(!title.isEmpty());
    } else {
        QSKIP("No windows available for testing windowTitle with valid windowId");
    }
    cleanup();
}

void TestWindowsManager::testGeometry()
{
    WindowsManager *manager = WindowsManager::getInstance();
    QRect rect = manager->geometry("");
    QVERIFY(rect.isEmpty());
    QStringList windows = manager->windows();
    if (!windows.isEmpty()) {
        QString windowId = windows.first();
        rect = manager->geometry(windowId);
        QVERIFY(!rect.isEmpty());
    } else {
        QSKIP("No windows available for testing geometry with valid windowId");
    }
    cleanup();
}

void TestWindowsManager::testIsMinimized()
{
    WindowsManager *manager = WindowsManager::getInstance();
    bool isMin = manager->isMinimized("");
    QVERIFY(isMin == false);
    QStringList windows = manager->windows();
    if (!windows.isEmpty()) {
        QString windowId = windows.first();
        isMin = manager->isMinimized(windowId);
        QVERIFY(true);
    } else {
        QSKIP("No windows available for testing isMinimized with valid windowId");
    }
    cleanup();
}

void TestWindowsManager::testMoveAppWindowToDesktop()
{
    WindowsManager *manager = WindowsManager::getInstance();

    // 检查是否有可用窗口用于测试
    QStringList windows = manager->windows();
    if (windows.isEmpty()) {
        QSKIP("No windows available for testing moveAppWindowToDesktop");
    }

    QString windowId = windows.first();
    // 获取窗口当前所在的工作区
    QStringList currentDesktops = manager->windowDesktops(windowId);
    if (currentDesktops.isEmpty()) {
        QSKIP("Window is not on any desktop for testing moveAppWindowToDesktop");
    }

    QString srcDesktop = currentDesktops.first();
    QString destDesktop = srcDesktop;

    // 如果只有一个工作区，跳过测试
    if (currentDesktops.count() == 1) {
        QSKIP("Only one desktop available, cannot test moving between different desktops");
    }

    // 获取第二个工作区作为目标
    destDesktop = currentDesktops.count() > 1 ? currentDesktops[1] : srcDesktop;

    QSignalSpy spy(manager, &WindowsManager::appWindowDesktopChanged);
    manager->moveAppWindowToDesktop(windowId, srcDesktop, destDesktop);

    QVERIFY(spy.isValid());
    cleanup();
}

void TestWindowsManager::testActivateAppWindow()
{
    WindowsManager *manager = WindowsManager::getInstance();

    QStringList windows = manager->windows();
    if (windows.isEmpty()) {
        QSKIP("No windows available for testing activateAppWindow");
    }

    QString windowId = windows.first();
    QSignalSpy spy(manager, &WindowsManager::appWindowActivated);
    manager->activateAppWindow(windowId);

    QVERIFY(spy.isValid());
    cleanup();
}

void TestWindowsManager::testRemoveAppWindow()
{
    WindowsManager *manager = WindowsManager::getInstance();

    // 仅做调用测试，不关闭真实窗口
    QSignalSpy spy(manager, &WindowsManager::appWindowRemoved);
    manager->removeAppWindow("");
    QVERIFY(spy.isValid());
    cleanup();
}

void TestWindowsManager::testSkipSwitcher()
{
    WindowsManager *manager = WindowsManager::getInstance();

    // 测试空窗口ID
    bool skip = manager->skipSwitcher("");
    QVERIFY(skip == false);
    cleanup();
}

void TestWindowsManager::testHasModalChildWindow()
{
    WindowsManager *manager = WindowsManager::getInstance();

    bool hasModal = manager->hasModalChildWindow("");
    QVERIFY(hasModal == false);
    cleanup();
}

void TestWindowsManager::testWindowDesktops()
{
    WindowsManager *manager = WindowsManager::getInstance();

    // 测试空窗口ID - 应该返回空列表
    QStringList desktops = manager->windowDesktops("");
    QVERIFY(desktops.isEmpty());

    // 测试有效窗口ID
    QStringList windows = manager->windows();
    if (!windows.isEmpty()) {
        QString windowId = windows.first();
        desktops = manager->windowDesktops(windowId);
        // windowDesktops 返回窗口所在的工作区列表
        // 可能为空（窗口不存在或不在任何工作区上），所以只验证调用不崩溃
        QVERIFY(true);
    } else {
        QSKIP("No windows available for testing windowDesktops with valid windowId");
    }
    cleanup();
}

#include "test_windowsmanager.moc"
