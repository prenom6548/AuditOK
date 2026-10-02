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
#include <QAction>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QMenu>
#include <QVariant>
#include <QtQml/qqml.h>

class FakeMenu : public QMenu
{
public:
    explicit FakeMenu(QObject *parent = nullptr)
        : QMenu(nullptr)
    {
        Q_UNUSED(parent);
        s_lastCreated = this;
    }

    ~FakeMenu() override
    {
        if (s_lastCreated == this) {
            s_lastCreated = nullptr;
        }
    }

    void popup(const QPoint &point)
    {
        m_popupCalled = true;
        m_popupPoint = point;
    }

    void close()
    {
        m_closed = true;
        QMenu::close();
    }

    QSize sizeHint() const override
    {
        return QSize(40, 30);
    }

    bool popupCalled() const { return m_popupCalled; }
    QPoint popupPoint() const { return m_popupPoint; }
    bool closed() const { return m_closed; }

    static FakeMenu *lastCreated() { return s_lastCreated; }
    static void resetLastCreated() { s_lastCreated = nullptr; }

private:
    bool m_popupCalled = false;
    bool m_closed = false;
    QPoint m_popupPoint;

    static FakeMenu *s_lastCreated;
};

FakeMenu *FakeMenu::s_lastCreated = nullptr;

namespace {

QString g_powerSoleOperation;
QString g_lastBinaryApp;
QString g_lastBinaryArgs;
int g_launchBinaryCalls = 0;

bool g_sessionManagerValid = false;
bool g_accountsValid = false;
bool g_displayManagerValid = false;
bool g_upgradeInterfaceValid = false;

bool g_canLockscreen = false;
bool g_canSuspend = false;
bool g_canHibernate = false;
bool g_canPowerOff = false;
bool g_canReboot = false;
bool g_hasMultipleUsers = false;
bool g_canSwitch = false;

QDBusMessage g_checkInstallRequiredReply;
QDBusMessage g_checkHideUpdateReply;
QString g_lastAsyncMethod;
QVariantList g_lastAsyncArgs;

QDBusMessage makeBoolReply(bool value)
{
    const QDBusMessage request = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                QStringLiteral("/path"),
                                                                QStringLiteral("iface"),
                                                                QStringLiteral("method"));
    return request.createReply(QVariantList{value});
}

QDBusMessage makeVariantReply(bool value)
{
    const QDBusMessage request = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                QStringLiteral("/path"),
                                                                QStringLiteral("iface"),
                                                                QStringLiteral("method"));
    return request.createReply(QVariantList{QVariant::fromValue(QDBusVariant(QVariant(value)))});
}

void resetDbusState()
{
    g_sessionManagerValid = false;
    g_accountsValid = false;
    g_displayManagerValid = false;
    g_upgradeInterfaceValid = false;

    g_canLockscreen = false;
    g_canSuspend = false;
    g_canHibernate = false;
    g_canPowerOff = false;
    g_canReboot = false;
    g_hasMultipleUsers = false;
    g_canSwitch = false;

    g_checkInstallRequiredReply = QDBusMessage();
    g_checkHideUpdateReply = QDBusMessage();
    g_lastAsyncMethod.clear();
    g_lastAsyncArgs.clear();
}

} // namespace

class TestFakeDBusConnection
{
public:
    static TestFakeDBusConnection sessionBus()
    {
        return {};
    }

    static TestFakeDBusConnection systemBus()
    {
        return {};
    }

    QDBusMessage call(const QDBusMessage &message) const
    {
        if (message.member() == QStringLiteral("CheckInstallRequired")) {
            return g_checkInstallRequiredReply;
        }
        if (message.member() == QStringLiteral("CheckHideUpdate")) {
            return g_checkHideUpdateReply;
        }
        return QDBusMessage();
    }

    QDBusError lastError() const
    {
        return QDBusError(QDBusError::Failed, QStringLiteral("fake-dbus-error"));
    }
};

class TestFakeDBusInterface
{
public:
    TestFakeDBusInterface(const QString &service, const QString &path, const QString &interface, const TestFakeDBusConnection &connection)
        : m_service(service)
        , m_path(path)
        , m_interface(interface)
    {
        Q_UNUSED(connection);
    }

