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
#include "../../../src/windows/notification-window.h"
#undef private

#include "QX11Info"
#include "event-track.h"
#include "global-settings.h"
#include "hand-gesture-helper.h"
#include "notification-model.h"
#include "shared-engine-view.h"
#include "window-helper.h"

using namespace UkuiNotification;

class TestRootItem : public QQuickItem
{
    Q_OBJECT
public:
    Q_INVOKABLE void updateContentY(const QVariant &value)
    {
        lastContentY = value.toInt();
        ++updateCalls;
    }

    Q_INVOKABLE void startAnimation(const QVariant &value)
    {
        lastAnimationValue = value.toBool();
        ++animationCalls;
    }

    int lastContentY = 0;
    bool lastAnimationValue = false;
    int updateCalls = 0;
    int animationCalls = 0;
};

class notificationWindow_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_globalGroupModel_filtersStoredNotifications();
    void test_setContentVisible_emitsSignal();
    void test_updateGeometry_withoutScreen_noChange();
    void test_onPrimaryScreenChanged_updatesGeometry();
    void test_onPrimaryScreenChanged_nullScreenIgnored();
    void test_init_setsQmlSource();
    void test_globalSettingsTabletModeChangesDriveWindowState();
    void test_handGestureSignalsDriveNotificationCenter();
    void test_callNotificationCenter_requiresRootAndTabletMode();
    void test_callNotificationCenter_clampsPositionAndShowsWindow();
    void test_callNotificationCenter_whenAlreadyVisibleOnlyUpdatesPosition();
    void test_callNotificationCenterEnd_hiddenWindow_noAnimation();
    void test_callNotificationCenterEnd_recordsSlideEvent();
    void test_callNotificationCenterEnd_belowThresholdHides();
    void test_activeNotificationCenter_showAndHide();
    void test_event_exposeBranches();
};

void notificationWindow_test::init()
{
    Sidebar::GlobalSettings::globalInstance()->reset();
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);
    Sidebar::HandGestureHelper::getInstance()->reset();
    Sidebar::EventTrack::reset();
    UkuiNotification::NotificationModel::instance()->reset();
    QX11Info::setPlatformX11(false);
}

void notificationWindow_test::test_globalGroupModel_filtersStoredNotifications()
{
    QList<UkuiNotification::TestNotificationData> items;
    items.append({1, "mail", false, true, true, "summary", "body"});
    items.append({2, "mail", false, false, true, "summary", "body"});
    items.append({3, "chat", false, true, true, "summary", "body"});
    NotificationModel::instance()->setItems(items);

    NotificationGroupModel *model = NotificationCenterWindow::globalGroupModel();

    QCOMPARE(model->rowCount(QModelIndex()), 2);
    QCOMPARE(model->data(model->index(0, 0, QModelIndex()), NotificationItem::GroupName).toString(), QString("mail"));
    QCOMPARE(model->data(model->index(1, 0, QModelIndex()), NotificationItem::GroupName).toString(), QString("chat"));
}

void notificationWindow_test::test_setContentVisible_emitsSignal()
{
    NotificationCenterWindow window;
    QSignalSpy spy(&window, &NotificationCenterWindow::contentVisibleChanged);

    window.setContentVisible(true);

    QCOMPARE(window.contentVisible(), true);
    QCOMPARE(spy.count(), 1);
}

void notificationWindow_test::test_updateGeometry_withoutScreen_noChange()
{
    NotificationCenterWindow window;
    QRect before = window.geometry();

    window.m_screen = nullptr;
    window.updateGeometry();

    QCOMPARE(window.geometry(), before);
}

void notificationWindow_test::test_onPrimaryScreenChanged_updatesGeometry()
{
    NotificationCenterWindow window;
    QScreen *screen = qApp->primaryScreen();
    QVERIFY(screen != nullptr);

    window.onPrimaryScreenChanged(screen);

    QCOMPARE(window.m_screen, screen);
    QCOMPARE(window.geometry(), screen->geometry());
    QCOMPARE(window.m_windowProxy->position(), screen->geometry().topLeft());
}

void notificationWindow_test::test_onPrimaryScreenChanged_nullScreenIgnored()
{
    NotificationCenterWindow window;
    QScreen *screen = window.m_screen;
    QRect geometry = window.geometry();

    window.onPrimaryScreenChanged(nullptr);

    QCOMPARE(window.m_screen, screen);
    QCOMPARE(window.geometry(), geometry);
}

void notificationWindow_test::test_init_setsQmlSource()
{
    NotificationCenterWindow window;

    window.init();

    QVERIFY(window.engine() != nullptr);
}

void notificationWindow_test::test_globalSettingsTabletModeChangesDriveWindowState()
{
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    NotificationCenterWindow window;
    window.resize(200, 100);
    auto *root = new TestRootItem;
    window.setTestRootObject(root);
    window.setVisible(true);

    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, false);

    QCOMPARE(window.m_isTabletMode, false);
    QCOMPARE(root->animationCalls, 1);
    QCOMPARE(root->lastAnimationValue, false);
}

