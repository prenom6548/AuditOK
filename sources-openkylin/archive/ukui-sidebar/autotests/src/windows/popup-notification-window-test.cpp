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
#include <QQuickItem>
#include <QMetaProperty>

#define private public
#include "../../../src/windows/popup-notification-window.h"
#undef private

#include "global-settings.h"
#include "notification-model.h"
#include "screen-monitor.h"
#include "shared-engine-view.h"
#include "window-helper.h"

using namespace UkuiNotification;

class PopupRootItem : public QQuickItem
{
    Q_OBJECT
};

class popupNotificationWindow_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_initialStateAndScreenLock();
    void test_screenLockStatePropertyUsesRenamedNotifySignal();
    void test_loadQML_updatesViewWidthFromRoot();
    void test_updateGeometry_panelTop();
    void test_updateGeometry_panelLeft_mirroredAndDisplaced();
    void test_updateGeometry_panelTypeIslandOffset();
    void test_updateGeometry_ignoresInvalidWidthOrScreen();
    void test_updataWindowRegion_buildsRegion();
    void test_updataGroupsPosition_translatesRegion();
    void test_enableWindowBlur_updatesProxy();
    void test_updatePopupWindowGeometry_changesState();
    void test_onPrimaryScreenChanged_updatesScreen();
    void test_onRootWidthChanged_updatesGeometry();
    void test_modelVisibilityFollowsRowChanges();
};

void popupNotificationWindow_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->reset();
    Sidebar::ScreenMonitor::getInstance()->reset();
    NotificationModel::instance()->reset();

    Sidebar::ScreenMonitor::getInstance()->setPrimaryScreen(qApp->primaryScreen());
    Sidebar::ScreenMonitor::getInstance()->setPanelPosition(0);
    Sidebar::ScreenMonitor::getInstance()->setPanelSize(40);
    Sidebar::ScreenMonitor::getInstance()->setLayoutDirection(Qt::LeftToRight);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_PANEL_TYPE_KEY, 0);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_DATA_ISLAND_POSITION_KEY, 0);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_SETTINGS_ISLAND_POSITION_KEY, 0);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_TOPBAR_SIZE_KEY, 32);
}

void popupNotificationWindow_test::test_initialStateAndScreenLock()
{
    Sidebar::ScreenMonitor::getInstance()->setScreenLockState(true);

    PopupNotificationWindow window;

    QCOMPARE(window.screenLockState(), true);
    QCOMPARE(window.m_windowProxy->windowType(), UkuiQuick::WindowType::ScreenLockNotification);
}

void popupNotificationWindow_test::test_screenLockStatePropertyUsesRenamedNotifySignal()
{
    PopupNotificationWindow window;
    const QMetaObject *metaObject = window.metaObject();
    int propertyIndex = metaObject->indexOfProperty("screenLockState");
    QVERIFY(propertyIndex >= 0);

    QMetaProperty property = metaObject->property(propertyIndex);
    QVERIFY(property.hasNotifySignal());
    QCOMPARE(property.notifySignal().methodSignature(), QByteArray("screenLockStateChanged()"));
    QVERIFY(metaObject->indexOfSignal("screenLockStatehChanged()") < 0);
}

void popupNotificationWindow_test::test_loadQML_updatesViewWidthFromRoot()
{
    PopupNotificationWindow window;
    auto *root = new PopupRootItem;
    root->setWidth(512);
    window.setTestRootObject(root);

    window.loadQML();

    QCOMPARE(window.viewWidth(), 512);
}

void popupNotificationWindow_test::test_updateGeometry_panelTop()
{
    PopupNotificationWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.m_viewWidth = 200;
    Sidebar::ScreenMonitor::getInstance()->setPanelPosition(1);
    Sidebar::ScreenMonitor::getInstance()->setPanelSize(40);

    window.updateGeometry();

    QCOMPARE(window.m_notificationPoint.y(), screen->geometry().y() + 40);
    QCOMPARE(window.m_windowProxy->position(), window.m_notificationPoint);
}

void popupNotificationWindow_test::test_updateGeometry_panelLeft_mirroredAndDisplaced()
{
    PopupNotificationWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.m_viewWidth = 200;
    window.m_needDisplace = true;
    window.m_sidebarWidth = 50;
    Sidebar::ScreenMonitor::getInstance()->setPanelPosition(2);
    Sidebar::ScreenMonitor::getInstance()->setPanelSize(30);
    Sidebar::ScreenMonitor::getInstance()->setLayoutDirection(Qt::RightToLeft);

    window.updateGeometry();

    QCOMPARE(window.m_notificationPoint.x(), screen->geometry().x() + 80);
}