    bool isValid() const
    {
        if (m_service == QStringLiteral("org.gnome.SessionManager")) {
            return g_sessionManagerValid;
        }
        if (m_service == QStringLiteral("org.freedesktop.Accounts")) {
            return g_accountsValid;
        }
        if (m_service == QStringLiteral("org.freedesktop.DisplayManager")) {
            return g_displayManagerValid;
        }
        if (m_service == QStringLiteral("com.kylin.systemupgrade")) {
            return g_upgradeInterfaceValid;
        }
        return false;
    }

    QDBusMessage call(const QString &method) const
    {
        if (!isValid()) {
            return QDBusMessage();
        }

        if (m_service == QStringLiteral("org.gnome.SessionManager")) {
            if (method == QStringLiteral("canLockscreen")) {
                return makeBoolReply(g_canLockscreen);
            }
            if (method == QStringLiteral("canSuspend")) {
                return makeBoolReply(g_canSuspend);
            }
            if (method == QStringLiteral("canHibernate")) {
                return makeBoolReply(g_canHibernate);
            }
            if (method == QStringLiteral("canPowerOff")) {
                return makeBoolReply(g_canPowerOff);
            }
            if (method == QStringLiteral("canReboot")) {
                return makeBoolReply(g_canReboot);
            }
        }

        return QDBusMessage();
    }

    QDBusMessage call(const QString &method, const QString &arg1, const QString &arg2) const
    {
        Q_UNUSED(arg1);
        Q_UNUSED(arg2);

        if (!isValid() || method != QStringLiteral("Get")) {
            return QDBusMessage();
        }

        if (m_service == QStringLiteral("org.freedesktop.Accounts")) {
            return makeVariantReply(g_hasMultipleUsers);
        }
        if (m_service == QStringLiteral("org.freedesktop.DisplayManager")) {
            return makeVariantReply(g_canSwitch);
        }
        return QDBusMessage();
    }

    QDBusMessage asyncCall(const QString &method, const QString &arg) const
    {
        g_lastAsyncMethod = method;
        g_lastAsyncArgs = {arg};
        const QDBusMessage request = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                    QStringLiteral("/path"),
                                                                    QStringLiteral("iface"),
                                                                    method);
        return request.createReply(QVariantList{arg});
    }

private:
    QString m_service;
    QString m_path;
    QString m_interface;
};

#define QMenu FakeMenu
#define private public
#include "../../../src/utils/power-button.h"

#include "fakes/app-manager.h"
#include "fakes/security-function-control.h"

namespace UkuiMenu {

AppManager *AppManager::instance()
{
    static AppManager instance;
    return &instance;
}

void AppManager::launchApp(const QString &appid)
{
    Q_UNUSED(appid);
}

void AppManager::launchBinaryApp(const QString &app, const QString &args)
{
    g_lastBinaryApp = app;
    g_lastBinaryArgs = args;
    ++g_launchBinaryCalls;
}

void AppManager::launchAppWithArguments(const QString &appid, const QStringList &args, const QString &applicationName)
{
    Q_UNUSED(appid);
    Q_UNUSED(args);
    Q_UNUSED(applicationName);
}

void AppManager::changeFavoriteState(const QString &appid, bool isFavorite)
{
    Q_UNUSED(appid);
    Q_UNUSED(isFavorite);
}

QStringList AppManager::parseDesktopFile(const QString &desktopFilePath)
{
    Q_UNUSED(desktopFilePath);
    return {};
}

SecurityFunctionControl *SecurityFunctionControl::instance()
{
    static SecurityFunctionControl instance;
    return &instance;
}

bool SecurityFunctionControl::disable() const { return false; }
bool SecurityFunctionControl::disableMouseLeftButton() const { return false; }
bool SecurityFunctionControl::disableMouseRightButton() const { return false; }
bool SecurityFunctionControl::disableMouseHover() const { return false; }
QStringList SecurityFunctionControl::appWhiteList() const { return {}; }
QStringList SecurityFunctionControl::appBlackList() const { return {}; }
bool SecurityFunctionControl::disableUninstallAction() const { return false; }
bool SecurityFunctionControl::windowMaximize() const { return true; }
bool SecurityFunctionControl::showFoldButton() const { return true; }
QString SecurityFunctionControl::powerButtonSoleOperation() const { return g_powerSoleOperation; }
bool SecurityFunctionControl::compareListBySort(QStringList &list1, QStringList &list2) { return list1 == list2; }
void SecurityFunctionControl::onFunctionControlChanged(const QString &name, bool exit) { Q_UNUSED(name); Q_UNUSED(exit); }
bool SecurityFunctionControl::canAppDisplay(const QString &desktop) const { Q_UNUSED(desktop); return true; }

} // namespace UkuiMenu

