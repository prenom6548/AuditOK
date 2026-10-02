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

#define private public
#include "../../../src/notification/notification-model.h"
#undef private
#include "stubs/applications-settings.h"
#include "stubs/notification-global-settings.h"
#include "stubs/notification-client.h"
#include "stubs/popup-notification.h"
#include "stubs/screen-monitor.h"

using namespace UkuiNotification;

class notificationModel_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_instance_singleton();
    void test_roleNames_containsExpectedEntries();
    void test_onNotificationReceived_addsNotification();
    void test_onNotificationReceived_updatesExistingNotification();
    void test_onNotificationReceived_honorsGlobalAndAppSettings();
    void test_notificationData_roles_coverCoreFields();
    void test_setNotificationData_marksStored();
    void test_execAction_invokesClientAndRemovesNotification();
    void test_expirationAndStoreTransitions();
    void test_dndClockNotification_usesInitPath();
    void test_maximumNotificationCount_removesOldest();
    void test_onNotificationClosed_sidebarVisible_marksDelayRemove();
    void test_storePopupNotification_marksTransientAsStored();
    void test_clearAll_removesAllRows();
    void test_processedBody_usesLinkColor();
    void test_closeNotification_removesRow();
    void test_actions_withoutIconsAndDefaultState();
    void test_invalidDataAndSetDataBranches();
    void test_onNotificationClosed_hiddenSidebar_removesAfterDelay();
    void test_removeNotifications_groupsPendingRows();
    void test_updateNotification_replacesPendingRemovalAndKeepsReplacement();
    void test_oldCloseForPendingReplacement_isIgnoredOnlyOnce();
    void test_execAction_expiredNotificationOnlyCloses();
    void test_timerCombinations_coverStoreAndExpireBranches();
    void test_sidebarVisibleNewPopupIsStored();
    void test_residentPopupAndSoundBranches();
    void test_missingIdsAndPrivateHelpersReturnSafely();
    void test_residentExecActionKeepsNotification();
    void test_linkColorAndProcessedBodyFallbacks();
    void test_storePopupNotificationNoopAndRepeat();

private:
    PopupNotification createNotification(uint id, const QString &body = QString("body"), int popupTimeout = 0);
    NotificationModel *model();
    void clearModel();
    bool sendNotification(const PopupNotification &notification);
};

PopupNotification notificationModel_test::createNotification(uint id, const QString &body, int popupTimeout)
{
    PopupNotification notification;
    notification.setId(id);
    notification.setDisplay(QString("display-%1").arg(id));
    notification.setApplicationName(QString("app-%1").arg(id));
    notification.setApplicationIconName(QString("icon-%1").arg(id));
    notification.setSummary(QString("summary-%1").arg(id));
    notification.setBody(body);
    notification.setCategory(QString("category-%1").arg(id));
    notification.setIcon(QString("pixmap-%1").arg(id));
    notification.setCreatedTime(QDateTime(QDate(2024, 1, 1), QTime(10, 0, 0).addSecs(id)));
    notification.setHasDefaultAction(true);
    notification.setEnableActionIcons(true);
    notification.setActions(ActionList{{"default", "Open"}, {"reply", "Reply"}});
    notification.setActionState(QStringList{"ready", "idle"});
    notification.setSuppressSound(false);
    notification.setResident(false);
    notification.setTransient(true);
    notification.setUrgency(PopupNotification::CriticalUrgency);
    notification.setPopupTimeout(popupTimeout);
    notification.setTimeout(0);
    return notification;
}

NotificationModel *notificationModel_test::model()
{
    return NotificationModel::instance();
}

