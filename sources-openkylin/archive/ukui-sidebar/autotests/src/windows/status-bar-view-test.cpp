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

#define private public
#define protected public
#include "../../../src/windows/status-bar-view.h"
#undef protected
#undef private

#include "global-settings.h"
#include "hand-gesture-helper.h"
#include "shared-engine-view.h"
#include "sidebar-window-helper.h"
#include "window-helper.h"

class statusBarView_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_initialStateAndContext();
    void test_tabletModeVisibilityAndSignal();
    void test_updateGeometry_usesHelper();
    void test_updateGeometry_ignoresEmptyGeometry();
    void test_mouseGestureEvents();
    void test_touchEventsWithNoTargetAreIgnored();
    void test_moveAndExposeEvents();
    void test_initLoadsSource();
};

void statusBarView_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->reset();
    Sidebar::HandGestureHelper::getInstance()->reset();
    Sidebar::SidebarWindowHelper::instance()->reset();
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);
    Sidebar::SidebarWindowHelper::instance()->setWindowGeometry(Sidebar::SidebarWindowType::StatusBar,
                                                                QRect(10, 20, 300, 2));
}

void statusBarView_test::test_initialStateAndContext()
{
    StatusBarView view;

    QCOMPARE(view.isTabletModel(), false);
    QCOMPARE(view.isVisible(), false);
    QCOMPARE(view.geometry(), QRect(10, 20, 300, 2));
    QCOMPARE(view.m_windowProxy->position(), QPoint(10, 20));
    QCOMPARE(view.m_windowProxy->windowType(), UkuiQuick::WindowType::Switcher);
    QCOMPARE(view.rootContext()->contextProperty("statusBarView").value<QObject *>(), static_cast<QObject *>(&view));
    QCOMPARE(view.rootContext()->contextProperty("handGestureHelper").value<QObject *>(),
             static_cast<QObject *>(Sidebar::HandGestureHelper::getInstance()));
}

void statusBarView_test::test_tabletModeVisibilityAndSignal()
{
    StatusBarView view;
    QSignalSpy spy(&view, &StatusBarView::isTabletModelChanged);

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    QCOMPARE(view.isTabletModel(), true);
    QCOMPARE(view.isVisible(), true);
    QCOMPARE(spy.count(), 1);

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);
    QCOMPARE(view.isVisible(), false);
    QCOMPARE(spy.count(), 2);
}

void statusBarView_test::test_updateGeometry_usesHelper()
{
    StatusBarView view;

    Sidebar::SidebarWindowHelper::instance()->setWindowGeometry(Sidebar::SidebarWindowType::StatusBar,
                                                                QRect(0, 0, 500, 2));

    QCOMPARE(view.geometry(), QRect(0, 0, 500, 2));
    QCOMPARE(view.m_windowProxy->position(), QPoint(0, 0));
}

void statusBarView_test::test_updateGeometry_ignoresEmptyGeometry()
{
    StatusBarView view;
    QRect before = view.geometry();

    Sidebar::SidebarWindowHelper::instance()->setWindowGeometry(Sidebar::SidebarWindowType::StatusBar, QRect());
    view.updateGeometry();

    QCOMPARE(view.geometry(), before);
}

void statusBarView_test::test_mouseGestureEvents()
{
    StatusBarView view;
    QSignalSpy callSpy(Sidebar::HandGestureHelper::getInstance(), &Sidebar::HandGestureHelper::notificationCenterCalled);
    QSignalSpy releaseSpy(Sidebar::HandGestureHelper::getInstance(), &Sidebar::HandGestureHelper::top2BottomReleased);

    QMouseEvent ignoredPress(QEvent::MouseButtonPress, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    view.event(&ignoredPress);
    QCOMPARE(view.m_isPressed, false);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(4, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(view.event(&press));
    QCOMPARE(view.m_isPressed, true);

    QMouseEvent move(QEvent::MouseMove, QPointF(7, 42), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(view.event(&move));
    QCOMPARE(callSpy.count(), 1);
    QCOMPARE(callSpy.at(0).at(0).toInt(), 42);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(8, 60), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(view.event(&release));
    QCOMPARE(view.m_isPressed, false);
    QCOMPARE(releaseSpy.count(), 1);
    QCOMPARE(releaseSpy.at(0).at(0).toInt(), 8);
    QCOMPARE(releaseSpy.at(0).at(1).toInt(), 60);

    QMouseEvent ignoredMove(QEvent::MouseMove, QPointF(7, 99), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    view.event(&ignoredMove);
    QCOMPARE(callSpy.count(), 1);

    QMouseEvent ignoredRelease(QEvent::MouseButtonRelease, QPointF(3, 4), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    view.event(&ignoredRelease);
    QCOMPARE(releaseSpy.count(), 1);
}

void statusBarView_test::test_touchEventsWithNoTargetAreIgnored()
{
    StatusBarView view;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QList<QTouchEvent::TouchPoint> points;
    QTouchEvent begin(QEvent::TouchBegin, nullptr, Qt::NoModifier, Qt::TouchPointPressed, points);
    QTouchEvent update(QEvent::TouchUpdate, nullptr, Qt::NoModifier, Qt::TouchPointMoved, points);
    QTouchEvent end(QEvent::TouchEnd, nullptr, Qt::NoModifier, Qt::TouchPointReleased, points);
#else
    QList<QEventPoint> points;
    QTouchEvent begin(QEvent::TouchBegin, nullptr, Qt::NoModifier, points);
    QTouchEvent update(QEvent::TouchUpdate, nullptr, Qt::NoModifier, points);
    QTouchEvent end(QEvent::TouchEnd, nullptr, Qt::NoModifier, points);
#endif

    view.event(&begin);
    QCOMPARE(view.m_isPressed, false);

    view.m_isPressed = true;
    view.event(&update);
    QCOMPARE(view.m_isPressed, true);

    view.event(&end);
    QCOMPARE(view.m_isPressed, true);
}

void statusBarView_test::test_moveAndExposeEvents()
{
    StatusBarView view;
    Sidebar::SidebarWindowHelper::instance()->setWindowGeometry(Sidebar::SidebarWindowType::StatusBar,
                                                                QRect(20, 30, 700, 2));

    QMoveEvent move(QPoint(1, 1), QPoint(0, 0));
    QVERIFY(view.event(&move));
    QCOMPARE(view.geometry(), QRect(20, 30, 700, 2));

    QExposeEvent expose(QRegion(0, 0, 10, 10));
    view.event(&expose);
}

void statusBarView_test::test_initLoadsSource()
{
    StatusBarView view;

    view.init();

    QCOMPARE(view.isTabletModel(), false);
}

QTEST_MAIN(statusBarView_test)

#include "status-bar-view-test.moc"