#define UKUI_MENU_APP_MANAGER_H
#define SECURITYFUNCTIONCONTROL_H
#define QDBusInterface TestFakeDBusInterface
#define QDBusConnection TestFakeDBusConnection
#include "../../../src/utils/power-button.cpp"
#undef private
#undef QDBusConnection
#undef QDBusInterface
#undef QMenu

using namespace UkuiMenu;

class powerButton_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_defineModule_registersWithoutCrash();
    void test_basicMetadata();
    void test_minutesToHour_formatsExpectedText();
    void test_functionControlOperation_executesMappedActions();
    void test_functionControlOperation_acceptsUpgradeActions();
    void test_functionControlOperation_rejectsUnknownOperation();
    void test_privateDbusHelpers_failClosedWithoutServices();
    void test_privateDbusHelpers_successPaths();
    void test_clicked_leftButtonFallsBackToSessionTools();
    void test_clicked_usesFunctionControlOperationWhenConfigured();
    void test_clicked_rightButton_withoutMenuLeavesContextMenuNull();
    void test_openMenu_closesExistingMenu();
    void test_openMenu_createsUpgradeOnlyMenu();
    void test_openMenu_createsMixedMenuInFullScreen();
    void test_openMenu_createsNormalActionsAndTriggersThem();
    void test_openMenu_upgradeActionsTriggerDbus();
    void test_canUpgrade_handlesMalformedReply();
    void test_checkHideUpdate_emptyReplyReturnsFalse();
};

void powerButton_test::init()
{
    g_powerSoleOperation.clear();
    g_lastBinaryApp.clear();
    g_lastBinaryArgs.clear();
    g_launchBinaryCalls = 0;
    FakeMenu::resetLastCreated();
    resetDbusState();
}

void powerButton_test::test_defineModule_registersWithoutCrash()
{
    PowerButton::defineModule("org.ukui.menu.utils", 1, 0);
    QVERIFY(qmlTypeId("org.ukui.menu.utils", 1, 0, "PowerButton") >= 0);
}

void powerButton_test::test_basicMetadata()
{
    PowerButton button;

    QCOMPARE(button.getIcon(), QStringLiteral("exit-symbolic"));
    QCOMPARE(button.getToolTip(), QStringLiteral("Power"));
}

void powerButton_test::test_minutesToHour_formatsExpectedText()
{
    PowerButton button;

    QVERIFY(button.minutesToHour(125).contains(QStringLiteral("2 hour 5 minutes")));
    QVERIFY(button.minutesToHour(60).contains(QStringLiteral("60 minutes")));
    QVERIFY(button.minutesToHour(15).contains(QStringLiteral("15 minutes")));
    QCOMPARE(button.minutesToHour(0), QString());
}

void powerButton_test::test_functionControlOperation_executesMappedActions()
{
    PowerButton button;

    QVERIFY(button.functionControlOperation(QStringLiteral("lockScreen")));
    QCOMPARE(g_lastBinaryApp, QStringLiteral("ukui-screensaver-command"));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("-l"));

    QVERIFY(button.functionControlOperation(QStringLiteral("suspend")));
    QCOMPARE(g_lastBinaryApp, QStringLiteral("ukui-session-tools"));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--suspend"));

    QVERIFY(button.functionControlOperation(QStringLiteral("hibernate")));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--hibernate"));

    QVERIFY(button.functionControlOperation(QStringLiteral("shutdown")));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--shutdown"));

    QVERIFY(button.functionControlOperation(QStringLiteral("restart")));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--reboot"));
}

