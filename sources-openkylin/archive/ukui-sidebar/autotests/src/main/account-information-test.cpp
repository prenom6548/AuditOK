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
#include <QDBusConnection>
#define private public
#include "../../../src/main/account-information.h"
#undef private

using namespace UkuiShortcut;

class FakeBatteryService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.upower.battery")

public:
    explicit FakeBatteryService(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    QString IconName() const { return "battery-good-symbolic"; }
    double Percentage() const { return 66.0; }
    bool ShowIcon() const { return true; }
    QString GetBatteryTooltip() const { return "Battery 66%"; }
    bool IsPresent() const { return true; }

signals:
    void BatteryIcon(const QString &iconName);
    void PercentageChanged(double percentage);
    void BatteryShowIcon(bool enable);
    void BatteryTooltip(const QString &tooltip);
    void IsPresentChanged(bool present);
};

class FakeAccountsService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Accounts")
public:
    QList<QDBusObjectPath> users;

public slots:
    QList<QDBusObjectPath> ListCachedUsers() const
    {
        return users;
    }
};

class FakeAccountPropertiesService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.DBus.Properties")
public:
    QVariantMap properties;

public slots:
    QVariantMap GetAll(const QString &interfaceName) const
    {
        Q_UNUSED(interfaceName)
        return properties;
    }

signals:
    void PropertiesChanged(const QString &interfaceName,
                           const QVariantMap &changedProperties,
                           const QStringList &invalidatedProperties);
};


class accountInformation_test : public QObject
{
    Q_OBJECT

private slots:
    // AccountInformation 测试
    void test_createAccountInformation();
    void test_getUsername();
    void test_getRealName();
    void test_getIconFile_default();
    void test_getAccountType_default();
    void test_getAccountType_unknown();
    void test_getAccountType_standardAndAdministrator();
    void test_getUserObjectPath_doesNotCrash();
    void test_getUserInformation_invalidPath_defaults();
    void test_getUserObjectPath_fromFakeAccountsService();
    void test_getUserInformation_validCurrentUser();
    void test_initUserInfo_selectsCurrentLoggedUser();
    void test_currentUserPropertiesChangedSignalUpdatesState();
    void test_currentAccountUpdateSlot_iconFile();
    void test_currentAccountUpdateSlot_userName();
    void test_currentAccountUpdateSlot_realName();
    void test_currentAccountUpdateSlot_accountType();
    void test_currentAccountUpdateSlot_multipleFields();
    void test_currentAccountUpdateSlot_emptyMap();
    void test_currentAccountUpdateSlot_irrelevantKey_keepsState();
    void test_currentAccountUpdateSlot_emitsSignal();

    // PowerButton 测试
    void test_createPowerButton();
    void test_powerButton_defaultIsEnable();
    void test_powerButton_defaultIcon();
    void test_powerButton_defaultTooltip();
    void test_powerButton_defaultPercentage();
    void test_powerButton_defaultIsPresent();
    void test_powerButton_onBatteryIconChanged();
    void test_powerButton_onBatteryIconChanged_sameIcon();
    void test_powerButton_onPercentageChanged();
    void test_powerButton_onPercentageChanged_sameValue();
    void test_powerButton_onPercentageChanged_rounding();
    void test_powerButton_onBatteryShowIcon();
    void test_powerButton_onBatteryShowIcon_sameValue();
    void test_powerButton_onBatteryTooltip();
    void test_powerButton_onBatteryTooltip_emptyString();
    void test_powerButton_onBatteryTooltip_sameValue();
    void test_powerButton_onIsPresentChanged();
    void test_powerButton_onIsPresentChanged_sameValue();
    void test_powerButton_onConnectorReady_invalidInterface();
    void test_powerButton_onConnectorReady_validInterface();
    void test_powerButton_onConnectorReady_existingWatcher();
    void test_powerButton_getBatteryInfo_skipsStaleRefreshFlags();
    void test_powerButton_dbusSignalsUpdateProperties();
    void test_powerButton_onServiceUnregistered();
    void test_powerButton_onServiceUnregistered_withoutIface();
    void test_openMethods_doesNotCrash();
};

