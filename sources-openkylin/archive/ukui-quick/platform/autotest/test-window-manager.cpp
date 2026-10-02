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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QtTest>
#include <type_traits>

#include "window-manager.h"

using namespace UkuiQuick;

namespace {

class SessionTypeGuard
{
public:
    SessionTypeGuard()
        : m_hadValue(qEnvironmentVariableIsSet("XDG_SESSION_TYPE"))
        , m_original(qgetenv("XDG_SESSION_TYPE"))
    {
    }

    ~SessionTypeGuard()
    {
        if (m_hadValue) {
            qputenv("XDG_SESSION_TYPE", m_original);
        } else {
            qunsetenv("XDG_SESSION_TYPE");
        }
    }

private:
    bool m_hadValue;
    QByteArray m_original;
};

}

class window_manager_test : public QObject
{
    Q_OBJECT

private slots:
    void detectsWaylandSessionFromEnvironment();
    void treatsOtherOrMissingSessionTypesAsNonWayland();
    void moveWindowToDesktopHasCurrentSignature();
    void windowDesktopsHasCurrentSignature();
    void skipSwitcherHasCurrentSignature();
    void skipSwitcherChangedHasCurrentSignature();
    void waylandBackendHandlesUnknownWindows();
};

void window_manager_test::detectsWaylandSessionFromEnvironment()
{
    SessionTypeGuard guard;

    qputenv("XDG_SESSION_TYPE", QByteArrayLiteral("wayland"));
    QVERIFY(WindowManager::isWaylandSession());
}

void window_manager_test::treatsOtherOrMissingSessionTypesAsNonWayland()
{
    SessionTypeGuard guard;

    qputenv("XDG_SESSION_TYPE", QByteArrayLiteral("x11"));
    QVERIFY(!WindowManager::isWaylandSession());

    qputenv("XDG_SESSION_TYPE", QByteArrayLiteral("tty"));
    QVERIFY(!WindowManager::isWaylandSession());

    qunsetenv("XDG_SESSION_TYPE");
    QVERIFY(!WindowManager::isWaylandSession());
}

void window_manager_test::moveWindowToDesktopHasCurrentSignature()
{
    using MoveWindowToDesktop = void (*)(const QString &, const QString &, const QString &);

    QVERIFY((std::is_same<decltype(&WindowManager::moveWindowToDesktop), MoveWindowToDesktop>::value));
}

void window_manager_test::windowDesktopsHasCurrentSignature()
{
    using WindowDesktops = QStringList (*)(const QString &);

    QVERIFY((std::is_same<decltype(&WindowManager::windowDesktops), WindowDesktops>::value));
}

void window_manager_test::skipSwitcherHasCurrentSignature()
{
    using SkipSwitcher = bool (*)(const QString &);

    QVERIFY((std::is_same<decltype(&WindowManager::skipSwitcher), SkipSwitcher>::value));
}

void window_manager_test::skipSwitcherChangedHasCurrentSignature()
{
    using SkipSwitcherChanged = void (WindowManager::*)(QString);

    QVERIFY((std::is_same<decltype(&WindowManager::skipSwitcherChanged), SkipSwitcherChanged>::value));
}

void window_manager_test::waylandBackendHandlesUnknownWindows()
{
    SessionTypeGuard guard;
    const QString windowId = QStringLiteral("missing-window");

    qputenv("XDG_SESSION_TYPE", QByteArrayLiteral("wayland"));

    QTest::ignoreMessage(QtWarningMsg, "get windowDesktops failed, Window not found \"missing-window\"");
    QCOMPARE(WindowManager::windowDesktops(windowId), QStringList());

    QTest::ignoreMessage(QtWarningMsg, "skipSwitcher failed, Window not found \"missing-window\"");
    QVERIFY(!WindowManager::skipSwitcher(windowId));
    QVERIFY(!WindowManager::skipTaskBar(windowId));

    QTest::ignoreMessage(QtWarningMsg, "moveWindowToDesktop failed, Window not found \"missing-window\"");
    WindowManager::moveWindowToDesktop(windowId, QStringLiteral("desktop-a"), QStringLiteral("desktop-b"));

    QTest::ignoreMessage(QtWarningMsg, "moveWindowToDesktop failed, Window already on target desktop \"missing-window\"");
    WindowManager::moveWindowToDesktop(windowId, QStringLiteral("desktop-a"), QStringLiteral("desktop-a"));
}

QTEST_GUILESS_MAIN(window_manager_test)

#include "test-window-manager.moc"