void notificationModel_test::clearModel()
{
    if (model()->rowCount(QModelIndex()) > 0) {
        model()->clearAll();
    }
    model()->m_sidebarVisible = false;
    NotificationClient::reset();
    ApplicationsSettings::settings()->setAllowNotify(true);
    ApplicationsSettings::settings()->setAllowSound(false);
    ApplicationsSettings::settings()->setShowNotificationOnLockScreen(true);
    ApplicationsSettings::settings()->setShowContentOnLockScreen(true);
    Sidebar::ScreenMonitor::getInstance()->setScreenLockState(false);
    if (NotificationGlobalSettings::lastInstance()) {
        NotificationGlobalSettings::lastInstance()->setReceiveNotificationsFromApps(true);
        NotificationGlobalSettings::lastInstance()->setDND(false);
        NotificationGlobalSettings::lastInstance()->setNotifyAlarmWhileDND(false);
    }
}

bool notificationModel_test::sendNotification(const PopupNotification &notification)
{
    return QMetaObject::invokeMethod(model(),
                                     "onNotificationReceived",
                                     Qt::DirectConnection,
                                     Q_ARG(UkuiNotification::PopupNotification, notification));
}

void notificationModel_test::init()
{
    clearModel();
}

void notificationModel_test::test_instance_singleton()
{
    QCOMPARE(model(), NotificationModel::instance());
}

void notificationModel_test::test_roleNames_containsExpectedEntries()
{
    QHash<int, QByteArray> roles = model()->roleNames();

    QCOMPARE(roles.value(NotificationItem::Id), QByteArray("id"));
    QCOMPARE(roles.value(NotificationItem::Summary), QByteArray("summary"));
    QCOMPARE(roles.value(NotificationItem::ProcessedBody), QByteArray("processedBody"));
}

void notificationModel_test::test_onNotificationReceived_addsNotification()
{
    PopupNotification notification = createNotification(1, "plain", 0);

    QVERIFY(sendNotification(notification));

    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Id).toUInt(), 1u);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::AppName).toString(), QString("app-1"));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::HasDefaultAction).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Actions).toList().size(), 2);
}

void notificationModel_test::test_onNotificationReceived_updatesExistingNotification()
{
    PopupNotification notification = createNotification(11, "first", 0);
    QVERIFY(sendNotification(notification));

    notification.setSummary("summary-updated");
    notification.setBody("body-updated");
    QSignalSpy spy(model(), &QAbstractItemModel::dataChanged);

    QVERIFY(sendNotification(notification));

    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QVERIFY(spy.count() >= 1);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Summary).toString(), QString("summary-updated"));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Body).toString(), QString("body-updated"));
}

void notificationModel_test::test_onNotificationReceived_honorsGlobalAndAppSettings()
{
    PopupNotification notification = createNotification(12, "plain", 0);

    NotificationGlobalSettings::lastInstance()->setReceiveNotificationsFromApps(false);
    QVERIFY(sendNotification(notification));
    QCOMPARE(model()->rowCount(QModelIndex()), 0);

    NotificationGlobalSettings::lastInstance()->setReceiveNotificationsFromApps(true);
    ApplicationsSettings::settings()->setAllowNotify(false);
    QVERIFY(sendNotification(notification));
    QCOMPARE(model()->rowCount(QModelIndex()), 0);
}

