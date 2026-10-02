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
#include "../../../src/windows/right-hand-gesture-view.h"
#undef protected
#undef private

#include "global-settings.h"
#include "hand-gesture-helper.h"
#include "settings.h"
#include "sidebar-window-helper.h"
#include "window-helper.h"

using namespace Sidebar;

class rightHandGestureView_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_initialGeometryAndWindowType();
    void test_initShowsWindow();
    void test_updateGeometry_ignoresEmptyGeometry();
    void test_callControlCenter_ltrThresholds();
    void test_callControlCenter_rtlThresholds();
    void test_callControlCenterEnd_emitsRelease();
    void test_mouseEvents_driveGesture();
    void test_touchEvents_driveGesture();
    void test_moveEvent_updatesGeometry();
    void test_tabletModeChangeClearsPressed();
};

void rightHandGestureView_test::init()
{
    GlobalSettings::globalInstance()->reset();
    HandGestureHelper::getInstance()->reset();
    SidebarWindowHelper::instance()->reset();
    UkuiQuick::Settings::instance()->reset();
    SidebarWindowHelper::instance()->setWindowGeometry(SidebarWindowType::RightHandGesture, QRect(100, 20, 30, 400));
    SidebarWindowHelper::instance()->setLayoutDirection(Qt::LeftToRight);
}

void rightHandGestureView_test::test_initialGeometryAndWindowType()
{
    RightHandGestureView view;

    QCOMPARE(view.geometry(), QRect(100, 20, 30, 400));
    QCOMPARE(view.m_windowProxy->position(), QPoint(100, 20));
    QCOMPARE(view.m_windowProxy->windowType(), UkuiQuick::WindowType::SystemWindow);

    UkuiQuick::Settings::instance()->setPlatformName("wayland");
    RightHandGestureView waylandView;
    QCOMPARE(waylandView.m_windowProxy->windowType(), UkuiQuick::WindowType::Switcher);
}

void rightHandGestureView_test::test_initShowsWindow()
{
    RightHandGestureView view;

    view.init();

    QCOMPARE(view.isVisible(), true);
}

void rightHandGestureView_test::test_updateGeometry_ignoresEmptyGeometry()
{
    RightHandGestureView view;
    QRect before = view.geometry();

    SidebarWindowHelper::instance()->setWindowGeometry(SidebarWindowType::RightHandGesture, QRect());
    view.updateGeometry();

    QCOMPARE(view.geometry(), before);
}

void rightHandGestureView_test::test_callControlCenter_ltrThresholds()
{
    RightHandGestureView view;
    QSignalSpy spy(HandGestureHelper::getInstance(), &HandGestureHelper::controlCenterCalled);

    view.callControlCenter(-10);
    QCOMPARE(spy.count(), 0);

    view.callControlCenter(-30);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 82);
}

void rightHandGestureView_test::test_callControlCenter_rtlThresholds()
{
    RightHandGestureView view;
    SidebarWindowHelper::instance()->setLayoutDirection(Qt::RightToLeft);
    QSignalSpy spy(HandGestureHelper::getInstance(), &HandGestureHelper::controlCenterCalled);

    view.callControlCenter(10);
    QCOMPARE(spy.count(), 0);

    view.callControlCenter(25);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 13);
}

void rightHandGestureView_test::test_callControlCenterEnd_emitsRelease()
{
    RightHandGestureView view;
    QSignalSpy spy(HandGestureHelper::getInstance(), &HandGestureHelper::right2LeftReleased);

    view.callControlCenterEnd(-30, 5);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 82);
    QCOMPARE(spy.at(0).at(1).toInt(), 37);
}

void rightHandGestureView_test::test_mouseEvents_driveGesture()
{
    RightHandGestureView view;
    QSignalSpy callSpy(HandGestureHelper::getInstance(), &HandGestureHelper::controlCenterCalled);
    QSignalSpy releaseSpy(HandGestureHelper::getInstance(), &HandGestureHelper::right2LeftReleased);

    QMouseEvent ignoredPress(QEvent::MouseButtonPress, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    view.event(&ignoredPress);
    QCOMPARE(view.m_isPressed, false);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(view.event(&press));
    QCOMPARE(view.m_isPressed, true);

    QMouseEvent move(QEvent::MouseMove, QPointF(-40, 3), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(view.event(&move));
    QCOMPARE(callSpy.count(), 1);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(-40, 3), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QVERIFY(view.event(&release));
    QCOMPARE(view.m_isPressed, false);
    QCOMPARE(releaseSpy.count(), 1);

    QMouseEvent ignoredRelease(QEvent::MouseButtonRelease, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    view.event(&ignoredRelease);
    QCOMPARE(view.m_isPressed, false);

    QMouseEvent ignoredMove(QEvent::MouseMove, QPointF(-50, 3), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    view.event(&ignoredMove);
    QCOMPARE(callSpy.count(), 1);

    QExposeEvent expose(QRegion(0, 0, 10, 10));
    view.event(&expose);
}

void rightHandGestureView_test::test_touchEvents_driveGesture()
{
    RightHandGestureView view;
    QSignalSpy callSpy(HandGestureHelper::getInstance(), &HandGestureHelper::controlCenterCalled);
    QSignalSpy releaseSpy(HandGestureHelper::getInstance(), &HandGestureHelper::right2LeftReleased);

    QList<QTouchEvent::TouchPoint> points;
    QTouchEvent::TouchPoint point(1);
    point.setState(Qt::TouchPointPressed);
    point.setPos(QPointF(-40, 4));
    point.setScreenPos(QPointF(-40, 4));
    points.append(point);

    QTouchEvent ignoredBegin(QEvent::TouchBegin,
                             nullptr,
                             Qt::NoModifier,
                             Qt::TouchPointPressed,
                             points);
    view.event(&ignoredBegin);
    QCOMPARE(view.m_isPressed, false);

    QTouchEvent begin(QEvent::TouchBegin,
                      nullptr,
                      Qt::NoModifier,
                      Qt::TouchPointPressed,
                      points);
    begin.setWindow(&view);
    QVERIFY(view.event(&begin));
    QCOMPARE(view.m_isPressed, true);

    points[0].setState(Qt::TouchPointMoved);
    QTouchEvent update(QEvent::TouchUpdate,
                       nullptr,
                       Qt::NoModifier,
                       Qt::TouchPointMoved,
                       points);
    update.setWindow(&view);
    QVERIFY(view.event(&update));
    QCOMPARE(callSpy.count(), 1);

    points[0].setState(Qt::TouchPointReleased);
    QTouchEvent end(QEvent::TouchEnd,
                    nullptr,
                    Qt::NoModifier,
                    Qt::TouchPointReleased,
                    points);
    end.setWindow(&view);
    QVERIFY(view.event(&end));
    QCOMPARE(view.m_isPressed, false);
    QCOMPARE(releaseSpy.count(), 1);
}

void rightHandGestureView_test::test_moveEvent_updatesGeometry()
{
    RightHandGestureView view;
    SidebarWindowHelper::instance()->setWindowGeometry(SidebarWindowType::RightHandGesture, QRect(200, 10, 40, 300));

    QMoveEvent event(QPoint(1, 1), QPoint(0, 0));
    QVERIFY(view.event(&event));

    QCOMPARE(view.geometry(), QRect(200, 10, 40, 300));
}

void rightHandGestureView_test::test_tabletModeChangeClearsPressed()
{
    RightHandGestureView view;
    view.m_isPressed = true;

    GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    QCOMPARE(view.m_isPressed, false);
}

QTEST_MAIN(rightHandGestureView_test)

#include "right-hand-gesture-view-test.moc"