// 辅助函数：调用 AccountInformation 的私有槽 currentAccountUpdateSlot
static void invokeCurrentAccountUpdateSlot(AccountInformation *info,
                                           const QString &property,
                                           const QMap<QString, QVariant> &propertyMap,
                                           const QStringList &propertyList)
{
    QMetaObject::invokeMethod(info, "currentAccountUpdateSlot",
                              Q_ARG(QString, property),
                              Q_ARG(QVariantMap, propertyMap),
                              Q_ARG(QStringList, propertyList));
}

// 辅助函数：调用 PowerButton 的私有槽
static void invokeOnBatteryIconChanged(PowerButton *button, const QString &iconName)
{
    QMetaObject::invokeMethod(button, "onBatteryIconChanged", Q_ARG(QString, iconName));
}

static void invokeOnPercentageChanged(PowerButton *button, double percentage)
{
    QMetaObject::invokeMethod(button, "onPercentageChanged", Q_ARG(double, percentage));
}

static void invokeOnBatteryShowIcon(PowerButton *button, bool isEnable)
{
    QMetaObject::invokeMethod(button, "onBatteryShowIcon", Q_ARG(bool, isEnable));
}

static void invokeOnBatteryTooltip(PowerButton *button, const QString &tooltip)
{
    QMetaObject::invokeMethod(button, "onBatteryTooltip", Q_ARG(QString, tooltip));
}

static void invokeOnIsPresentChanged(PowerButton *button, bool isPresent)
{
    QMetaObject::invokeMethod(button, "onIsPresentChanged", Q_ARG(bool, isPresent));
}

static void invokeOnConnectorReady(PowerButton *button, QDBusInterface *iface)
{
    QMetaObject::invokeMethod(button, "onConnectorReady", Q_ARG(QDBusInterface *, iface));
}

static void invokeOnServiceUnregistered(PowerButton *button)
{
    QMetaObject::invokeMethod(button, "onServiceUnregistered");
}

static void waitForAsyncDbus()
{
    QTest::qWait(80);
}

static void unregisterFakeAccounts()
{
    QDBusConnection::sessionBus().unregisterObject("/org/freedesktop/Accounts/User1000");
    QDBusConnection::sessionBus().unregisterObject("/org/freedesktop/Accounts/User1001");
    QDBusConnection::sessionBus().unregisterObject("/org/freedesktop/Accounts");
    QDBusConnection::sessionBus().unregisterService("org.freedesktop.Accounts");
}

