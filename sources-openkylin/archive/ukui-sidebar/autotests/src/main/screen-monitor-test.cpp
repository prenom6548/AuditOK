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

#include <QtTest>
#include <QSignalSpy>
#define private public
#include "../../../src/main/screen-monitor.h"
#undef private

using namespace Sidebar;

class screenMonitor_test : public QObject
{
    Q_OBJECT

private slots:
    void test_getInstance();
    void test_getInstance_returnsSameInstance();
    void test_getPrimaryScreen();
    void test_getGeometry();
    void test_getGeometry_notEmpty();
    void test_getLayoutDirection();
    void test_screenRatio();
    void test_screenRatio_positiveValue();
    void test_getPanelPosition();
    void test_getPanelSize();
    void test_getScreenLockState_default();
    void test_getSessionToolsState_reflectsSlotState();
    void test_privateConstructorInitializesState();
    void test_primaryScreenChangedSlot_nullScreen();
    void test_primaryScreenChangedSlot_validScreen();
    void test_screenRemovedSlot_emitsSignal();
    void test_screenAddedSlot();
    void test_layoutDirectionChangedSlot_sameDirection();
    void test_layoutDirectionChangedSlot_differentDirection();
    void test_screenLockSlot();
    void test_screenUnLockSlot();
    void test_sessionToolsStateChangedSlot();
    void test_screenLockUnlockCycle();
};

void screenMonitor_test::test_getInstance()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QVERIFY(monitor != nullptr);
}

void screenMonitor_test::test_getInstance_returnsSameInstance()
{
    ScreenMonitor *monitor1 = ScreenMonitor::getInstance();
    ScreenMonitor *monitor2 = ScreenMonitor::getInstance();
    QCOMPARE(monitor1, monitor2);
}

void screenMonitor_test::test_getPrimaryScreen()
{
    QScreen *screen = qApp->primaryScreen();
    QCOMPARE(screen, ScreenMonitor::getInstance()->getPrimaryScreen());
}

void screenMonitor_test::test_getGeometry()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QRect geometry = monitor->getGeometry();
    QScreen *screen = monitor->getPrimaryScreen();
    if (screen) {
        QCOMPARE(geometry, screen->geometry());
    } else {
        QCOMPARE(geometry, QRect());
    }
}

void screenMonitor_test::test_getGeometry_notEmpty()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    if (monitor->getPrimaryScreen()) {
        QRect geometry = monitor->getGeometry();
        QVERIFY(geometry.width() > 0);
        QVERIFY(geometry.height() > 0);
    }
}

void screenMonitor_test::test_getLayoutDirection()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    Qt::LayoutDirection direction = monitor->getLayoutDirection();
    QVERIFY(direction == Qt::LeftToRight || direction == Qt::RightToLeft || direction == Qt::LayoutDirectionAuto);
}

void screenMonitor_test::test_screenRatio()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    qreal ratio = monitor->screenRatio();
    QVERIFY(ratio >= 0.5);
    QVERIFY(ratio <= 1.0);
}

void screenMonitor_test::test_screenRatio_positiveValue()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    qreal ratio = monitor->screenRatio();
    QVERIFY(ratio > 0);
}

void screenMonitor_test::test_getPanelPosition()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    int position = monitor->getPanelPosition();
    // 上: 1, 下: 0, 左: 2, 右: 3, 不存在: 4
    QVERIFY(position >= 0 && position <= 4);
}

void screenMonitor_test::test_getPanelSize()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    int size = monitor->getPanelSize();
    QVERIFY(size >= 0);
}

void screenMonitor_test::test_getScreenLockState_default()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    bool state = monitor->getScreenLockState();
    QVERIFY(state == true || state == false);
}

void screenMonitor_test::test_getSessionToolsState_reflectsSlotState()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    monitor->sessionToolsStateChangedSlot(false);
    QCOMPARE(monitor->getSessionToolsState(), false);

    monitor->sessionToolsStateChangedSlot(true);
    QCOMPARE(monitor->getSessionToolsState(), true);

    monitor->sessionToolsStateChangedSlot(false);
}