void powerButton_test::test_functionControlOperation_acceptsUpgradeActions()
{
    PowerButton button;
    g_upgradeInterfaceValid = true;

    QVERIFY(button.functionControlOperation(QStringLiteral("upgradeAndShutdown")));
    QCOMPARE(g_launchBinaryCalls, 0);
    QCOMPARE(g_lastAsyncMethod, QStringLiteral("TriggerInstallOnShutdown"));
    QCOMPARE(g_lastAsyncArgs, QVariantList({QStringLiteral("shutdown")}));

    g_lastAsyncMethod.clear();
    g_lastAsyncArgs.clear();

    QVERIFY(button.functionControlOperation(QStringLiteral("upgradeAndRestart")));
    QCOMPARE(g_launchBinaryCalls, 0);
    QCOMPARE(g_lastAsyncMethod, QStringLiteral("TriggerInstallOnShutdown"));
    QCOMPARE(g_lastAsyncArgs, QVariantList({QStringLiteral("reboot")}));
}

void powerButton_test::test_functionControlOperation_rejectsUnknownOperation()
{
    PowerButton button;

    QVERIFY(!button.functionControlOperation(QStringLiteral("unknown")));
    QCOMPARE(g_launchBinaryCalls, 0);
}

void powerButton_test::test_privateDbusHelpers_failClosedWithoutServices()
{
    PowerButton button;

    QCOMPARE(button.hasMultipleUsers(), false);
    QCOMPARE(button.canSwitch(), false);
    QCOMPARE(button.canUpgrade(), false);
    QCOMPARE(button.checkHideUpdate(), false);

    button.execUpgradeAction(QStringLiteral("shutdown"));
    QCOMPARE(g_lastAsyncMethod, QString());
    QCOMPARE(g_launchBinaryCalls, 0);
}

void powerButton_test::test_privateDbusHelpers_successPaths()
{
    PowerButton button;

    g_accountsValid = true;
    g_hasMultipleUsers = true;
    QCOMPARE(button.hasMultipleUsers(), true);

    g_displayManagerValid = true;
    g_canSwitch = true;
    QCOMPARE(button.canSwitch(), true);

    const QDBusMessage upgradeRequest = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                       QStringLiteral("/path"),
                                                                       QStringLiteral("iface"),
                                                                       QStringLiteral("CheckInstallRequired"));
    g_checkInstallRequiredReply = upgradeRequest.createReply(QVariantList{true, 95});
    QCOMPARE(button.canUpgrade(), true);
    QCOMPARE(button.m_estimatedLeftMinutes, 95);

    g_checkHideUpdateReply = makeBoolReply(true);
    QCOMPARE(button.checkHideUpdate(), true);

    g_upgradeInterfaceValid = true;
    button.execUpgradeAction(QStringLiteral("shutdown"));
    QCOMPARE(g_lastAsyncMethod, QStringLiteral("TriggerInstallOnShutdown"));
    QCOMPARE(g_lastAsyncArgs, QVariantList({QStringLiteral("shutdown")}));
}

void powerButton_test::test_clicked_leftButtonFallsBackToSessionTools()
{
    PowerButton button;

    button.clicked(true, 10, 20, false);

    QCOMPARE(g_lastBinaryApp, QStringLiteral("ukui-session-tools"));
    QCOMPARE(g_lastBinaryArgs, QString());
}

void powerButton_test::test_clicked_usesFunctionControlOperationWhenConfigured()
{
    PowerButton button;
    g_powerSoleOperation = QStringLiteral("shutdown");

    button.clicked(true, 10, 20, false);

    QCOMPARE(g_lastBinaryApp, QStringLiteral("ukui-session-tools"));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--shutdown"));
    QCOMPARE(g_launchBinaryCalls, 1);
}

void powerButton_test::test_clicked_rightButton_withoutMenuLeavesContextMenuNull()
{
    PowerButton button;

    button.clicked(false, 10, 20, false);

    QCOMPARE(button.m_contextMenu.data(), nullptr);
    QCOMPARE(g_launchBinaryCalls, 0);
}

