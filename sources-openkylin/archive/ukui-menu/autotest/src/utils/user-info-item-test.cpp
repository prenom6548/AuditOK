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

#define private public
#include "../../../src/utils/user-info-item.h"
#undef private

#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QImage>

using namespace UkuiMenu;

class userInfoItem_test : public QObject
{
    Q_OBJECT

private slots:
    void test_constructor_defaultsWhenDbusInvalid();
    void test_createCircularIcon_handlesMissingFile();
    void test_createCircularIcon_generatesIcon();
    void test_userInfoUpdateSlot_updatesAllTrackedFields();
};

void userInfoItem_test::test_constructor_defaultsWhenDbusInvalid()
{
    UserInfoItem item(QStringLiteral("/org/freedesktop/Accounts/User99999999"));

    QCOMPARE(item.objPath(), QStringLiteral("/org/freedesktop/Accounts/User99999999"));
    QCOMPARE(item.userName(), QString());
    QCOMPARE(item.realName(), QString());
    QCOMPARE(item.iconFile(), QString());
    QCOMPARE(item.isCurrentUser(), false);
    QCOMPARE(item.isLogged(), false);
    QCOMPARE(item.autoLogin(), false);
}

void userInfoItem_test::test_createCircularIcon_handlesMissingFile()
{
    UserInfoItem item(QStringLiteral("/org/freedesktop/Accounts/User1000"));
    const QIcon originalIcon = item.circularIcon();
    item.m_iconFile = QStringLiteral("/tmp/this-file-should-not-exist.png");

    item.createCircularIcon();

    QCOMPARE(item.circularIcon().cacheKey(), originalIcon.cacheKey());
}

void userInfoItem_test::test_createCircularIcon_generatesIcon()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imagePath = dir.path() + QLatin1String("/avatar.png");
    QImage image(20, 10, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath));

    UserInfoItem item(QStringLiteral("/org/freedesktop/Accounts/User1000"));
    item.m_iconFile = imagePath;

    item.createCircularIcon();

    QVERIFY(!item.circularIcon().isNull());
}

void userInfoItem_test::test_userInfoUpdateSlot_updatesAllTrackedFields()
{
    UserInfoItem item(QStringLiteral("/org/freedesktop/Accounts/User1000"));
    QSignalSpy spy(&item, &UserInfoItem::userInfoChanged);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.path() + QLatin1String("/avatar.png");
    QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::blue);
    QVERIFY(image.save(imagePath));

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert(QStringLiteral("IconFile"), imagePath);
    propertyMap.insert(QStringLiteral("UserName"), QStringLiteral("tester"));
    propertyMap.insert(QStringLiteral("RealName"), QStringLiteral("Test User"));
    propertyMap.insert(QStringLiteral("AccountType"), 1);

    item.userInfoUpdateSlot(QStringLiteral("org.freedesktop.Accounts.User"), propertyMap, {});

    QCOMPARE(item.iconFile(), imagePath);
    QCOMPARE(item.userName(), QStringLiteral("tester"));
    QCOMPARE(item.realName(), QStringLiteral("Test User"));
    QCOMPARE(item.accountType(), 1);
    QVERIFY(!item.circularIcon().isNull());
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(userInfoItem_test)

#include "user-info-item-test.moc"