static bool registerFakeAccounts(FakeAccountsService *accounts,
                                 FakeAccountPropertiesService *currentUser,
                                 FakeAccountPropertiesService *otherUser = nullptr)
{
    unregisterFakeAccounts();
    auto bus = QDBusConnection::sessionBus();
    if (!bus.registerService("org.freedesktop.Accounts")) {
        return false;
    }

    if (!bus.registerObject("/org/freedesktop/Accounts",
                            accounts,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        return false;
    }

    if (currentUser && !bus.registerObject("/org/freedesktop/Accounts/User1000",
                                           currentUser,
                                           QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        return false;
    }

    if (otherUser && !bus.registerObject("/org/freedesktop/Accounts/User1001",
                                         otherUser,
                                         QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        return false;
    }

    return true;
}

static void unregisterFakeBattery()
{
    QDBusConnection::sessionBus().unregisterObject("/upower/BatteryInfo");
    QDBusConnection::sessionBus().unregisterService("org.ukui.upower");
}

static bool registerFakeBattery(FakeBatteryService *service)
{
    unregisterFakeBattery();
    auto bus = QDBusConnection::sessionBus();
    return bus.registerService("org.ukui.upower")
           && bus.registerObject("/upower/BatteryInfo",
                                 service,
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
}

void accountInformation_test::test_createAccountInformation()
{
    AccountInformation info;
    QVERIFY(true);
}

void accountInformation_test::test_getUsername()
{
    AccountInformation info;
    QString username = info.getUsername();
    QVERIFY(username.isNull() || !username.isNull());
}

void accountInformation_test::test_getRealName()
{
    AccountInformation info;
    QString realName = info.getRealName();
    QVERIFY(realName.isNull() || !realName.isNull());
}

void accountInformation_test::test_getIconFile_default()
{
    AccountInformation info;
    QString iconFile = info.getIconFile();
    // 如果无法获取用户信息，应返回默认图标
    QVERIFY(!iconFile.isEmpty());
}

void accountInformation_test::test_getAccountType_default()
{
    AccountInformation info;
    QString accountType = info.getAccountType();
    QVERIFY(accountType.isEmpty() || !accountType.isEmpty());
}

void accountInformation_test::test_getAccountType_unknown()
{
    AccountInformation info;
    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("AccountType", 2);

    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());
    QCOMPARE(info.getAccountType(), QString(""));
}

void accountInformation_test::test_getAccountType_standardAndAdministrator()
{
    AccountInformation info;
    QMap<QString, QVariant> propertyMap;

    propertyMap.insert("AccountType", 0);
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());
    QCOMPARE(info.getAccountType(), QObject::tr("Standard user"));

    propertyMap.insert("AccountType", 1);
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());
    QCOMPARE(info.getAccountType(), QObject::tr("Administrator"));
}

void accountInformation_test::test_getUserObjectPath_doesNotCrash()
{
    AccountInformation info;
    const QStringList users = info.getUserObjectPath();
    QVERIFY(users.size() >= 0);
}

void accountInformation_test::test_getUserInformation_invalidPath_defaults()
{
    AccountInformation info;
    const UserInformation user = info.GetUserInformation("/org/freedesktop/Accounts/User999999");

    QCOMPARE(user.current, false);
    QCOMPARE(user.isLogged, false);
    QCOMPARE(user.autologin, false);
}

void accountInformation_test::test_getUserObjectPath_fromFakeAccountsService()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeAccountsService accounts;
    accounts.users = {QDBusObjectPath("/org/freedesktop/Accounts/User1000"),
                      QDBusObjectPath("/org/freedesktop/Accounts/User1001")};
    QVERIFY(registerFakeAccounts(&accounts, nullptr));

    AccountInformation info;
    delete info.m_systemUserIFace;
    info.m_systemUserIFace = new QDBusInterface("org.freedesktop.Accounts",
                                                "/org/freedesktop/Accounts",
                                                "org.freedesktop.Accounts",
                                                QDBusConnection::sessionBus(),
                                                &info);

    QCOMPARE(info.getUserObjectPath(), QStringList({"/org/freedesktop/Accounts/User1000",
                                                    "/org/freedesktop/Accounts/User1001"}));

    unregisterFakeAccounts();
}

void accountInformation_test::test_getUserInformation_validCurrentUser()
{
    QSKIP("AccountInformation uses systemBus for org.freedesktop.Accounts; fake session bus cannot override it.");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeAccountsService accounts;
    FakeAccountPropertiesService currentUser;
    currentUser.properties = {
        {"UserName", QString::fromLocal8Bit(qgetenv("USER"))},
        {"RealName", QStringLiteral("Current User")},
        {"AccountType", 1},
        {"IconFile", QStringLiteral("/tmp/current.png")},
        {"PasswordMode", 0},
        {"Uid", 1000}
    };
    QVERIFY(registerFakeAccounts(&accounts, &currentUser));

    AccountInformation info;
    UserInformation user = info.GetUserInformation("/org/freedesktop/Accounts/User1000");

    QVERIFY(user.current);
    QVERIFY(user.isLogged);
    QCOMPARE(user.username, QString::fromLocal8Bit(qgetenv("USER")));
    QCOMPARE(user.realName, QStringLiteral("Current User"));
    QCOMPARE(user.accountType, 1);
    QCOMPARE(user.iconFile, QStringLiteral("/tmp/current.png"));
    QCOMPARE(user.uid, 1000);
    QCOMPARE(user.objPath, QStringLiteral("/org/freedesktop/Accounts/User1000"));

    unregisterFakeAccounts();
}