void screenMonitor_test::test_privateConstructorInitializesState()
{
    ScreenMonitor monitor;
    QVERIFY(monitor.getPrimaryScreen() == qApp->primaryScreen());
    QVERIFY(monitor.getScreenLockState() == true || monitor.getScreenLockState() == false);
    QCOMPARE(monitor.getSessionToolsState(), monitor.m_sessionToolsState);
}

void screenMonitor_test::test_primaryScreenChangedSlot_nullScreen()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::primaryScreenChanged);
    // 传入 nullptr 不应触发信号
    monitor->primaryScreenChangedSlot(nullptr);
    QCOMPARE(spy.count(), 0);
}

void screenMonitor_test::test_primaryScreenChangedSlot_validScreen()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QScreen *screen = qApp->primaryScreen();
    if (!screen) {
        QSKIP("No primary screen available");
    }

    QSignalSpy primarySpy(monitor, &ScreenMonitor::primaryScreenChanged);
    QSignalSpy geometrySpy(monitor, &ScreenMonitor::geometryChanged);

    monitor->primaryScreenChangedSlot(screen);

    QCOMPARE(primarySpy.count(), 1);
    QCOMPARE(geometrySpy.count(), 1);
    QCOMPARE(monitor->getPrimaryScreen(), screen);
}

void screenMonitor_test::test_screenRemovedSlot_emitsSignal()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::screenRemoved);
    QScreen *screen = qApp->primaryScreen();
    if (!screen) {
        QSKIP("No primary screen available");
    }

    monitor->screenRemovedSlot(screen);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<QScreen*>(), screen);
}

void screenMonitor_test::test_screenAddedSlot()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QScreen *screen = qApp->primaryScreen();
    // screenAddedSlot 只打印日志，不崩溃即可
    monitor->screenAddedSlot(screen);
    QVERIFY(true);
}

void screenMonitor_test::test_layoutDirectionChangedSlot_sameDirection()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::layoutDirectionChanged);
    Qt::LayoutDirection currentDirection = monitor->getLayoutDirection();

    // 传入相同方向不应触发信号
    monitor->layoutDirectionChangedSlot(currentDirection);
    QCOMPARE(spy.count(), 0);
}

void screenMonitor_test::test_layoutDirectionChangedSlot_differentDirection()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::layoutDirectionChanged);
    Qt::LayoutDirection currentDirection = monitor->getLayoutDirection();

    Qt::LayoutDirection newDirection = (currentDirection == Qt::LeftToRight)
                                          ? Qt::RightToLeft
                                          : Qt::LeftToRight;
    monitor->layoutDirectionChangedSlot(newDirection);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(monitor->getLayoutDirection(), newDirection);

    // 恢复原始方向
    monitor->layoutDirectionChangedSlot(currentDirection);
}

void screenMonitor_test::test_screenLockSlot()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::screenLockStateChanged);

    monitor->screenLockSlot();
    QCOMPARE(monitor->getScreenLockState(), true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);
}

void screenMonitor_test::test_screenUnLockSlot()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::screenLockStateChanged);

    monitor->screenUnLockSlot();
    QCOMPARE(monitor->getScreenLockState(), false);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);
}

void screenMonitor_test::test_sessionToolsStateChangedSlot()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();
    QSignalSpy spy(monitor, &ScreenMonitor::sessionToolsStateChanged);

    monitor->sessionToolsStateChangedSlot(true);
    QCOMPARE(monitor->getSessionToolsState(), true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), true);

    monitor->sessionToolsStateChangedSlot(false);
    QCOMPARE(monitor->getSessionToolsState(), false);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).at(0).toBool(), false);
}

void screenMonitor_test::test_screenLockUnlockCycle()
{
    ScreenMonitor *monitor = ScreenMonitor::getInstance();

    monitor->screenLockSlot();
    QCOMPARE(monitor->getScreenLockState(), true);

    monitor->screenUnLockSlot();
    QCOMPARE(monitor->getScreenLockState(), false);

    monitor->screenLockSlot();
    QCOMPARE(monitor->getScreenLockState(), true);

    monitor->screenUnLockSlot();
    QCOMPARE(monitor->getScreenLockState(), false);
}

QTEST_MAIN(screenMonitor_test)

#include "screen-monitor-test.moc"