void powerButton_test::test_openMenu_closesExistingMenu()
{
    PowerButton button;
    auto *menu = new FakeMenu;
    button.m_contextMenu = menu;

    button.openMenu(10, 20, false);

    QVERIFY(menu->closed());
    QVERIFY(button.m_contextMenu.data() == nullptr || button.m_contextMenu.data() == menu);
}

void powerButton_test::test_openMenu_createsUpgradeOnlyMenu()
{
    PowerButton button;

    g_sessionManagerValid = true;
    g_canLockscreen = true;
    g_canSuspend = true;
    g_canHibernate = true;
    g_canPowerOff = true;
    g_canReboot = true;

    const QDBusMessage upgradeRequest = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                       QStringLiteral("/path"),
                                                                       QStringLiteral("iface"),
                                                                       QStringLiteral("CheckInstallRequired"));
    g_checkInstallRequiredReply = upgradeRequest.createReply(QVariantList{true, 125});
    g_checkHideUpdateReply = makeBoolReply(true);

    button.openMenu(100, 200, false);

    QVERIFY(button.m_contextMenu != nullptr);
    QCOMPARE(button.m_contextMenu->actions().size(), 5);
    QCOMPARE(button.m_contextMenu->actions().at(0)->text(), QStringLiteral("Lock Screen"));
    QCOMPARE(button.m_contextMenu->actions().at(1)->text(), QStringLiteral("Suspend"));
    QCOMPARE(button.m_contextMenu->actions().at(2)->text(), QStringLiteral("Hibernate"));
    QCOMPARE(button.m_contextMenu->actions().at(3)->text(), QStringLiteral("Upgrade and Shut Down"));
    QCOMPARE(button.m_contextMenu->actions().at(4)->text(), QStringLiteral("Upgrade and Restart"));
    QVERIFY(button.m_contextMenu->actions().at(3)->toolTip().contains(QStringLiteral("2 hour 5 minutes")));
    QVERIFY(button.m_contextMenu->actions().at(4)->toolTip().contains(QStringLiteral("2 hour 5 minutes")));
    auto *menu = static_cast<FakeMenu *>(button.m_contextMenu.data());
    QVERIFY(menu != nullptr);
    QVERIFY(menu->popupCalled());
    QCOMPARE(menu->popupPoint(), QPoint(100, 170));
}

void powerButton_test::test_openMenu_createsMixedMenuInFullScreen()
{
    PowerButton button;

    g_sessionManagerValid = true;
    g_canPowerOff = true;
    g_canReboot = true;

    const QDBusMessage upgradeRequest = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                       QStringLiteral("/path"),
                                                                       QStringLiteral("iface"),
                                                                       QStringLiteral("CheckInstallRequired"));
    g_checkInstallRequiredReply = upgradeRequest.createReply(QVariantList{true, 15});
    g_checkHideUpdateReply = makeBoolReply(false);

    button.clicked(false, 80, 60, true);

    QVERIFY(button.m_contextMenu != nullptr);
    QCOMPARE(button.m_contextMenu->actions().size(), 4);
    QCOMPARE(button.m_contextMenu->actions().at(0)->text(), QStringLiteral("Upgrade and Shut Down"));
    QCOMPARE(button.m_contextMenu->actions().at(1)->text(), QStringLiteral("Shut Down"));
    QCOMPARE(button.m_contextMenu->actions().at(2)->text(), QStringLiteral("Upgrade and Restart"));
    QCOMPARE(button.m_contextMenu->actions().at(3)->text(), QStringLiteral("Restart"));
    QVERIFY(button.m_contextMenu->actions().at(0)->toolTip().contains(QStringLiteral("15 minutes")));
    QVERIFY(button.m_contextMenu->actions().at(2)->toolTip().contains(QStringLiteral("15 minutes")));
    auto *menu = static_cast<FakeMenu *>(button.m_contextMenu.data());
    QVERIFY(menu != nullptr);
    QVERIFY(menu->popupCalled());
    QCOMPARE(menu->popupPoint(), QPoint(40, 30));
}