void notificationModel_test::test_notificationData_roles_coverCoreFields()
{
    PopupNotification notification = createNotification(13, "plain", 0);
    notification.setIcon(QString());
    notification.setApplicationIconName(QString());
    notification.setImage(QImage(2, 2, QImage::Format_ARGB32));
    notification.setSoundFile("/tmp/a.ogg");
    notification.setResident(true);
    notification.setTransient(false);
    notification.setNoFold(true);
    notification.setTimeout(150);

    QVERIFY(sendNotification(notification));

    QModelIndex idx = model()->index(0, 0);
    QCOMPARE(model()->data(idx, NotificationItem::Display).toString(), QString("display-13"));
    QCOMPARE(model()->data(idx, NotificationItem::AppIconName).toString(), QString("application-x-desktop"));
    QCOMPARE(model()->data(idx, NotificationItem::Category).toString(), QString("category-13"));
    QVERIFY(!model()->data(idx, NotificationItem::Image).value<QImage>().isNull());
    QCOMPARE(model()->data(idx, NotificationItem::CreateTime).toDateTime(), notification.createdTime());
    QCOMPARE(model()->data(idx, NotificationItem::EnableActionIcons).toBool(), true);
    QCOMPARE(model()->data(idx, NotificationItem::SoundFile).toString(), QString("/tmp/a.ogg"));
    QCOMPARE(model()->data(idx, NotificationItem::SuppressSound).toBool(), false);
    QCOMPARE(model()->data(idx, NotificationItem::Resident).toBool(), true);
    QCOMPARE(model()->data(idx, NotificationItem::Transient).toBool(), false);
    QCOMPARE(model()->data(idx, NotificationItem::Urgency).toInt(), PopupNotification::CriticalUrgency);
    QCOMPARE(model()->data(idx, NotificationItem::Timout).toInt(), 150);
    QCOMPARE(model()->data(idx, NotificationItem::NoFold).toBool(), true);
    QCOMPARE(model()->data(idx, NotificationItem::PopupTimeout).toInt(), 0);
    QCOMPARE(model()->data(idx, NotificationItem::ShowOnLockScreen).toBool(), true);
    QCOMPARE(model()->data(idx, NotificationItem::ShowContentOnLockScreen).toBool(), true);
}

void notificationModel_test::test_setNotificationData_marksStored()
{
    PopupNotification notification = createNotification(2, "plain", 1000);
    QVERIFY(sendNotification(notification));

    QVERIFY(model()->setNotificationData(2, true, NotificationItem::IsStored));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ToBeStored).toBool(), false);
}

void notificationModel_test::test_execAction_invokesClientAndRemovesNotification()
{
    PopupNotification notification = createNotification(14, "plain", 0);
    notification.setResident(false);
    QVERIFY(sendNotification(notification));

    model()->execAction(14, "reply");
    QTest::qWait(80);

    QCOMPARE(NotificationClient::invokedIds(), QList<uint>({14u}));
    QCOMPARE(NotificationClient::invokedActions(), QStringList({"reply"}));
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({14u}));
    QCOMPARE(model()->rowCount(QModelIndex()), 0);
}

void notificationModel_test::test_expirationAndStoreTransitions()
{
    PopupNotification transientNotification = createNotification(15, "plain", 20);
    transientNotification.setTimeout(40);
    ApplicationsSettings::settings()->setShowNotificationOnLockScreen(false);
    Sidebar::ScreenMonitor::getInstance()->setScreenLockState(true);

    QVERIFY(sendNotification(transientNotification));
    QTest::qWait(30);

    QModelIndex idx = model()->index(0, 0);
    QCOMPARE(model()->data(idx, NotificationItem::IsStored).toBool(), true);
    QCOMPARE(model()->getUnreadMessageCount(), 1);

    QTest::qWait(40);
    QCOMPARE(model()->data(idx, NotificationItem::IsExpired).toBool(), true);

    PopupNotification dndNotification = createNotification(16, "plain", 0);
    NotificationGlobalSettings::lastInstance()->setDND(true);
    QVERIFY(sendNotification(dndNotification));
    QCOMPARE(model()->data(model()->index(1, 0), NotificationItem::IsStored).toBool(), true);
}

void notificationModel_test::test_dndClockNotification_usesInitPath()
{
    PopupNotification notification = createNotification(160, "clock", 10);
    notification.setDesktopEntry("ukui-clock.desktop");
    NotificationGlobalSettings::lastInstance()->setDND(true);
    NotificationGlobalSettings::lastInstance()->setNotifyAlarmWhileDND(true);

    QVERIFY(sendNotification(notification));
    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), false);
}

void notificationModel_test::test_maximumNotificationCount_removesOldest()
{
    for (uint id = 1000; id < 1500; ++id) {
        QVERIFY(sendNotification(createNotification(id, QString("body-%1").arg(id), 0)));
    }
    QCOMPARE(model()->rowCount(QModelIndex()), 500);

    QVERIFY(sendNotification(createNotification(1500, "body-1500", 0)));

    QCOMPARE(model()->rowCount(QModelIndex()), 500);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Id).toUInt(), 1001u);
    QCOMPARE(model()->data(model()->index(499, 0), NotificationItem::Id).toUInt(), 1500u);
}