void notificationWindow_test::test_handGestureSignalsDriveNotificationCenter()
{
    Sidebar::GlobalSettings::globalInstance()->setValue(TABLET_MODE, true);

    NotificationCenterWindow window;
    window.resize(200, 100);
    auto *root = new TestRootItem;
    window.setTestRootObject(root);

    Sidebar::HandGestureHelper::getInstance()->callNotificationCenter(40);
    QCOMPARE(window.isVisible(), true);
    QCOMPARE(root->lastContentY, -60);

    Sidebar::HandGestureHelper::getInstance()->top2BottomRelease(0, 10);
    QCOMPARE(root->animationCalls, 1);
    QCOMPARE(root->lastAnimationValue, false);
}

void notificationWindow_test::test_callNotificationCenter_requiresRootAndTabletMode()
{
    NotificationCenterWindow window;
    window.resize(200, 100);
    window.m_isTabletMode = false;

    window.callNotificationCenter(30);
    QCOMPARE(window.isVisible(), false);

    auto *root = new TestRootItem;
    window.setTestRootObject(root);
    window.m_isTabletMode = true;
    window.m_contentVisible = true;
    window.callNotificationCenter(30);

    QCOMPARE(root->updateCalls, 0);
}

void notificationWindow_test::test_callNotificationCenter_clampsPositionAndShowsWindow()
{
    NotificationCenterWindow window;
    window.resize(200, 100);
    window.m_isTabletMode = true;
    auto *root = new TestRootItem;
    window.setTestRootObject(root);

    window.callNotificationCenter(120);

    QCOMPARE(window.isVisible(), true);
    QCOMPARE(root->updateCalls, 2);
    QCOMPARE(root->lastContentY, 0);

    window.callNotificationCenter(-10);
    QCOMPARE(root->lastContentY, -100);
}

void notificationWindow_test::test_callNotificationCenter_whenAlreadyVisibleOnlyUpdatesPosition()
{
    NotificationCenterWindow window;
    window.resize(200, 100);
    window.m_isTabletMode = true;
    auto *root = new TestRootItem;
    window.setTestRootObject(root);
    window.setVisible(true);

    window.callNotificationCenter(25);

    QCOMPARE(root->updateCalls, 1);
    QCOMPARE(root->lastContentY, -75);
}

void notificationWindow_test::test_callNotificationCenterEnd_hiddenWindow_noAnimation()
{
    NotificationCenterWindow window;
    auto *root = new TestRootItem;
    window.setTestRootObject(root);

    window.callNotificationCenterEnd(0, 50);

    QCOMPARE(root->animationCalls, 0);
    QVERIFY(Sidebar::EventTrack::lastCode().isEmpty());
}

void notificationWindow_test::test_callNotificationCenterEnd_recordsSlideEvent()
{
    NotificationCenterWindow window;
    window.resize(200, 100);
    auto *root = new TestRootItem;
    window.setTestRootObject(root);
    window.setVisible(true);

    window.callNotificationCenterEnd(0, 30);

    QCOMPARE(root->animationCalls, 1);
    QCOMPARE(root->lastAnimationValue, true);
    QCOMPARE(Sidebar::EventTrack::lastCode(), QString("open_notification_center"));
    QCOMPARE(Sidebar::EventTrack::lastPage(), QString("notificationCenter"));
    QCOMPARE(Sidebar::EventTrack::lastMap().value("type").toString(), QString("slide"));
}

void notificationWindow_test::test_callNotificationCenterEnd_belowThresholdHides()
{
    NotificationCenterWindow window;
    window.resize(200, 100);
    auto *root = new TestRootItem;
    window.setTestRootObject(root);
    window.setVisible(true);

    window.callNotificationCenterEnd(0, 20);

    QCOMPARE(root->animationCalls, 1);
    QCOMPARE(root->lastAnimationValue, false);
}

void notificationWindow_test::test_activeNotificationCenter_showAndHide()
{
    NotificationCenterWindow window;
    window.resize(200, 100);
    window.m_isTabletMode = true;
    auto *root = new TestRootItem;
    window.setTestRootObject(root);

    window.activeNotificationCenter(true);
    QCOMPARE(root->updateCalls >= 1, true);
    QCOMPARE(root->animationCalls >= 1, true);

    const int previousCalls = root->animationCalls;
    window.activeNotificationCenter(false);
    QCOMPARE(root->animationCalls, previousCalls + 1);
}

void notificationWindow_test::test_event_exposeBranches()
{
    NotificationCenterWindow window;
    QExposeEvent expose(QRegion(0, 0, 10, 10));

    QX11Info::setPlatformX11(false);
    QVERIFY(window.event(&expose));

    QX11Info::setPlatformX11(true);
    QVERIFY(window.event(&expose));
}

QTEST_MAIN(notificationWindow_test)

#include "notification-window-test.moc"
