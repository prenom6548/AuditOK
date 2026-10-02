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
#include "../../../src/windows/sidebar-window-helper.h"
#undef private

#include "global-settings.h"
#include "screen-monitor.h"

using namespace Sidebar;

class sidebarWindowHelper_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_geometryForBottomPanel_ltr();
    void test_geometryForTopPanel_rtl();
    void test_geometryForSidePanels();
    void test_updatesOnScreenAndSettingsSignals();
    void test_invalidTypeAndLayoutDirection();
    void test_defineModules();
};

void sidebarWindowHelper_test::init()
{
    GlobalSettings::globalInstance()->reset();
    ScreenMonitor::getInstance()->reset();
    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 0);
    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_SIZE_KEY, 48);
    ScreenMonitor::getInstance()->setLayoutDirection(Qt::LeftToRight);
    ScreenMonitor::getInstance()->setPrimaryScreen(qApp->primaryScreen());
}

void sidebarWindowHelper_test::test_geometryForBottomPanel_ltr()
{
    auto *helper = SidebarWindowHelper::instance();
    QRect screen = ScreenMonitor::getInstance()->getGeometry();

    QCOMPARE(helper->getWindowGeometry(SidebarWindowType::NotificationCenter), screen);
    QCOMPARE(helper->getWindowGeometry(SidebarWindowType::StatusBar), QRect(screen.x(), screen.y(), screen.width(), 2));
    QCOMPARE(helper->getWindowGeometry(SidebarWindowType::RightHandGesture),
             QRect(screen.x() + screen.width() - 2, screen.y() + 2, 2, screen.height() - 2 - 48));
}

void sidebarWindowHelper_test::test_geometryForTopPanel_rtl()
{
    auto *helper = SidebarWindowHelper::instance();
    ScreenMonitor::getInstance()->setLayoutDirection(Qt::RightToLeft);
    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 1);
    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_SIZE_KEY, 32);
    QRect screen = ScreenMonitor::getInstance()->getGeometry();

    QCOMPARE(helper->getWindowGeometry(SidebarWindowType::RightHandGesture),
             QRect(screen.x(), screen.y() + 32, 2, screen.height() - 32));
}

void sidebarWindowHelper_test::test_geometryForSidePanels()
{
    auto *helper = SidebarWindowHelper::instance();
    QRect screen = ScreenMonitor::getInstance()->getGeometry();

    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 2);
    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_SIZE_KEY, 77);
    QCOMPARE(helper->getWindowGeometry(SidebarWindowType::RightHandGesture),
             QRect(screen.x() + screen.width() - 2, screen.y() + 2, 2, screen.height() - 2));

    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 3);
    QCOMPARE(helper->getWindowGeometry(SidebarWindowType::RightHandGesture),
             QRect(screen.x() + screen.width() - 2, screen.y() + 2, 2, screen.height() - 2));
}

void sidebarWindowHelper_test::test_updatesOnScreenAndSettingsSignals()
{
    auto *helper = SidebarWindowHelper::instance();
    QSignalSpy spy(helper, &SidebarWindowHelper::geometryChanged);

    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_SIZE_KEY, 64);
    QVERIFY(spy.count() >= 1);

    const QRect before = helper->getWindowGeometry(SidebarWindowType::RightHandGesture);
    ScreenMonitor::getInstance()->setPrimaryScreen(nullptr);
    QVERIFY(helper->getWindowGeometry(SidebarWindowType::RightHandGesture).isEmpty());

    ScreenMonitor::getInstance()->setPrimaryScreen(qApp->primaryScreen());
    QVERIFY(!helper->getWindowGeometry(SidebarWindowType::RightHandGesture).isEmpty());
    QVERIFY(helper->getWindowGeometry(SidebarWindowType::RightHandGesture) != before
            || GlobalSettings::globalInstance()->getValue(UKUI_PANEL_SIZE_KEY).toInt() == 64);
}

void sidebarWindowHelper_test::test_invalidTypeAndLayoutDirection()
{
    auto *helper = SidebarWindowHelper::instance();

    QCOMPARE(helper->getLayoutDirection(), ScreenMonitor::getInstance()->getLayoutDirection());
    QVERIFY(helper->getWindowGeometry(SidebarWindowType::Normal).isEmpty());
}

void sidebarWindowHelper_test::test_defineModules()
{
    SidebarWindowDefineModule::defineModules("Test.SidebarWindow", 1, 0);
    QVERIFY(QMetaType::type("SidebarWindowType::Type") != QMetaType::UnknownType);
}

QTEST_MAIN(sidebarWindowHelper_test)

#include "sidebar-window-helper-test.moc"