void popupNotificationWindow_test::test_updateGeometry_panelTypeIslandOffset()
{
    PopupNotificationWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.m_viewWidth = 200;
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_PANEL_TYPE_KEY, 1);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_DATA_ISLAND_POSITION_KEY, 1);
    Sidebar::GlobalSettings::globalInstance()->setValue(UKUI_TOPBAR_SIZE_KEY, 28);

    window.updateGeometry();

    QCOMPARE(window.m_notificationPoint.y(), screen->geometry().y() + 28);
}

void popupNotificationWindow_test::test_updateGeometry_ignoresInvalidWidthOrScreen()
{
    PopupNotificationWindow window;
    window.m_viewWidth = 0;
    QPoint before = window.m_notificationPoint;

    window.updateGeometry();
    QCOMPARE(window.m_notificationPoint, before);

    window.m_viewWidth = 100;
    window.m_screen = nullptr;
    window.updateGeometry();
    QCOMPARE(window.m_notificationPoint, before);
}

void popupNotificationWindow_test::test_updataWindowRegion_buildsRegion()
{
    PopupNotificationWindow window;
    QVariantMap group1;
    group1.insert("count", 1);
    group1.insert("height", 50);
    group1.insert("width", 100);
    group1.insert("radius", 8);
    QVariantMap group2;
    group2.insert("count", 3);
    group2.insert("height", 60);
    group2.insert("width", 120);
    group2.insert("radius", 10);
    QVariantMap input;
    input.insert("regions", QVariantList{group1, group2});

    window.updataWindowRegion(input, 12);

    QVERIFY(!window.m_windowRegion.isEmpty());
    QCOMPARE(window.m_windowProxy->blurCallCount(), 0);
}

void popupNotificationWindow_test::test_updataGroupsPosition_translatesRegion()
{
    PopupNotificationWindow window;
    window.m_windowRegion = QRegion(0, 0, 20, 20);

    window.updataGroupsPosition(5);

    QCOMPARE(window.m_windowRegion.boundingRect().y(), -5);
}

void popupNotificationWindow_test::test_enableWindowBlur_updatesProxy()
{
    PopupNotificationWindow window;
    window.m_windowRegion = QRegion(0, 0, 20, 20);

    window.enableWindowBlur(true);
    QCOMPARE(window.m_windowProxy->blurEnabled(), true);

    const int count = window.m_windowProxy->blurCallCount();
    window.enableWindowBlur(true);
    QCOMPARE(window.m_windowProxy->blurCallCount(), count);
}

void popupNotificationWindow_test::test_updatePopupWindowGeometry_changesState()
{
    PopupNotificationWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    window.m_viewWidth = 200;

    window.updatePopupWindowGeometry(true, 88);

    QCOMPARE(window.m_needDisplace, true);
    QCOMPARE(window.m_sidebarWidth, 88);
}

void popupNotificationWindow_test::test_onPrimaryScreenChanged_updatesScreen()
{
    PopupNotificationWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);

    window.onPrimaryScreenChanged(screen);

    QCOMPARE(window.m_screen, screen);
}

void popupNotificationWindow_test::test_onRootWidthChanged_updatesGeometry()
{
    PopupNotificationWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    window.m_screen = screen;
    auto *root = new PopupRootItem;
    root->setWidth(600);
    window.setTestRootObject(root);
    window.m_viewWidth = 300;

    window.onRootWidthChanged();

    QCOMPARE(window.m_viewWidth, 600);
}

void popupNotificationWindow_test::test_modelVisibilityFollowsRowChanges()
{
    PopupNotificationWindow window;
    QList<TestNotificationData> items;
    items.append({1, "mail", false, false, true, "summary", "body"});

    NotificationModel::instance()->setItems(items);
    QCoreApplication::processEvents();
    QCOMPARE(window.rootContext()->contextProperty("groupModel").isValid(), true);
    QCOMPARE(window.rootContext()->contextProperty("sourceModel").isValid(), true);

    NotificationModel::instance()->setItems({});
    QCoreApplication::processEvents();
    QCOMPARE(window.rootContext()->contextProperty("popupNotificationWindow").value<QObject*>(), static_cast<QObject*>(&window));
}

QTEST_MAIN(popupNotificationWindow_test)

#include "popup-notification-window-test.moc"