void notificationModel_test::test_onNotificationClosed_sidebarVisible_marksDelayRemove()
{
    PopupNotification notification = createNotification(170, "plain", 0);
    QVERIFY(sendNotification(notification));

    model()->m_sidebarVisible = true;
    QModelIndex idx = model()->index(0, 0);
    QCOMPARE(model()->data(idx, NotificationItem::DelayRemove).toBool(), false);

    QSignalSpy spy(model(), &QAbstractItemModel::dataChanged);
    QMetaObject::invokeMethod(model(),
                              "onNotificationClosed",
                              Qt::DirectConnection,
                              Q_ARG(uint, 170u),
                              Q_ARG(UkuiNotification::NotificationCloseReason::CloseReason,
                                    UkuiNotification::NotificationCloseReason::DismissedByUser));

    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QCOMPARE(model()->data(idx, NotificationItem::DelayRemove).toBool(), true);
    QVERIFY(spy.count() >= 1);
}

void notificationModel_test::test_storePopupNotification_marksTransientAsStored()
{
    PopupNotification notification = createNotification(17, "plain", 100);
    QVERIFY(sendNotification(notification));

    model()->storePopupNotification(true);

    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), true);
    QCOMPARE(model()->getUnreadMessageCount(), 0);
}

void notificationModel_test::test_clearAll_removesAllRows()
{
    QVERIFY(sendNotification(createNotification(18, "one", 0)));
    QVERIFY(sendNotification(createNotification(19, "two", 0)));

    model()->clearAll();

    QCOMPARE(model()->rowCount(QModelIndex()), 0);
    QCOMPARE(NotificationClient::closedIds().size(), 2);
}

void notificationModel_test::test_processedBody_usesLinkColor()
{
    PopupNotification notification = createNotification(3, "<a href='https://example.com'>link</a>", 0);
    QVERIFY(sendNotification(notification));

    model()->setLinkActiveColor("#ff0000");

    const QString processed = model()->data(model()->index(0, 0), NotificationItem::ProcessedBody).toString();
    QVERIFY(processed.contains("#ff0000"));
    QVERIFY(processed.contains("link"));
}

void notificationModel_test::test_closeNotification_removesRow()
{
    PopupNotification notification = createNotification(4, "plain", 0);
    QVERIFY(sendNotification(notification));

    model()->closeNotification(4);
    QTest::qWait(80);

    QCOMPARE(model()->rowCount(QModelIndex()), 0);
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({4u}));
}

void notificationModel_test::test_actions_withoutIconsAndDefaultState()
{
    PopupNotification notification = createNotification(20, "plain", 0);
    notification.setHasDefaultAction(false);
    notification.setEnableActionIcons(false);
    notification.setActions(ActionList{{"default", "Open"}, {"reply", "Reply"}, {"close", "Close"}});
    notification.setActionState(QStringList{"ready"});

    QVERIFY(sendNotification(notification));

    const QVariantList actions = model()->data(model()->index(0, 0), NotificationItem::Actions).toList();
    QCOMPARE(actions.size(), 3);

    const NotificationAction first = actions.at(0).value<NotificationAction>();
    const NotificationAction second = actions.at(1).value<NotificationAction>();
    const NotificationAction third = actions.at(2).value<NotificationAction>();
    QCOMPARE(first.isDefault(), false);
    QCOMPARE(first.icon(), QString());
    QCOMPARE(first.state(), QString("ready"));
    QCOMPARE(second.icon(), QString());
    QCOMPARE(second.state(), QString());
    QCOMPARE(third.action(), QString("close"));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ActionState).isValid(), false);
}

