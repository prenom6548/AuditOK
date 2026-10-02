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

#define private public
#define protected public
#include "../../../src/windows/sidebar-view.h"
#undef protected
#undef private

#include "QX11Info"
#include "global-settings.h"
#include "notification-model.h"
#include "screen-monitor.h"
#include "shared-engine-view.h"
#include "sidebar-window-helper.h"
#include "window-helper.h"

using namespace Sidebar;

class SidebarRootItem : public QQuickItem
{
    Q_OBJECT
public:
    Q_INVOKABLE QVariant isRunning(const QVariant &active)
    {
        lastRunningArg = active.toBool();
        return running;
    }

    Q_INVOKABLE void updateProperty(const QVariant &active)
    {
        lastUpdateActive = active.toBool();
        ++updateCalls;
    }

    bool running = false;
    bool lastRunningArg = false;
    bool lastUpdateActive = false;
    int updateCalls = 0;
};

class sidebarView_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_initialGeometryAndProperties();
    void test_propertySettersAndSignals();
    void test_updateGeometry_panelBranches();
    void test_windowExpandPaddingAndBlur();
    void test_activeWindow_requiresRootAndRespectsRunning();
    void test_windowVisibleAndContentState();
    void test_tabletModeChangeUpdatesState();
    void test_eventAndExposeBranches();
    void test_notificationEmptySignals();
};

void sidebarView_test::init()
{
    GlobalSettings::globalInstance()->reset();
    ScreenMonitor::getInstance()->reset();
    SidebarWindowHelper::instance()->reset();
    UkuiQuick::WindowProxy::setCurrentScreen(qApp->primaryScreen());
    UkuiNotification::NotificationModel::instance()->reset();
    QX11Info::setPlatformX11(false);

    auto *settings = GlobalSettings::globalInstance();
    settings->setValue(TABLET_MODE, false);
    settings->setValue(UKUI_PANEL_POSITION_KEY, 0);
    settings->setValue(UKUI_PANEL_SIZE_KEY, 48);
    settings->setValue(UKUI_PANEL_TYPE_KEY, 0);
    settings->setValue(UKUI_DATA_ISLAND_POSITION_KEY, 0);
    settings->setValue(UKUI_SETTINGS_ISLAND_POSITION_KEY, 0);
    settings->setValue(UKUI_TOPBAR_SIZE_KEY, 24);
    SidebarWindowHelper::instance()->setLayoutDirection(Qt::LeftToRight);
}

void sidebarView_test::test_initialGeometryAndProperties()
{
    SidebarView view;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);

    QCOMPARE(view.isTabletMode(), false);
    QCOMPARE(view.contentVisible(), false);
    QCOMPARE(view.isNotificationEmpty(), true);
    QCOMPARE(view.isWindowFold(), true);
    QCOMPARE(view.windowPadding(), 8);
    QCOMPARE(view.minWindowSize(), 144);
    QCOMPARE(view.minimumThreshold(), static_cast<qint32>(screen->geometry().width() * 0.05));
    QCOMPARE(view.primaryScreenRight(), screen->geometry().right());
    QCOMPARE(view.m_windowProxy->position(), view.geometry().topLeft());
}

void sidebarView_test::test_propertySettersAndSignals()
{
    SidebarView view;
    QSignalSpy contentSpy(&view, &SidebarView::contentVisibleChanged);
    QSignalSpy emptySpy(&view, &SidebarView::isNotificationEmptyChanged);
    QSignalSpy foldSpy(&view, &SidebarView::isWindowFoldChanged);
    QSignalSpy radiusSpy(&view, &SidebarView::radiusChanged);

    view.setContentVisible(true);
    view.setIsNotificationEmpty(false);
    view.setIsWindowFold(false);
    view.setRadius(20);

    QCOMPARE(view.contentVisible(), true);
    QCOMPARE(view.isNotificationEmpty(), false);
    QCOMPARE(view.isWindowFold(), false);
    QCOMPARE(view.m_radius, 20);
    QCOMPARE(contentSpy.count(), 1);
    QCOMPARE(emptySpy.count(), 1);
    QCOMPARE(foldSpy.count(), 1);
    QCOMPARE(radiusSpy.count(), 1);

    view.setIsNotificationEmpty(false);
    view.setIsWindowFold(false);
    QCOMPARE(emptySpy.count(), 1);
    QCOMPARE(foldSpy.count(), 1);
}