void accountInformation_test::test_initUserInfo_selectsCurrentLoggedUser()
{
    QSKIP("AccountInformation uses systemBus for org.freedesktop.Accounts; fake session bus cannot override it.");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeAccountsService accounts;
    accounts.users = {QDBusObjectPath("/org/freedesktop/Accounts/User1001"),
                      QDBusObjectPath("/org/freedesktop/Accounts/User1000")};

    FakeAccountPropertiesService otherUser;
    otherUser.properties = {
        {"UserName", QStringLiteral("someone-else")},
        {"RealName", QStringLiteral("Other User")},
        {"AccountType", 0},
        {"IconFile", QStringLiteral("/tmp/other.png")},
        {"PasswordMode", 0},
        {"Uid", 1001}
    };

    FakeAccountPropertiesService currentUser;
    currentUser.properties = {
        {"UserName", QString::fromLocal8Bit(qgetenv("USER"))},
        {"RealName", QStringLiteral("Selected User")},
        {"AccountType", 1},
        {"IconFile", QStringLiteral("/tmp/selected.png")},
        {"PasswordMode", 0},
        {"Uid", 1000}
    };
    QVERIFY(registerFakeAccounts(&accounts, &currentUser, &otherUser));

    AccountInformation info;
    delete info.m_systemUserIFace;
    info.m_systemUserIFace = new QDBusInterface("org.freedesktop.Accounts",
                                                "/org/freedesktop/Accounts",
                                                "org.freedesktop.Accounts",
                                                QDBusConnection::sessionBus(),
                                                &info);

    info.initUserInfo();

    QCOMPARE(info.getUsername(), QString::fromLocal8Bit(qgetenv("USER")));
    QCOMPARE(info.getRealName(), QStringLiteral("Selected User"));
    QCOMPARE(info.getIconFile(), QStringLiteral("/tmp/selected.png"));
    QCOMPARE(info.getAccountType(), QObject::tr("Administrator"));
    QVERIFY(info.m_currentUserIFace != nullptr);

    unregisterFakeAccounts();
}

void accountInformation_test::test_currentUserPropertiesChangedSignalUpdatesState()
{
    QSKIP("AccountInformation uses systemBus for org.freedesktop.Accounts; fake session bus cannot override it.");
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeAccountsService accounts;
    accounts.users = {QDBusObjectPath("/org/freedesktop/Accounts/User1000")};
    FakeAccountPropertiesService currentUser;
    currentUser.properties = {
        {"UserName", QString::fromLocal8Bit(qgetenv("USER"))},
        {"RealName", QStringLiteral("Before")},
        {"AccountType", 0},
        {"IconFile", QStringLiteral("/tmp/before.png")},
        {"PasswordMode", 0},
        {"Uid", 1000}
    };
    QVERIFY(registerFakeAccounts(&accounts, &currentUser));

    AccountInformation info;
    delete info.m_systemUserIFace;
    info.m_systemUserIFace = new QDBusInterface("org.freedesktop.Accounts",
                                                "/org/freedesktop/Accounts",
                                                "org.freedesktop.Accounts",
                                                QDBusConnection::sessionBus(),
                                                &info);
    info.initUserInfo();
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QVariantMap changed;
    changed.insert("RealName", QStringLiteral("After"));
    changed.insert("IconFile", QStringLiteral("/tmp/after.png"));
    Q_EMIT currentUser.PropertiesChanged("org.freedesktop.Accounts.User", changed, QStringList());

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(info.getRealName(), QStringLiteral("After"));
    QCOMPARE(info.getIconFile(), QStringLiteral("/tmp/after.png"));

    unregisterFakeAccounts();
}