void notificationModel_test::test_invalidDataAndSetDataBranches()
{
    PopupNotification notification = createNotification(21, "plain", 0);
    notification.setIcon(QString());
    notification.setApplicationIconName("app-fallback-icon");
    notification.setImage(QImage());
    QVERIFY(sendNotification(notification));

    QCOMPARE(model()->data(QModelIndex(), NotificationItem::Id), QVariant());
    QCOMPARE(model()->data(model()->index(0, 0), 9999), QVariant());
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::AppIconName).toString(),
             QString("app-fallback-icon"));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Icon).toString(), QString());
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Image), QVariant());

    QVERIFY(!model()->setData(QModelIndex(), true, NotificationItem::IsStored));
    QVERIFY(!model()->setData(model()->index(0, 0), false, NotificationItem::IsStored));
    QVERIFY(!model()->setData(model()->index(0, 0), true, 9999));
    QVERIFY(!model()->setNotificationData(9999, true, NotificationItem::IsStored));

    QVERIFY(model()->setData(model()->index(0, 0), true, NotificationItem::DelayRemove));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::DelayRemove).toBool(), true);
    QVERIFY(model()->setData(model()->index(0, 0), false, NotificationItem::DelayRemove));
    QTest::qWait(80);
    QCOMPARE(model()->rowCount(QModelIndex()), 0);
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({21u}));
}

void notificationModel_test::test_onNotificationClosed_hiddenSidebar_removesAfterDelay()
{
    QVERIFY(sendNotification(createNotification(22, "plain", 0)));

    QMetaObject::invokeMethod(model(),
                              "onNotificationClosed",
                              Qt::DirectConnection,
                              Q_ARG(uint, 22u),
                              Q_ARG(UkuiNotification::NotificationCloseReason::CloseReason,
                                    UkuiNotification::NotificationCloseReason::DismissedByUser));
    QCOMPARE(model()->rowCount(QModelIndex()), 1);

    QTest::qWait(80);
    QCOMPARE(model()->rowCount(QModelIndex()), 0);
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({22u}));
}

void notificationModel_test::test_removeNotifications_groupsPendingRows()
{
    QVERIFY(sendNotification(createNotification(30, "first", 0)));
    QVERIFY(sendNotification(createNotification(31, "second", 0)));
    QVERIFY(sendNotification(createNotification(32, "third", 0)));
    QVERIFY(sendNotification(createNotification(33, "fourth", 0)));

    model()->removeNotification(30);
    model()->removeNotification(32);
    model()->removeNotification(33);
    QTest::qWait(80);

    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Id).toUInt(), 31u);
    QCOMPARE(NotificationClient::closedIds().size(), 3);
    QVERIFY(NotificationClient::closedIds().contains(30u));
    QVERIFY(NotificationClient::closedIds().contains(32u));
    QVERIFY(NotificationClient::closedIds().contains(33u));
}

void notificationModel_test::test_updateNotification_replacesPendingRemovalAndKeepsReplacement()
{
    PopupNotification notification = createNotification(40, "old", 20);
    QVERIFY(sendNotification(notification));
    QTest::qWait(40);
    QCOMPARE(model()->getUnreadMessageCount(), 1);

    QSignalSpy rowsRemovedSpy(model(), &QAbstractItemModel::rowsRemoved);
    QSignalSpy rowsInsertedSpy(model(), &QAbstractItemModel::rowsInserted);
    QSignalSpy dataChangedSpy(model(), &QAbstractItemModel::dataChanged);

    model()->closeNotification(40);

    PopupNotification updated = createNotification(40, "new", 0);

    QVERIFY(sendNotification(updated));
    QTest::qWait(80);

    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Body).toString(), QString("new"));
    QCOMPARE(model()->getUnreadMessageCount(), 0);
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({40u}));
    QCOMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 0);
}

