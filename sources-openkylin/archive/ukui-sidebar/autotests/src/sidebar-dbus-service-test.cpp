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

#include "sidebar-main.h"
#define private public
#include "../../src/sidebar-dbus-service.h"
#undef private

#include "sidebaradaptor.h"

class sidebarDbusService_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_parentAndAdaptor();
    void test_activeDispatchesKnownModules();
    void test_shortcutWidgetActive_ignoresEmptyId();
    void test_shortcutWidgetActive_dispatchesValidId();
    void test_stateSignalForwardedToAdaptor();
};

void sidebarDbusService_test::init()
{
    SidebarMain::resetRecords();
}

void sidebarDbusService_test::test_parentAndAdaptor()
{
    SidebarMain main;
    Sidebar::SidebarDbusService service(&main);

    QCOMPARE(service.parent(), &main);
    QVERIFY(service.m_adaptor != nullptr);
}

void sidebarDbusService_test::test_activeDispatchesKnownModules()
{
    SidebarMain main;
    Sidebar::SidebarDbusService service(&main);

    service.sidebarActive();
    service.shortcutsActive();
    service.active("notice");
    service.active("unknown");

    QStringList calls = SidebarMain::calls();
    QVERIFY(calls.contains("sidebar:0"));
    QVERIFY(calls.contains("shortcut:0"));
    QVERIFY(calls.contains("notice:0"));
    QCOMPARE(calls.filter("unknown").count(), 0);
}

void sidebarDbusService_test::test_shortcutWidgetActive_ignoresEmptyId()
{
    SidebarMain main;
    Sidebar::SidebarDbusService service(&main);

    service.shortcutWidgetActive("", true);

    QVERIFY(!SidebarMain::calls().contains("widget"));
}

void sidebarDbusService_test::test_shortcutWidgetActive_dispatchesValidId()
{
    SidebarMain main;
    Sidebar::SidebarDbusService service(&main);

    service.shortcutWidgetActive("network", true);

    QVERIFY(SidebarMain::calls().contains("widget"));
    QCOMPARE(SidebarMain::lastWidgetId(), QString("network"));
    QCOMPARE(SidebarMain::lastShowReturnButton(), true);
}

void sidebarDbusService_test::test_stateSignalForwardedToAdaptor()
{
    SidebarMain main;
    Sidebar::SidebarDbusService service(&main);
    QVariantMap data;
    data.insert("unreadMsg", 3);

    Q_EMIT main.stateChange("sidebar", data);

    auto *adaptor = qobject_cast<SidebarAdaptor *>(service.m_adaptor);
    QVERIFY(adaptor != nullptr);
    QCOMPARE(adaptor->lastModule(), QString("sidebar"));
    QCOMPARE(adaptor->lastData().value("unreadMsg").toInt(), 3);
}

QTEST_MAIN(sidebarDbusService_test)

#include "sidebar-dbus-service-test.moc"