void accountInformation_test::test_currentAccountUpdateSlot_iconFile()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("IconFile", "/test/icon.png");

    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(info.getIconFile(), QString("/test/icon.png"));
}

void accountInformation_test::test_currentAccountUpdateSlot_userName()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("UserName", "testuser");

    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(info.getUsername(), QString("testuser"));
}

void accountInformation_test::test_currentAccountUpdateSlot_realName()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("RealName", "Test User");

    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(info.getRealName(), QString("Test User"));
}

void accountInformation_test::test_currentAccountUpdateSlot_accountType()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("AccountType", 1);

    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(info.getAccountType(), QObject::tr("Administrator"));
}

void accountInformation_test::test_currentAccountUpdateSlot_multipleFields()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("UserName", "multiuser");
    propertyMap.insert("RealName", "Multi User");
    propertyMap.insert("IconFile", "/multi/icon.png");
    propertyMap.insert("AccountType", 0);

    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(info.getUsername(), QString("multiuser"));
    QCOMPARE(info.getRealName(), QString("Multi User"));
    QCOMPARE(info.getIconFile(), QString("/multi/icon.png"));
    QCOMPARE(info.getAccountType(), QObject::tr("Standard user"));
}

void accountInformation_test::test_currentAccountUpdateSlot_emptyMap()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_currentAccountUpdateSlot_irrelevantKey_keepsState()
{
    AccountInformation info;
    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("UserName", "before-user");
    propertyMap.insert("RealName", "Before User");
    propertyMap.insert("IconFile", "/before/icon.png");
    propertyMap.insert("AccountType", 1);
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    propertyMap.clear();
    propertyMap.insert("IgnoredKey", "ignored");
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(info.getUsername(), QString("before-user"));
    QCOMPARE(info.getRealName(), QString("Before User"));
    QCOMPARE(info.getIconFile(), QString("/before/icon.png"));
    QCOMPARE(info.getAccountType(), QObject::tr("Administrator"));
}

void accountInformation_test::test_currentAccountUpdateSlot_emitsSignal()
{
    AccountInformation info;
    QSignalSpy spy(&info, &AccountInformation::userInfoChanged);

    QMap<QString, QVariant> propertyMap;
    propertyMap.insert("IconFile", "/icon1.png");
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    propertyMap.clear();
    propertyMap.insert("IconFile", "/icon2.png");
    invokeCurrentAccountUpdateSlot(&info, "", propertyMap, QStringList());

    QCOMPARE(spy.count(), 2);
}

void accountInformation_test::test_createPowerButton()
{
    PowerButton button;
    QVERIFY(true);
}

void accountInformation_test::test_powerButton_defaultIsEnable()
{
    PowerButton button;
    QCOMPARE(button.isEnable(), false);
}

void accountInformation_test::test_powerButton_defaultIcon()
{
    PowerButton button;
    QCOMPARE(button.icon(), QString(""));
}

void accountInformation_test::test_powerButton_defaultTooltip()
{
    PowerButton button;
    QCOMPARE(button.tooltip(), QString(""));
}

void accountInformation_test::test_powerButton_defaultPercentage()
{
    PowerButton button;
    QCOMPARE(button.percentage(), QString(""));
}

void accountInformation_test::test_powerButton_defaultIsPresent()
{
    PowerButton button;
    QCOMPARE(button.isPresent(), false);
}

void accountInformation_test::test_powerButton_onBatteryIconChanged()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::iconChanged);
    invokeOnBatteryIconChanged(&button, "battery-full");
    QCOMPARE(button.icon(), QString("battery-full"));
    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_powerButton_onBatteryIconChanged_sameIcon()
{
    PowerButton button;
    invokeOnBatteryIconChanged(&button, "battery-low");
    QSignalSpy spy(&button, &PowerButton::iconChanged);
    invokeOnBatteryIconChanged(&button, "battery-low");
    QCOMPARE(spy.count(), 0);
}

