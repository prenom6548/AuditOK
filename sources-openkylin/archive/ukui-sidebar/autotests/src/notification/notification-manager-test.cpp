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

#include "../../../src/notification/notification-manager.h"
#include "stubs/notification-helper.h"
#include "stubs/app-manager.h"

using namespace Notify;

Q_DECLARE_METATYPE(Notify::Message)

class notificationManager_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_newMessageRecived_storesMessage();
    void test_action_returnsMessageResult();
    void test_deleteMessage_removesSingleEntry();
    void test_deleteAppMessage_removesAppEntries();
    void test_deleteAllMessage_clearsAll();
    void test_openSystemSetting_forwardsToAppManager();

private:
    NotificationManager manager;
};

void notificationManager_test::init()
{
    qRegisterMetaType<Notify::Message>("Message");
    manager.deleteAllMessage();
    Sidebar::AppManager::reset();
}

void notificationManager_test::test_newMessageRecived_storesMessage()
{
    Message message("1", "mail", true);
    QSignalSpy newMessageSpy(&manager, &NotificationManager::newMessage);
    QSignalSpy unreadSpy(&manager, &NotificationManager::iHaveUnreadMessage);

    Notify::NotificationHelper::instance()->emitNewMessage(message);

    QCOMPARE(newMessageSpy.count(), 1);
    QCOMPARE(unreadSpy.count(), 1);
    QCOMPARE(manager.allAppNames(), QStringList() << "mail");

    QString appName("mail");
    QCOMPARE(manager.getMessages(appName).size(), 1);
}

void notificationManager_test::test_action_returnsMessageResult()
{
    Message message("2", "chat", false);
    manager.newMessageRecived(message);

    QCOMPARE(manager.action("2"), false);
    QCOMPARE(manager.action("missing"), true);
}

void notificationManager_test::test_deleteMessage_removesSingleEntry()
{
    Message first("1", "mail", true);
    Message second("2", "mail", true);
    manager.newMessageRecived(first);
    manager.newMessageRecived(second);

    manager.deleteMessage("1", "mail");

    QString appName("mail");
    QCOMPARE(manager.getMessages(appName).size(), 1);
}

void notificationManager_test::test_deleteAppMessage_removesAppEntries()
{
    Message first("1", "mail", true);
    Message second("2", "mail", true);
    Message third("3", "chat", true);
    manager.newMessageRecived(first);
    manager.newMessageRecived(second);
    manager.newMessageRecived(third);

    manager.deleteAppMessage("mail");

    QString mail("mail");
    QString chat("chat");
    QCOMPARE(manager.getMessages(mail).size(), 0);
    QCOMPARE(manager.getMessages(chat).size(), 1);
}

void notificationManager_test::test_deleteAllMessage_clearsAll()
{
    Message first("1", "mail", true);
    Message second("2", "chat", true);
    manager.newMessageRecived(first);
    manager.newMessageRecived(second);

    manager.deleteAllMessage();

    QCOMPARE(manager.allAppNames().size(), 0);
}

void notificationManager_test::test_openSystemSetting_forwardsToAppManager()
{
    manager.openSystemSetting();

    QCOMPARE(Sidebar::AppManager::lastDesktopFile(), QString("/usr/share/applications/ukui-control-center.desktop"));
    QCOMPARE(Sidebar::AppManager::lastAppName(), QString("ukui-control-center"));
    QCOMPARE(Sidebar::AppManager::lastArgs(), QStringList({"-m", "Notice"}));
}

QTEST_MAIN(notificationManager_test)

#include "notification-manager-test.moc"
