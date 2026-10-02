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
#include "../../src/sidebar-main.h"
#undef private

#include "stubs/sidebar-main-deps.h"

static SidebarMain *createMainForTest()
{
    return new SidebarMain;
}

class sidebarMain_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_requestWidgetActive_withoutShortcutWindow();
    void test_requestWidgetActive_switchesByCurrentId();
    void test_loadTabletWindows_createAndDestroy();
    void test_sidebarStateChanged_payload();
    void test_updateSystrayIcon_variants();
    void test_requestSidebar_dispatches();
    void test_requestNotificationCenter_dispatches();
    void test_requestShortcut_dispatches();
    void test_parseMessage_paths();
    void test_initPublicObjects();
    void test_initTabletPopupWindow();
    void test_initPopupWindow_updatesGeometry();
};

void sidebarMain_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->reset();
    UkuiNotification::NotificationModel::instance()->reset();
}

void sidebarMain_test::test_requestWidgetActive_withoutShortcutWindow()
{
    SidebarMain *main = createMainForTest();
    main->m_shortcutsWindow = nullptr;

    main->requestWidgetActive("wifi", true);

    QVERIFY(true);
}

void sidebarMain_test::test_requestWidgetActive_switchesByCurrentId()
{
    SidebarMain *main = createMainForTest();
    auto *window = new ShortcutsWindow;
    window->setCurrentId("wifi");
    window->setRequestMenuWidgetResult(true);
    main->m_shortcutsWindow = window;

    main->requestWidgetActive("wifi", true);
    QCOMPARE(window->requestedWidgetId(), QString("wifi"));
    QCOMPARE(window->lastActive(), true);

    window->setVisible(false);
    window->setCurrentId("network");
    main->requestWidgetActive("wifi", false);
    QCOMPARE(window->requestedWidgetId(), QString("wifi"));
    QCOMPARE(window->lastActive(), true);

    window->setRequestMenuWidgetResult(false);
    window->setVisible(false);
    main->requestWidgetActive("bluetooth", false);
    QCOMPARE(window->requestedWidgetId(), QString("bluetooth"));
    QCOMPARE(window->lastActive(), true);
}

void sidebarMain_test::test_loadTabletWindows_createAndDestroy()
{
    SidebarMain *main = createMainForTest();
    main->m_notificationTablet = nullptr;
    main->m_popupTablet = nullptr;

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);
    main->loadTabletWindows();
    QVERIFY(main->m_notificationTablet != nullptr);
    QVERIFY(main->m_popupTablet != nullptr);

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);
    main->loadTabletWindows();
    QCOMPARE(main->m_notificationTablet, nullptr);
    QCOMPARE(main->m_popupTablet, nullptr);
}

void sidebarMain_test::test_sidebarStateChanged_payload()
{
    SidebarMain *main = createMainForTest();
    QSignalSpy spy(main, &SidebarMain::stateChange);

    QList<UkuiNotification::TestNotificationData> items;
    items.append({1, "mail", false, false, true, "summary", "body"});
    items.append({2, "chat", false, false, true, "summary", "body"});
    UkuiNotification::NotificationModel::instance()->setItems(items);

    main->sidebarStateChanged(true);
    main->sidebarStateChanged(false);

    QVERIFY(spy.count() >= 2);
    QCOMPARE(spy.at(spy.count() - 2).at(0).toString(), QString("sidebar"));
    QCOMPARE(spy.at(spy.count() - 2).at(1).toMap().value("unreadMsg").toInt(), 0);
    QCOMPARE(spy.at(spy.count() - 1).at(1).toMap().value("unreadMsg").toInt(), 2);
}

void sidebarMain_test::test_updateSystrayIcon_variants()
{
    SidebarMain *main = createMainForTest();
    main->m_systemTray = new QSystemTrayIcon(main);

    main->updateSystrayIcon(true);
    QCOMPARE(main->m_systemTray->toolTip(), QString("NotificationCenter"));

    QList<UkuiNotification::TestNotificationData> items;
    items.append({1, "mail", false, true, true, "summary", "body"});
    UkuiNotification::NotificationModel::instance()->setItems(items);
    main->updateSystrayIcon(false);
    QVERIFY(main->m_systemTray->toolTip().contains("1 Notifications"));

    main->m_systemTray = nullptr;
    main->updateSystrayIcon(true);
}

