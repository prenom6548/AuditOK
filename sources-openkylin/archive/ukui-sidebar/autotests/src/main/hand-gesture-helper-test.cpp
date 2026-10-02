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
#include "../../../src/main/hand-gesture-helper.h"

using namespace Sidebar;

class handGestureHelper_test : public QObject
{
    Q_OBJECT

private slots:
    void test_getInstance();
    void test_getInstance_returnsSameInstance();
    void test_callNotificationCenter_emitsSignal();
    void test_callNotificationCenter_correctValue();
    void test_callNotificationCenter_negativeValue();
    void test_callNotificationCenter_zeroValue();
    void test_callControlCenter_emitsSignal();
    void test_callControlCenter_correctValue();
    void test_callControlCenter_negativeValue();
    void test_callControlCenter_zeroValue();
    void test_top2BottomRelease_emitsSignal();
    void test_top2BottomRelease_correctValues();
    void test_top2BottomRelease_negativeValues();
    void test_top2BottomRelease_zeroValues();
    void test_right2LeftRelease_emitsSignal();
    void test_right2LeftRelease_correctValues();
    void test_right2LeftRelease_negativeValues();
    void test_right2LeftRelease_zeroValues();
    void test_multipleSignalEmissions();
};

void handGestureHelper_test::test_getInstance()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QVERIFY(helper != nullptr);
}

void handGestureHelper_test::test_getInstance_returnsSameInstance()
{
    HandGestureHelper *helper1 = HandGestureHelper::getInstance();
    HandGestureHelper *helper2 = HandGestureHelper::getInstance();
    QCOMPARE(helper1, helper2);
}

void handGestureHelper_test::test_callNotificationCenter_emitsSignal()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::notificationCenterCalled);
    helper->callNotificationCenter(100);
    QCOMPARE(spy.count(), 1);
}

void handGestureHelper_test::test_callNotificationCenter_correctValue()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::notificationCenterCalled);
    helper->callNotificationCenter(200);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 200);
}

void handGestureHelper_test::test_callNotificationCenter_negativeValue()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::notificationCenterCalled);
    helper->callNotificationCenter(-50);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), -50);
}

void handGestureHelper_test::test_callNotificationCenter_zeroValue()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::notificationCenterCalled);
    helper->callNotificationCenter(0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
}

void handGestureHelper_test::test_callControlCenter_emitsSignal()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::controlCenterCalled);
    helper->callControlCenter(300);
    QCOMPARE(spy.count(), 1);
}

void handGestureHelper_test::test_callControlCenter_correctValue()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::controlCenterCalled);
    helper->callControlCenter(400);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 400);
}

void handGestureHelper_test::test_callControlCenter_negativeValue()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::controlCenterCalled);
    helper->callControlCenter(-100);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), -100);
}

void handGestureHelper_test::test_callControlCenter_zeroValue()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::controlCenterCalled);
    helper->callControlCenter(0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
}

void handGestureHelper_test::test_top2BottomRelease_emitsSignal()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::top2BottomReleased);
    helper->top2BottomRelease(10, 20);
    QCOMPARE(spy.count(), 1);
}

void handGestureHelper_test::test_top2BottomRelease_correctValues()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::top2BottomReleased);
    helper->top2BottomRelease(150, 250);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 150);
    QCOMPARE(spy.at(0).at(1).toInt(), 250);
}

void handGestureHelper_test::test_top2BottomRelease_negativeValues()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::top2BottomReleased);
    helper->top2BottomRelease(-10, -20);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), -10);
    QCOMPARE(spy.at(0).at(1).toInt(), -20);
}

void handGestureHelper_test::test_top2BottomRelease_zeroValues()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::top2BottomReleased);
    helper->top2BottomRelease(0, 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
    QCOMPARE(spy.at(0).at(1).toInt(), 0);
}

void handGestureHelper_test::test_right2LeftRelease_emitsSignal()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::right2LeftReleased);
    helper->right2LeftRelease(500, 600);
    QCOMPARE(spy.count(), 1);
}

void handGestureHelper_test::test_right2LeftRelease_correctValues()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::right2LeftReleased);
    helper->right2LeftRelease(700, 800);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 700);
    QCOMPARE(spy.at(0).at(1).toInt(), 800);
}

void handGestureHelper_test::test_right2LeftRelease_negativeValues()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::right2LeftReleased);
    helper->right2LeftRelease(-30, -40);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), -30);
    QCOMPARE(spy.at(0).at(1).toInt(), -40);
}

void handGestureHelper_test::test_right2LeftRelease_zeroValues()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy spy(helper, &HandGestureHelper::right2LeftReleased);
    helper->right2LeftRelease(0, 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 0);
    QCOMPARE(spy.at(0).at(1).toInt(), 0);
}

void handGestureHelper_test::test_multipleSignalEmissions()
{
    HandGestureHelper *helper = HandGestureHelper::getInstance();
    QSignalSpy notifSpy(helper, &HandGestureHelper::notificationCenterCalled);
    QSignalSpy ctrlSpy(helper, &HandGestureHelper::controlCenterCalled);
    QSignalSpy topSpy(helper, &HandGestureHelper::top2BottomReleased);
    QSignalSpy rightSpy(helper, &HandGestureHelper::right2LeftReleased);

    helper->callNotificationCenter(1);
    helper->callNotificationCenter(2);
    helper->callControlCenter(3);
    helper->top2BottomRelease(4, 5);
    helper->right2LeftRelease(6, 7);

    QCOMPARE(notifSpy.count(), 2);
    QCOMPARE(ctrlSpy.count(), 1);
    QCOMPARE(topSpy.count(), 1);
    QCOMPARE(rightSpy.count(), 1);
}

QTEST_MAIN(handGestureHelper_test)

#include "hand-gesture-helper-test.moc"