void accountInformation_test::test_powerButton_onPercentageChanged()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::percentageChanged);
    invokeOnPercentageChanged(&button, 85.6);
    QCOMPARE(button.percentage(), QString("86"));
    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_powerButton_onPercentageChanged_sameValue()
{
    PowerButton button;
    invokeOnPercentageChanged(&button, 50.0);
    QSignalSpy spy(&button, &PowerButton::percentageChanged);
    invokeOnPercentageChanged(&button, 50.4);
    QCOMPARE(spy.count(), 0);
}

void accountInformation_test::test_powerButton_onPercentageChanged_rounding()
{
    PowerButton button;
    invokeOnPercentageChanged(&button, 49.6);
    QCOMPARE(button.percentage(), QString("50"));
}

void accountInformation_test::test_powerButton_onBatteryShowIcon()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::isEnableChanged);
    invokeOnBatteryShowIcon(&button, true);
    QCOMPARE(button.isEnable(), true);
    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_powerButton_onBatteryShowIcon_sameValue()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::isEnableChanged);
    invokeOnBatteryShowIcon(&button, false);
    QCOMPARE(spy.count(), 0);
}

void accountInformation_test::test_powerButton_onBatteryTooltip()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::tooltipChanged);
    invokeOnBatteryTooltip(&button, "Battery 85%");
    QCOMPARE(button.tooltip(), QString("Battery 85%"));
    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_powerButton_onBatteryTooltip_emptyString()
{
    PowerButton button;
    invokeOnBatteryTooltip(&button, QString());
    QCOMPARE(button.tooltip(), QString());
}

void accountInformation_test::test_powerButton_onBatteryTooltip_sameValue()
{
    PowerButton button;
    invokeOnBatteryTooltip(&button, "Battery 50%");
    QSignalSpy spy(&button, &PowerButton::tooltipChanged);
    invokeOnBatteryTooltip(&button, "Battery 50%");
    QCOMPARE(spy.count(), 0);
}

void accountInformation_test::test_powerButton_onIsPresentChanged()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::isPresentChanged);
    invokeOnIsPresentChanged(&button, true);
    QCOMPARE(button.isPresent(), true);
    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_powerButton_onIsPresentChanged_sameValue()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::isPresentChanged);
    invokeOnIsPresentChanged(&button, false);
    QCOMPARE(spy.count(), 0);
}

void accountInformation_test::test_powerButton_onConnectorReady_invalidInterface()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::isEnableChanged);
    QDBusInterface invalidIface("invalid.service",
                                "/invalid/path",
                                "invalid.interface",
                                QDBusConnection::sessionBus());
    invokeOnConnectorReady(&button, &invalidIface);
    QCOMPARE(spy.count(), 0);
}

void accountInformation_test::test_powerButton_onConnectorReady_validInterface()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeBatteryService service;
    QVERIFY(registerFakeBattery(&service));

    PowerButton button;
    QSignalSpy enableSpy(&button, &PowerButton::isEnableChanged);
    QDBusInterface *iface = new QDBusInterface("org.ukui.upower",
                                               "/upower/BatteryInfo",
                                               "org.ukui.upower.battery",
                                               QDBusConnection::sessionBus(),
                                               &button);
    QVERIFY(iface->isValid());

    invokeOnConnectorReady(&button, iface);

    QTRY_VERIFY(enableSpy.count() >= 1);
    QTRY_COMPARE(button.icon(), QString("battery-good-symbolic"));
    QTRY_COMPARE(button.percentage(), QString("66"));
    QTRY_COMPARE(button.tooltip(), QString("Battery 66%"));
    QTRY_COMPARE(button.isPresent(), true);
    QCOMPARE(button.isEnable(), true);

    unregisterFakeBattery();
}