void notificationModel_test::test_oldCloseForPendingReplacement_isIgnoredOnlyOnce()
{
    PopupNotification notification = createNotification(41, "old", 0);
    QVERIFY(sendNotification(notification));

    model()->closeNotification(41);

    PopupNotification updated = notification;
    updated.setBody("new");
    QVERIFY(sendNotification(updated));

    QMetaObject::invokeMethod(model(),
                              "onNotificationClosed",
                              Qt::DirectConnection,
                              Q_ARG(uint, 41u),
                              Q_ARG(UkuiNotification::NotificationCloseReason::CloseReason,
                                    UkuiNotification::NotificationCloseReason::DismissedByUser));

    QCOMPARE(model()->rowCount(QModelIndex()), 1);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::Body).toString(), QString("new"));
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({41u}));

    QMetaObject::invokeMethod(model(),
                              "onNotificationClosed",
                              Qt::DirectConnection,
                              Q_ARG(uint, 41u),
                              Q_ARG(UkuiNotification::NotificationCloseReason::CloseReason,
                                    UkuiNotification::NotificationCloseReason::DismissedByUser));
    QTest::qWait(80);

    QCOMPARE(model()->rowCount(QModelIndex()), 0);
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({41u, 41u}));
}

void notificationModel_test::test_execAction_expiredNotificationOnlyCloses()
{
    PopupNotification notification = createNotification(50, "plain", 0);
    notification.setTimeout(20);
    QVERIFY(sendNotification(notification));
    QTest::qWait(40);

    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsExpired).toBool(), true);
    model()->execAction(50, "reply");
    QTest::qWait(80);

    QCOMPARE(NotificationClient::invokedIds(), QList<uint>());
    QCOMPARE(NotificationClient::closedIds(), QList<uint>({50u}));
    QCOMPARE(model()->rowCount(QModelIndex()), 0);
}

void notificationModel_test::test_timerCombinations_coverStoreAndExpireBranches()
{
    PopupNotification expireFirst = createNotification(60, "expire-first", 60);
    expireFirst.setTimeout(20);
    QVERIFY(sendNotification(expireFirst));
    QTest::qWait(35);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsExpired).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), false);
    QTest::qWait(55);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ToBeStored).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), false);

    clearModel();

    PopupNotification sameTimeout = createNotification(61, "same", 20);
    sameTimeout.setTimeout(20);
    QVERIFY(sendNotification(sameTimeout));
    QTest::qWait(40);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ToBeStored).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsExpired).toBool(), true);

    clearModel();

    PopupNotification toBeStored = createNotification(62, "visible-lock", 20);
    toBeStored.setTimeout(0);
    ApplicationsSettings::settings()->setShowNotificationOnLockScreen(true);
    Sidebar::ScreenMonitor::getInstance()->setScreenLockState(true);
    QVERIFY(sendNotification(toBeStored));
    QTest::qWait(40);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ToBeStored).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), false);
}

void notificationModel_test::test_sidebarVisibleNewPopupIsStored()
{
    model()->storePopupNotification(true);

    PopupNotification notification = createNotification(70, "plain", 100);
    QVERIFY(sendNotification(notification));

    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ToBeStored).toBool(), false);
}

void notificationModel_test::test_residentPopupAndSoundBranches()
{
    ApplicationsSettings::settings()->setAllowSound(true);

    PopupNotification residentPopup = createNotification(80, "resident-popup", -1);
    residentPopup.setSoundName("dialog-information");
    QVERIFY(sendNotification(residentPopup));

    QModelIndex residentIndex = model()->index(0, 0);
    QCOMPARE(model()->data(residentIndex, NotificationItem::IsStored).toBool(), false);
    QCOMPARE(model()->data(residentIndex, NotificationItem::ToBeStored).toBool(), false);

    PopupNotification defaultSound = createNotification(81, "default-sound", 0);
    QVERIFY(sendNotification(defaultSound));

    PopupNotification soundFile = createNotification(82, "sound-file", 0);
    soundFile.setSoundFile("/tmp/custom.ogg");
    QVERIFY(sendNotification(soundFile));

    PopupNotification suppressed = createNotification(83, "suppressed", 0);
    suppressed.setSuppressSound(true);
    QVERIFY(sendNotification(suppressed));

    QTest::qWait(20);
    QCOMPARE(model()->rowCount(QModelIndex()), 4);
}