void sidebarView_test::test_updateGeometry_panelBranches()
{
    SidebarView view;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    const QRect rect = screen->geometry();
    const int width = 384 + view.m_padding * 2;

    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 1);
    QCOMPARE(view.geometry(), rect.adjusted(rect.width() - width, 48, 0, 0));

    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 2);
    QCOMPARE(view.geometry(), rect.adjusted(rect.width() - width, 0, 0, 0));

    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_POSITION_KEY, 3);
    QCOMPARE(view.geometry(), rect.adjusted(rect.width() - width - 48, 0, -48, 0));

    SidebarWindowHelper::instance()->setLayoutDirection(Qt::RightToLeft);
    GlobalSettings::globalInstance()->setValue(UKUI_PANEL_TYPE_KEY, 1);
    GlobalSettings::globalInstance()->setValue(UKUI_DATA_ISLAND_POSITION_KEY, 1);
    QCOMPARE(view.geometry(), rect.adjusted(0, 24, -(rect.width() - width), -48));
}

void sidebarView_test::test_windowExpandPaddingAndBlur()
{
    SidebarView view;
    QSignalSpy spy(&view, &SidebarView::windowExpandPaddingChanged);

    view.setWindowExpandPadding(12);

    QCOMPARE(view.windowExpandPadding(), 12);
    QCOMPARE(view.contentWidth(), view.width() - 24);
    QCOMPARE(spy.count(), 1);

    view.setBlurHeight(120);
    QCOMPARE(view.m_blurHeight, 120);
    QVERIFY(!view.m_windowProxy->blurRegion().isEmpty());
}

void sidebarView_test::test_activeWindow_requiresRootAndRespectsRunning()
{
    SidebarView view;
    QSignalSpy showSpy(&view, &SidebarView::requestShowContent);
    QSignalSpy hideSpy(&view, &SidebarView::requestHideContent);

    view.activeWindow(true);
    QCOMPARE(showSpy.count(), 0);

    auto *root = new SidebarRootItem;
    view.setTestRootObject(root);
    root->running = true;
    view.activeWindow(true);
    QCOMPARE(root->updateCalls, 0);

    root->running = false;
    view.activeWindow(true);
    QCOMPARE(root->updateCalls, 1);
    QCOMPARE(root->lastUpdateActive, true);
    QCOMPARE(showSpy.count(), 1);

    view.activeWindow(false);
    QCOMPARE(root->updateCalls, 2);
    QCOMPARE(hideSpy.count(), 1);
}

void sidebarView_test::test_windowVisibleAndContentState()
{
    SidebarView view;
    view.setContentVisible(true);

    view.setWindowVisible(false);

    QCOMPARE(view.isVisible(), false);
    QCOMPARE(view.contentVisible(), false);
}

void sidebarView_test::test_tabletModeChangeUpdatesState()
{
    SidebarView view;
    QSignalSpy spy(&view, &SidebarView::isTabletModeChanged);

    GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    QCOMPARE(view.isTabletMode(), true);
    QCOMPARE(view.isVisible(), false);
    QCOMPARE(spy.count(), 1);
}

void sidebarView_test::test_eventAndExposeBranches()
{
    SidebarView view;
    QMoveEvent move(QPoint(2, 2), QPoint(1, 1));
    QVERIFY(view.event(&move));

    QExposeEvent expose(QRegion(0, 0, 10, 10));
    QX11Info::setPlatformX11(false);
    view.exposeEvent(&expose);

    QX11Info::setPlatformX11(true);
    view.exposeEvent(&expose);
}

void sidebarView_test::test_notificationEmptySignals()
{
    SidebarView view;
    QSignalSpy spy(&view, &SidebarView::isNotificationEmptyChanged);

    QList<UkuiNotification::TestNotificationData> items;
    items.append({1, "mail", false, true, true, "summary", "body"});
    UkuiNotification::NotificationModel::instance()->setItems(items);
    QCoreApplication::processEvents();

    QCOMPARE(view.isNotificationEmpty(), false);
    QCOMPARE(spy.count(), 1);

    UkuiNotification::NotificationModel::instance()->setItems({});
    QCoreApplication::processEvents();
    QCOMPARE(view.isNotificationEmpty(), true);
}

QTEST_MAIN(sidebarView_test)

#include "sidebar-view-test.moc"
