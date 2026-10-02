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
#include <QWindow>

#define private public
#include "../../../src/windows/window-blur-helper.h"
#undef private

#include "window-helper.h"
#include "xcb/xcb.h"

using namespace Sidebar;

class windowBlurHelper_test : public QObject
{
    Q_OBJECT

private slots:
    void test_initialState();
    void test_updateBlurArea_withoutProxy();
    void test_enableWithoutRegionDisablesProxyBlur();
    void test_setRegionAndEnableUpdatesProxy();
    void test_radiusClampSignalsAndOpacity();
    void test_disableClearsProxyBlur();
    void test_enableBlurBehindWithoutXConnection();
    void test_enableBlurBehindAtomReplyFailure();
    void test_enableBlurBehindWritesAndDeletesXProperties();
};

void windowBlurHelper_test::test_initialState()
{
    WindowBlurHelper helper;

    QCOMPARE(helper.enable(), false);
    QCOMPARE(helper.radius(), 0.0);
    QVERIFY(helper.m_region.isEmpty());
    QCOMPARE(helper.m_windowProxy, nullptr);
}

void windowBlurHelper_test::test_updateBlurArea_withoutProxy()
{
    WindowBlurHelper helper;
    QTest::ignoreMessage(QtWarningMsg, "WindowBlurHelper::updateBlurArea: windowProxy in not init.");

    QCOMPARE(helper.updateBlurArea(), false);
}

void windowBlurHelper_test::test_enableWithoutRegionDisablesProxyBlur()
{
    QWindow window;
    UkuiQuick::WindowProxy proxy(&window);
    WindowBlurHelper helper;
    helper.setWindowProxy(&proxy);

    helper.setEnable(true);

    QCOMPARE(helper.enable(), true);
    QCOMPARE(proxy.blurEnabled(), false);
    QCOMPARE(proxy.blurOpacity(), 0.0);
    QCOMPARE(proxy.blurCallCount(), 1);

    helper.setEnable(true);
    QCOMPARE(proxy.blurCallCount(), 1);
}

void windowBlurHelper_test::test_setRegionAndEnableUpdatesProxy()
{
    QWindow window;
    UkuiQuick::WindowProxy proxy(&window);
    WindowBlurHelper helper;
    helper.setWindowProxy(&proxy);
    helper.setRadius(0.5);
    helper.setEnable(true);

    helper.setRegion(0, 0, 80, 40, 8);

    QCOMPARE(proxy.blurEnabled(), true);
    QCOMPARE(proxy.blurOpacity(), 0.5);
    QVERIFY(!proxy.blurRegion().isEmpty());
    QVERIFY(proxy.blurRegion().boundingRect().width() <= 80);
    QVERIFY(proxy.blurRegion().boundingRect().height() <= 40);
    QCOMPARE(helper.updateBlurArea(), true);
}

void windowBlurHelper_test::test_radiusClampSignalsAndOpacity()
{
    QWindow window;
    UkuiQuick::WindowProxy proxy(&window);
    WindowBlurHelper helper;
    helper.setWindowProxy(&proxy);
    helper.setEnable(true);
    helper.setRegion(0, 0, 20, 20, 4);
    QSignalSpy spy(&helper, &WindowBlurHelper::radiusChanged);

    helper.setRadius(2.0);

    QCOMPARE(helper.radius(), 1.0);
    QCOMPARE(proxy.blurOpacity(), 1.0);
    QCOMPARE(spy.count(), 1);

    const int blurCalls = proxy.blurCallCount();
    helper.setRadius(1.0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(proxy.blurCallCount(), blurCalls);

    helper.setRadius(0.25);
    QCOMPARE(helper.radius(), 0.25);
    QCOMPARE(proxy.blurOpacity(), 0.25);
    QCOMPARE(spy.count(), 2);
}

void windowBlurHelper_test::test_disableClearsProxyBlur()
{
    QWindow window;
    UkuiQuick::WindowProxy proxy(&window);
    WindowBlurHelper helper;
    helper.setWindowProxy(&proxy);
    helper.setEnable(true);
    helper.setRegion(0, 0, 30, 30, 6);
    QVERIFY(proxy.blurEnabled());

    helper.setEnable(false);

    QCOMPARE(helper.enable(), false);
    QCOMPARE(proxy.blurEnabled(), false);
    QCOMPARE(proxy.blurOpacity(), 0.0);

    helper.setRegion(0, 0, 40, 40, 6);
    QCOMPARE(proxy.blurEnabled(), false);
}

void windowBlurHelper_test::test_enableBlurBehindWithoutXConnection()
{
    WindowBlurHelper helper;
    resetXcbStubState();

    helper.enableBlurBehind(0, true, QRegion(0, 0, 10, 10), 4);
    helper.enableBlurBehind(0, false, QRegion(0, 0, 10, 10), 4);

    QCOMPARE(xcbStubState().internCalls, 0);
    QCOMPARE(xcbStubState().changeCalls, 0);
    QCOMPARE(xcbStubState().deleteCalls, 0);
}

void windowBlurHelper_test::test_enableBlurBehindAtomReplyFailure()
{
    WindowBlurHelper helper;
    resetXcbStubState();
    xcbStubState().connectionAvailable = true;
    xcbStubState().atomReplyAvailable = false;

    helper.enableBlurBehind(11, true, QRegion(0, 0, 10, 10), 4);

    QCOMPARE(xcbStubState().internCalls, 1);
    QCOMPARE(xcbStubState().changeCalls, 0);
    QCOMPARE(xcbStubState().deleteCalls, 0);
}

void windowBlurHelper_test::test_enableBlurBehindWritesAndDeletesXProperties()
{
    WindowBlurHelper helper;
    resetXcbStubState();
    xcbStubState().connectionAvailable = true;
    xcbStubState().atomReplyAvailable = true;

    helper.enableBlurBehind(11, true, QRegion(QRect(0, 0, 10, 10)).united(QRegion(20, 20, 5, 5)), 7);

    QCOMPARE(xcbStubState().internCalls, 1);
    QCOMPARE(xcbStubState().changeCalls, 1);
    QCOMPARE(xcbStubState().deleteCalls, 0);
    QCOMPARE(xcbStubState().lastWindow, uint32_t(11));
    QCOMPARE(xcbStubState().lastAtom, uint32_t(42));
    QCOMPARE(xcbStubState().lastFormat, uint32_t(32));
    QVERIFY(xcbStubState().lastDataLength >= 5);
    QCOMPARE(xcbStubState().lastRadius, uint32_t(7));

    helper.enableBlurBehind(12, false, QRegion(0, 0, 10, 10), 9);

    QCOMPARE(xcbStubState().deleteCalls, 1);
    QCOMPARE(xcbStubState().flushCalls, 1);
    QCOMPARE(xcbStubState().lastWindow, uint32_t(12));
}

QTEST_MAIN(windowBlurHelper_test)

#include "window-blur-helper-test.moc"
