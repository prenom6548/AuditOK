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
#include <QStandardItemModel>

#define private public
#define protected public
#include "../../../src/windows/tablet-popup-view.h"
#undef protected
#undef private

#include "global-settings.h"
#include "notification-model.h"
#include "shared-engine-view.h"
#include "window-helper.h"

using namespace UkuiNotification;

class tabletPopupView_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_initialGeometryAndContext();
    void test_onPrimaryScreenChanged_ignoresNull();
    void test_updateHeight_clampsAndSkipsSameHeight();
    void test_enableWindowBlur();
    void test_eventBranchesAndInit();
    void test_emptyAndStoredNotificationsKeepHidden();
    void test_visibilityFollowsNonStoredNotification();
    void test_visibilityUpdatesWhenNotificationInsertedAfterConstruction();
    void test_tabletModeChangeInvalidatesFilter();
    void test_filterModelSourceResetDisconnectsPreviousModel();
    void test_primaryScreenReconnectAndNoScreenMaxHeight();
};

void tabletPopupView_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->reset();
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);
    NotificationModel::instance()->reset();
}

void tabletPopupView_test::test_initialGeometryAndContext()
{
    TabletPopupView view;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);
    const QRect screenRect = screen->geometry();

    QCOMPARE(view.m_windowProxy->windowType(), UkuiQuick::WindowType::Notification);
    QCOMPARE(view.windowWidth(), static_cast<int>(screenRect.width() * 0.45));
    QCOMPARE(view.m_topLeft.x(), screenRect.x() + ((screenRect.width() - view.windowWidth()) / 2));
    QCOMPARE(view.m_topLeft.y(), screenRect.y() + view.m_windowMargin);
    QCOMPARE(view.m_windowProxy->position(), QPoint());
    view.updateHeight(20);
    view.updateGeometry();
    QCOMPARE(view.m_windowProxy->position(), view.m_topLeft);
    QCOMPARE(view.windowMaxHeight(), screenRect.height() - view.m_windowMargin * 2);
    QCOMPARE(view.enableAnimation(), true);
    QCOMPARE(view.rootContext()->contextProperty("tabletPopupView").value<QObject *>(), static_cast<QObject *>(&view));
    QVERIFY(view.rootContext()->contextProperty("groupModel").isValid());
    QVERIFY(view.rootContext()->contextProperty("sourceModel").isValid());
}

void tabletPopupView_test::test_onPrimaryScreenChanged_ignoresNull()
{
    TabletPopupView view;
    QScreen *before = view.m_screen;

    view.onPrimaryScreenChanged(nullptr);

    QCOMPARE(view.m_screen, before);
}

void tabletPopupView_test::test_updateHeight_clampsAndSkipsSameHeight()
{
    TabletPopupView view;

    view.updateHeight(0);
    QCOMPARE(view.height(), 1);

    view.updateHeight(view.windowMaxHeight() + 100);
    QCOMPARE(view.height(), view.windowMaxHeight());

    const int before = view.height();
    view.updateHeight(before);
    QCOMPARE(view.height(), before);
}

void tabletPopupView_test::test_enableWindowBlur()
{
    TabletPopupView view;
    view.resize(200, 80);

    view.enableWindowBlur(12, true);

    QCOMPARE(view.m_windowProxy->blurEnabled(), true);
    QVERIFY(!view.m_windowProxy->blurRegion().isEmpty());

    view.enableWindowBlur(12, false);
    QCOMPARE(view.m_windowProxy->blurEnabled(), false);
}

void tabletPopupView_test::test_eventBranchesAndInit()
{
    TabletPopupView view;

    view.init();

    QShowEvent show;
    QVERIFY(view.event(&show));

    QExposeEvent expose(QRegion(0, 0, 10, 10));
    QVERIFY(view.event(&expose));

    QEvent userEvent(QEvent::User);
    QVERIFY(view.event(&userEvent));
}

void tabletPopupView_test::test_emptyAndStoredNotificationsKeepHidden()
{
    TabletPopupView emptyView;
    QCOMPARE(emptyView.isVisible(), false);

    QList<TestNotificationData> items;
    items.append({1, "chat", false, true, true, "summary", "body"});
    NotificationModel::instance()->setItems(items);

    TabletPopupView storedView;
    QCOMPARE(storedView.isVisible(), false);
}

void tabletPopupView_test::test_visibilityFollowsNonStoredNotification()
{
    QList<TestNotificationData> items;
    items.append({1, "chat", false, false, true, "summary", "body"});
    NotificationModel::instance()->setItems(items);

    TabletPopupView view;

    QCOMPARE(view.isVisible(), true);
}

void tabletPopupView_test::test_visibilityUpdatesWhenNotificationInsertedAfterConstruction()
{
    TabletPopupView view;
    QCOMPARE(view.isVisible(), false);

    QList<TestNotificationData> items;
    items.append({1, "chat", false, false, true, "summary", "body"});
    NotificationModel::instance()->setItems(items);
    QCoreApplication::processEvents();

    QCOMPARE(view.isVisible(), true);
}

void tabletPopupView_test::test_tabletModeChangeInvalidatesFilter()
{
    QList<TestNotificationData> items;
    items.append({1, "chat", false, false, true, "summary", "body"});
    NotificationModel::instance()->setItems(items);

    TabletPopupView view;
    QCOMPARE(view.isVisible(), true);

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);
    QCoreApplication::processEvents();

    auto *group = view.rootContext()->contextProperty("groupModel").value<QObject *>();
    QVERIFY(group != nullptr);
    QCOMPARE(qobject_cast<QAbstractItemModel *>(group)->rowCount(), 0);
}

void tabletPopupView_test::test_filterModelSourceResetDisconnectsPreviousModel()
{
    TabletPopupView view;
    auto *group = qobject_cast<QAbstractProxyModel *>(view.rootContext()->contextProperty("groupModel").value<QObject *>());
    QVERIFY(group != nullptr);
    auto *filter = qobject_cast<QAbstractProxyModel *>(group->sourceModel());
    QVERIFY(filter != nullptr);

    QStandardItemModel firstModel;
    firstModel.appendRow(new QStandardItem("first"));
    QStandardItemModel secondModel;
    secondModel.appendRow(new QStandardItem("second"));

    filter->setSourceModel(&firstModel);
    filter->setSourceModel(&secondModel);

    QCOMPARE(filter->rowCount(), 1);
}

void tabletPopupView_test::test_primaryScreenReconnectAndNoScreenMaxHeight()
{
    TabletPopupView view;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);

    view.onPrimaryScreenChanged(screen);
    QCOMPARE(view.m_screen, screen);

    view.m_screen = nullptr;
    QCOMPARE(view.windowMaxHeight(), 1);
}

QTEST_MAIN(tabletPopupView_test)

#include "tablet-popup-view-test.moc"