void powerButton_test::test_openMenu_createsNormalActionsAndTriggersThem()
{
    PowerButton button;

    g_sessionManagerValid = true;
    g_canLockscreen = true;
    g_canSuspend = true;
    g_canHibernate = true;
    g_canPowerOff = true;
    g_canReboot = true;
    g_checkInstallRequiredReply = QDBusMessage();
    g_checkHideUpdateReply = makeBoolReply(false);

    button.openMenu(20, 50, false);

    QVERIFY(button.m_contextMenu != nullptr);
    QCOMPARE(button.m_contextMenu->actions().size(), 5);
    QCOMPARE(button.m_contextMenu->actions().at(0)->text(), QStringLiteral("Lock Screen"));
    QCOMPARE(button.m_contextMenu->actions().at(1)->text(), QStringLiteral("Suspend"));
    QCOMPARE(button.m_contextMenu->actions().at(2)->text(), QStringLiteral("Hibernate"));
    QCOMPARE(button.m_contextMenu->actions().at(3)->text(), QStringLiteral("Shut Down"));
    QCOMPARE(button.m_contextMenu->actions().at(4)->text(), QStringLiteral("Restart"));

    button.m_contextMenu->actions().at(0)->trigger();
    QCOMPARE(g_lastBinaryApp, QStringLiteral("ukui-screensaver-command"));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("-l"));

    button.m_contextMenu->actions().at(1)->trigger();
    QCOMPARE(g_lastBinaryApp, QStringLiteral("ukui-session-tools"));
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--suspend"));

    button.m_contextMenu->actions().at(2)->trigger();
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--hibernate"));

    button.m_contextMenu->actions().at(3)->trigger();
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--shutdown"));

    button.m_contextMenu->actions().at(4)->trigger();
    QCOMPARE(g_lastBinaryArgs, QStringLiteral("--reboot"));
    QCOMPARE(g_launchBinaryCalls, 5);
}

void powerButton_test::test_openMenu_upgradeActionsTriggerDbus()
{
    PowerButton button;

    g_sessionManagerValid = true;
    g_upgradeInterfaceValid = true;
    g_canPowerOff = true;
    g_canReboot = true;

    const QDBusMessage upgradeRequest = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                       QStringLiteral("/path"),
                                                                       QStringLiteral("iface"),
                                                                       QStringLiteral("CheckInstallRequired"));
    g_checkInstallRequiredReply = upgradeRequest.createReply(QVariantList{true, 61});
    g_checkHideUpdateReply = makeBoolReply(true);

    button.openMenu(10, 40, false);

    QVERIFY(button.m_contextMenu != nullptr);
    QCOMPARE(button.m_contextMenu->actions().size(), 2);

    button.m_contextMenu->actions().at(0)->trigger();
    QCOMPARE(g_lastAsyncMethod, QStringLiteral("TriggerInstallOnShutdown"));
    QCOMPARE(g_lastAsyncArgs, QVariantList({QStringLiteral("shutdown")}));

    g_lastAsyncMethod.clear();
    g_lastAsyncArgs.clear();
    button.m_contextMenu->actions().at(1)->trigger();
    QCOMPARE(g_lastAsyncMethod, QStringLiteral("TriggerInstallOnShutdown"));
    QCOMPARE(g_lastAsyncArgs, QVariantList({QStringLiteral("reboot")}));
}

void powerButton_test::test_canUpgrade_handlesMalformedReply()
{
    PowerButton button;
    const QDBusMessage upgradeRequest = QDBusMessage::createMethodCall(QStringLiteral("svc"),
                                                                       QStringLiteral("/path"),
                                                                       QStringLiteral("iface"),
                                                                       QStringLiteral("CheckInstallRequired"));
    g_checkInstallRequiredReply = upgradeRequest.createReply(QVariantList{true});

    QCOMPARE(button.canUpgrade(), false);
    QCOMPARE(button.m_estimatedLeftMinutes, 0);
}

void powerButton_test::test_checkHideUpdate_emptyReplyReturnsFalse()
{
    PowerButton button;

    g_checkHideUpdateReply = QDBusMessage();

    QCOMPARE(button.checkHideUpdate(), false);
}

QTEST_MAIN(powerButton_test)

#include "power-button-test.moc"