void sidebarMain_test::test_requestSidebar_dispatches()
{
    SidebarMain *main = createMainForTest();
    auto *view = new Sidebar::SidebarView;
    main->m_notificationPC = view;

    main->requestSidebar(SidebarMain::Active);
    QCOMPARE(view->isVisible(), true);

    main->requestSidebar(SidebarMain::Show);
    QCOMPARE(view->lastActive(), true);

    main->requestSidebar(SidebarMain::Hide);
    QCOMPARE(view->lastActive(), false);

    main->requestSidebar(SidebarMain::DirectlyHide);
    QCOMPARE(view->lastWindowVisible(), false);

    main->m_notificationPC = nullptr;
    main->requestSidebar(SidebarMain::Show);
    QVERIFY(true);
}

void sidebarMain_test::test_requestNotificationCenter_dispatches()
{
    SidebarMain *main = createMainForTest();
    auto *view = new UkuiNotification::NotificationCenterWindow;
    main->m_notificationTablet = view;

    main->requestNotificationCenter(SidebarMain::Active);
    QCOMPARE(view->isVisible(), true);

    main->requestNotificationCenter(SidebarMain::Show);
    QCOMPARE(view->lastActive(), true);

    main->requestNotificationCenter(SidebarMain::Hide);
    QCOMPARE(view->lastActive(), false);

    main->m_notificationTablet = nullptr;
    main->requestNotificationCenter(SidebarMain::Show);
    QVERIFY(true);
}

void sidebarMain_test::test_requestShortcut_dispatches()
{
    SidebarMain *main = createMainForTest();
    auto *window = new ShortcutsWindow;
    main->m_shortcutsWindow = window;

    main->requestShortcut(SidebarMain::Active);
    QCOMPARE(window->isVisible(), true);

    main->requestShortcut(SidebarMain::Show);
    QCOMPARE(window->lastActive(), true);

    main->requestShortcut(SidebarMain::Hide);
    QCOMPARE(window->lastActive(), false);

    main->m_shortcutsWindow = nullptr;
    main->requestShortcut(SidebarMain::Show);
    QVERIFY(true);
}

void sidebarMain_test::test_parseMessage_paths()
{
    SidebarMain *main = createMainForTest();
    auto *shortcut = new ShortcutsWindow;
    auto *notice = new UkuiNotification::NotificationCenterWindow;
    main->m_shortcutsWindow = shortcut;
    main->m_notificationTablet = notice;

    main->parseMessage("show control");
    QCOMPARE(shortcut->isVisible(), true);

    main->parseMessage("show notify");
    QCOMPARE(notice->isVisible(), true);

    main->parseMessage("show");
    QVERIFY(true);

    main->parseMessage("noop");
    QVERIFY(true);
}

void sidebarMain_test::test_initPublicObjects()
{
    SidebarMain *main = createMainForTest();

    main->initPublicObjects();

    QVERIFY(Sidebar::GlobalSettings::globalInstance() != nullptr);
    QVERIFY(Sidebar::ScreenMonitor::getInstance() != nullptr);
    QVERIFY(Sidebar::HandGestureHelper::getInstance() != nullptr);
    QVERIFY(Sidebar::SidebarWindowHelper::instance() != nullptr);
}

void sidebarMain_test::test_initTabletPopupWindow()
{
    SidebarMain *main = createMainForTest();
    main->m_popupTablet = nullptr;

    main->initTabletPopupWindow();

    QVERIFY(main->m_popupTablet != nullptr);
}

void sidebarMain_test::test_initPopupWindow_updatesGeometry()
{
    SidebarMain *main = createMainForTest();

    auto *view = new Sidebar::SidebarView;
    view->setContentWidth(320);
    main->m_notificationPC = view;

    main->initPopupWindow();
    auto *popup = main->m_popupPC;
    QVERIFY(popup != nullptr);
    QCOMPARE(popup->loadQmlCalls(), 1);

    view->setVisible(true);
    QCOMPARE(popup->lastNeedDisplace(), true);
    QCOMPARE(popup->lastSidebarWidth(), 320);

    view->setIsWindowFold(true);
    QCOMPARE(popup->lastNeedDisplace(), false);
}

QTEST_MAIN(sidebarMain_test)

#include "sidebar-main-test.moc"