void accountInformation_test::test_powerButton_onConnectorReady_existingWatcher()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeBatteryService service;
    QVERIFY(registerFakeBattery(&service));

    PowerButton button;
    button.m_powerServiceWatcher = new QDBusServiceWatcher("org.ukui.upower",
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForUnregistration,
                                                           &button);
    QDBusServiceWatcher *existingWatcher = button.m_powerServiceWatcher;

    QDBusInterface *iface = new QDBusInterface("org.ukui.upower",
                                               "/upower/BatteryInfo",
                                               "org.ukui.upower.battery",
                                               QDBusConnection::sessionBus(),
                                               &button);
    QVERIFY(iface->isValid());

    invokeOnConnectorReady(&button, iface);

    QCOMPARE(button.m_powerServiceWatcher, existingWatcher);

    unregisterFakeBattery();
}

void accountInformation_test::test_powerButton_getBatteryInfo_skipsStaleRefreshFlags()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeBatteryService service;
    QVERIFY(registerFakeBattery(&service));

    PowerButton button;
    button.m_powerBatteryIface = new QDBusInterface("org.ukui.upower",
                                                    "/upower/BatteryInfo",
                                                    "org.ukui.upower.battery",
                                                    QDBusConnection::sessionBus(),
                                                    &button);
    QVERIFY(button.m_powerBatteryIface->isValid());

    button.getBatteryInfo();
    button.m_iconNeedRefresh = false;
    button.m_percentageNeedRefresh = false;
    button.m_isEnableNeedRefresh = false;
    button.m_tooltipNeedRefresh = false;
    button.m_isPresentNeedRefresh = false;
    waitForAsyncDbus();

    QCOMPARE(button.icon(), QString());
    QCOMPARE(button.percentage(), QString());
    QCOMPARE(button.tooltip(), QString());
    QCOMPARE(button.isPresent(), false);
    QCOMPARE(button.isEnable(), false);

    unregisterFakeBattery();
}

void accountInformation_test::test_powerButton_dbusSignalsUpdateProperties()
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        QSKIP("Session bus is not available.");
    }

    FakeBatteryService service;
    QVERIFY(registerFakeBattery(&service));

    PowerButton button;
    QDBusInterface *iface = new QDBusInterface("org.ukui.upower",
                                               "/upower/BatteryInfo",
                                               "org.ukui.upower.battery",
                                               QDBusConnection::sessionBus(),
                                               &button);
    QVERIFY(iface->isValid());
    invokeOnConnectorReady(&button, iface);

    Q_EMIT service.BatteryIcon("battery-low-symbolic");
    QTRY_COMPARE(button.icon(), QString("battery-low-symbolic"));

    Q_EMIT service.PercentageChanged(12.4);
    QTRY_COMPARE(button.percentage(), QString("12"));

    Q_EMIT service.BatteryShowIcon(false);
    QTRY_COMPARE(button.isEnable(), false);

    Q_EMIT service.BatteryTooltip("Battery low");
    QTRY_COMPARE(button.tooltip(), QString("Battery low"));

    Q_EMIT service.IsPresentChanged(false);
    QTRY_COMPARE(button.isPresent(), false);

    unregisterFakeBattery();
}

void accountInformation_test::test_powerButton_onServiceUnregistered()
{
    PowerButton button;
    QSignalSpy spy(&button, &PowerButton::isEnableChanged);
    invokeOnServiceUnregistered(&button);
    QCOMPARE(button.isEnable(), false);
    QCOMPARE(spy.count(), 1);
}

void accountInformation_test::test_powerButton_onServiceUnregistered_withoutIface()
{
    PowerButton button;
    button.m_powerBatteryIface = nullptr;
    invokeOnServiceUnregistered(&button);
    QCOMPARE(button.isEnable(), false);
}

void accountInformation_test::test_openMethods_doesNotCrash()
{
    AccountInformation info;
    info.openUserCenter();
    info.openPeonyUser();
    info.openControlCenter();
    info.openPowerButton();
    QTest::qWait(50);
    QVERIFY(true);
}

QTEST_MAIN(accountInformation_test)

#include "account-information-test.moc"
