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

#include "../../../src/notification/notification-utils.h"

using namespace UkuiNotification;

class notificationUtils_test : public QObject
{
    Q_OBJECT

private slots:
    void test_defaultConstructor();
    void test_valueConstructor();
    void test_settersAndGetters();
    void test_metaTypeRegistration();
};

void notificationUtils_test::test_defaultConstructor()
{
    NotificationAction action;

    QVERIFY(action.isEnable());
    QVERIFY(action.isDefault());
    QCOMPARE(action.index(), 0);
    QCOMPARE(action.mode(), NotificationAction::Text);
    QVERIFY(action.icon().isEmpty());
    QVERIFY(action.name().isEmpty());
    QVERIFY(action.action().isEmpty());
    QVERIFY(action.state().isEmpty());
}

void notificationUtils_test::test_valueConstructor()
{
    NotificationAction action(false,
                              false,
                              7,
                              NotificationAction::IconAndText,
                              "dialog-info",
                              "Open",
                              "disabled");

    QCOMPARE(action.isEnable(), false);
    QCOMPARE(action.isDefault(), false);
    QCOMPARE(action.index(), 7);
    QCOMPARE(action.mode(), NotificationAction::IconAndText);
    QCOMPARE(action.icon(), QString("dialog-info"));
    QCOMPARE(action.name(), QString("Open"));
    QCOMPARE(action.state(), QString("disabled"));
    QVERIFY(action.action().isEmpty());
}

void notificationUtils_test::test_settersAndGetters()
{
    NotificationAction action;

    action.setEnable(false);
    action.setDefault(false);
    action.setIndex(3);
    action.setMode(NotificationAction::Icon);
    action.setIcon("mail-unread");
    action.setName("Reply");
    action.setAction("reply");
    action.setState("ready");

    QCOMPARE(action.isEnable(), false);
    QCOMPARE(action.isDefault(), false);
    QCOMPARE(action.index(), 3);
    QCOMPARE(action.mode(), NotificationAction::Icon);
    QCOMPARE(action.icon(), QString("mail-unread"));
    QCOMPARE(action.name(), QString("Reply"));
    QCOMPARE(action.action(), QString("reply"));
    QCOMPARE(action.state(), QString("ready"));
}

void notificationUtils_test::test_metaTypeRegistration()
{
    NotificationAction action;
    QVariant variant = QVariant::fromValue(action);

    QVERIFY(variant.isValid());
    QCOMPARE(variant.userType(), qMetaTypeId<NotificationAction>());
    QVERIFY(variant.canConvert<NotificationAction>());
}

QTEST_MAIN(notificationUtils_test)

#include "notification-utils-test.moc"