void notificationModel_test::test_missingIdsAndPrivateHelpersReturnSafely()
{
    model()->closeNotification(9000);
    model()->execAction(9001, "missing");
    model()->markAsStoredNotification(9002);
    model()->markAsExpiredNotification(9003);

    QMetaObject::invokeMethod(model(),
                              "onNotificationClosed",
                              Qt::DirectConnection,
                              Q_ARG(uint, 9004u),
                              Q_ARG(UkuiNotification::NotificationCloseReason::CloseReason,
                                    UkuiNotification::NotificationCloseReason::DismissedByUser));
    QMetaObject::invokeMethod(model(), "removeNotifications", Qt::DirectConnection);

    QCOMPARE(model()->rowCount(QModelIndex()), 0);
    QCOMPARE(NotificationClient::closedIds(), QList<uint>());
    QCOMPARE(NotificationClient::invokedIds(), QList<uint>());
}

void notificationModel_test::test_residentExecActionKeepsNotification()
{
    PopupNotification notification = createNotification(90, "resident-action", 0);
    notification.setResident(true);
    QVERIFY(sendNotification(notification));

    model()->execAction(90, "reply");
    QTest::qWait(80);

    QCOMPARE(NotificationClient::invokedIds(), QList<uint>({90u}));
    QCOMPARE(NotificationClient::invokedActions(), QStringList({"reply"}));
    QCOMPARE(NotificationClient::closedIds(), QList<uint>());
    QCOMPARE(model()->rowCount(QModelIndex()), 1);
}

void notificationModel_test::test_linkColorAndProcessedBodyFallbacks()
{
    QSignalSpy spy(model(), &QAbstractItemModel::dataChanged);

    PopupNotification plain = createNotification(100, "plain body", 0);
    QVERIFY(sendNotification(plain));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::ProcessedBody).toString(), QString("plain body"));
    QCOMPARE(model()->updateNotificationBody("plain body"), QString("plain body"));

    PopupNotification richWithoutLink = createNotification(101, "<b>bold</b>", 0);
    QVERIFY(sendNotification(richWithoutLink));
    QCOMPARE(model()->data(model()->index(1, 0), NotificationItem::ProcessedBody).toString(), QString("<b>bold</b>"));

    PopupNotification iconBranch = createNotification(102, "icon branch", 0);
    iconBranch.setIcon("notification-icon");
    iconBranch.setApplicationIconName("app-icon");
    QVERIFY(sendNotification(iconBranch));
    QCOMPARE(model()->data(model()->index(2, 0), NotificationItem::AppIconName).toString(), QString("notification-icon"));

    model()->setLinkActiveColor("#00aa00");
    QCOMPARE(model()->linkActiveColor(), QString("#00aa00"));
    const int changedCount = spy.count();
    model()->setLinkActiveColor("#00aa00");
    QCOMPARE(spy.count(), changedCount);
}

void notificationModel_test::test_storePopupNotificationNoopAndRepeat()
{
    model()->storePopupNotification(false);
    QCOMPARE(model()->rowCount(QModelIndex()), 0);

    PopupNotification notification = createNotification(110, "popup", 100);
    QVERIFY(sendNotification(notification));
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), false);

    QSignalSpy unreadSpy(model(), &NotificationModel::unreadMessageCountChanged);
    model()->storePopupNotification(true);
    QCOMPARE(model()->data(model()->index(0, 0), NotificationItem::IsStored).toBool(), true);
    QVERIFY(unreadSpy.count() >= 1);

    const int unreadSignalCount = unreadSpy.count();
    model()->storePopupNotification(true);
    QCOMPARE(unreadSpy.count(), unreadSignalCount);
}

QTEST_MAIN(notificationModel_test)

#include "notification-model-test.moc"
