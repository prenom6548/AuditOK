/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include <QProcess>
#include <QQmlEngine>
#include <QMenu>
#include <QDBusReply>
#include <QScopedPointer>

#include "security-function-control.h"
#include "power-button.h"
#include "app-manager.h"

#define KYLIN_POWER_NORMAL_PATH_MAJOR "qrc:///res/icon/pad_mainpower.svg"

using namespace UkuiMenu;

PowerButton::PowerButton(QObject *parent) : QObject(nullptr)
{

}

void PowerButton::defineModule(const char *uri, int versionMajor, int versionMinor)
{
    qmlRegisterType<PowerButton>("org.ukui.menu.utils", versionMajor, versionMinor, "PowerButton");
}

PowerButton::~PowerButton()
= default;

QString PowerButton::getIcon()
{
    return "exit-symbolic";
}

QString PowerButton::getToolTip()
{
    return {tr("Power")};
}

void PowerButton::clicked(bool leftButtonClicked, int mouseX, int mouseY, bool isFullScreen)
{
    if (!functionControlOperation(SecurityFunctionControl::instance()->powerButtonSoleOperation())) {
        if (leftButtonClicked) {
            AppManager::instance()->launchBinaryApp("ukui-session-tools");
        } else {
            openMenu(mouseX, mouseY, isFullScreen);
        }
    }
}

void PowerButton::openMenu(int menuX, int menuY, bool isFullScreen)
{
    if (m_contextMenu) {
        m_contextMenu.data()->close();
        return;
    }

    QScopedPointer<QMenu> powerMenu(new QMenu);
    QDBusReply<bool> reply;
    QDBusInterface qDBusInterface("org.gnome.SessionManager", "/org/gnome/SessionManager",
                                  "org.gnome.SessionManager", QDBusConnection::sessionBus());

    reply = qDBusInterface.call("canLockscreen");
    if (reply.isValid() && reply.value()) {
        QAction *lockAction = new QAction(QIcon::fromTheme("system-lock-screen-symbolic"), tr("Lock Screen"), powerMenu.get());
        connect(lockAction, &QAction::triggered, powerMenu.get(), [] {
            AppManager::instance()->launchBinaryApp("ukui-screensaver-command", "-l");
        });
        powerMenu->addAction(lockAction);
    }

    reply = qDBusInterface.call("canSuspend");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(QIcon::fromTheme("ukui-sleep-symbolic"), tr("Suspend"), powerMenu.get());
        action->setToolTip(tr("<p>The computer remains on but consumes less power, "
                              "and the applications will remain open. You can quickly wake "
                              "up the computer and return to the state you left</p>"));
        connect(action, &QAction::triggered, powerMenu.get(), [] {
            AppManager::instance()->launchBinaryApp("ukui-session-tools", "--suspend");
        });
        powerMenu->addAction(action);
    }

    reply = qDBusInterface.call("canHibernate");
    if (reply.isValid() && reply.value()) {
        QAction *action = new QAction(QIcon::fromTheme("ukui-hibernate-symbolic"), tr("Hibernate"), powerMenu.get());
        action->setToolTip(tr("<p>Turn off the computer, but the applications "
                              "will remain open. When you turn on the computer again, "
                              "you can return to the state you were in before</p>"));
        connect(action, &QAction::triggered, powerMenu.get(), [] {
            AppManager::instance()->launchBinaryApp("ukui-session-tools", "--hibernate");
        });
        powerMenu->addAction(action);
    }

    bool isUpgradable = canUpgrade();
    bool isNeedToHide = checkHideUpdate();

    reply = qDBusInterface.call("canPowerOff");
    if (reply.isValid() && reply.value()) {
        if (isUpgradable) {
            QAction *upgradeshutdown = new QAction(QIcon::fromTheme("system-upd-shutdown-symbolic"), tr("Upgrade and Shut Down"), powerMenu.get());
            upgradeshutdown->setToolTip(tr("Close all applications, upgrade the computer") +
                                         minutesToHour(m_estimatedLeftMinutes).toUtf8() + tr(",and then turn off it"));
            connect(upgradeshutdown, &QAction::triggered, this, [this] {
                execUpgradeAction("shutdown");
            });
            powerMenu->addAction(upgradeshutdown);
        }
        if (!isUpgradable || !isNeedToHide) {
            QAction *action = new QAction(QIcon::fromTheme("system-shutdown-symbolic"), tr("Shut Down"), powerMenu.get());
            action->setToolTip(tr("<p>Close all applications, and then turn off the computer</p>"));
            connect(action, &QAction::triggered, powerMenu.get(), [] {
                AppManager::instance()->launchBinaryApp("ukui-session-tools", "--shutdown");
            });
            powerMenu->addAction(action);
        }
    }

    reply = qDBusInterface.call("canReboot");
    if (reply.isValid() && reply.value()) {
        if (isUpgradable) {
            QAction *upgradeReboot = new QAction(QIcon::fromTheme("system-upd-reboot-symbolic"), tr("Upgrade and Restart"), powerMenu.get());
            upgradeReboot->setToolTip(tr("Close all applications, upgrade the computer") +
                                         minutesToHour(m_estimatedLeftMinutes).toUtf8() + tr(",and then restart the computer"));
            connect(upgradeReboot, &QAction::triggered, this, [this] {
                execUpgradeAction("reboot");
            });
            powerMenu->addAction(upgradeReboot);
        }
        if (!isUpgradable || !isNeedToHide) {
            QAction *action = new QAction(QIcon::fromTheme("system-reboot-symbolic"), tr("Restart"), powerMenu.get());
            action->setToolTip(tr("<p>Close all applications, turn off the computer, and then turn it back on</p>"));
            connect(action, &QAction::triggered, powerMenu.get(), [] {
                AppManager::instance()->launchBinaryApp("ukui-session-tools", "--reboot");
            });
            powerMenu->addAction(action);
        }
    }

    if (powerMenu.get()->actions().size() == 0) return;
    m_contextMenu = powerMenu.take();
    m_contextMenu->setToolTipsVisible(true);
    m_contextMenu->setAttribute(Qt::WA_DeleteOnClose);

    if (isFullScreen) {
        m_contextMenu.data()->popup(QPoint(menuX - m_contextMenu->sizeHint().width(), menuY - m_contextMenu->sizeHint().height()));
    } else {
        m_contextMenu.data()->popup(QPoint(menuX, menuY - m_contextMenu->sizeHint().height()));
    }
}

bool PowerButton::functionControlOperation(QString operation)
{
    static const QMap<QString, std::function<bool()>> opMap = {
    {"lockScreen",         [this]() { AppManager::instance()->launchBinaryApp("ukui-screensaver-command", "-l"); return true; }},
    {"suspend",            [this]() { AppManager::instance()->launchBinaryApp("ukui-session-tools", "--suspend"); return true; }},
    {"hibernate",          [this]() { AppManager::instance()->launchBinaryApp("ukui-session-tools", "--hibernate"); return true; }},
    {"shutdown",           [this]() { AppManager::instance()->launchBinaryApp("ukui-session-tools", "--shutdown"); return true; }},
    {"restart",            [this]() { AppManager::instance()->launchBinaryApp("ukui-session-tools", "--reboot"); return true; }},
    {"upgradeAndShutdown", [this]() { execUpgradeAction("shutdown"); return true; }},
    {"upgradeAndRestart",  [this]() { execUpgradeAction("reboot"); return true; }}};

    auto it = opMap.find(operation);
    if (it != opMap.end()) {
        return it.value()();
    }
    return false;
}

bool PowerButton::hasMultipleUsers()
{
    QDBusInterface interface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!interface.isValid()) {
        return false;
    }

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.Accounts", "HasMultipleUsers");
    return reply.value().toBool();
}

bool PowerButton::canSwitch()
{
    QDBusInterface interface("org.freedesktop.DisplayManager", "/org/freedesktop/DisplayManager/Seat0",
                             "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (!interface.isValid()) {
        return false;
    }

    QDBusReply<QVariant> reply = interface.call("Get", "org.freedesktop.DisplayManager.Seat", "CanSwitch");
    return reply.value().toBool();
}

bool PowerButton::canUpgrade()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.kylin.systemupgrade", "/com/kylin/systemupgrade",
                                                      "com.kylin.systemupgrade.interface", "CheckInstallRequired");
    QDBusMessage res = QDBusConnection::systemBus().call(msg);
    if (res.type() == QDBusMessage::ReplyMessage && res.arguments().count() > 1) {
        m_estimatedLeftMinutes = res.arguments().at(1).toInt();
        return res.arguments().at(0).toBool();
    }
    if (res.type() == QDBusMessage::InvalidMessage || res.type() == QDBusMessage::ErrorMessage) {
        qCritical() << "Can't creat method call 'CheckInstallRequired'. Error: " << res.errorMessage();
        return false;
    }
    return false;
}

bool PowerButton::checkHideUpdate()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.kylin.systemupgrade", "/com/kylin/systemupgrade",
                                                      "com.kylin.systemupgrade.interface", "CheckHideUpdate");
    QDBusMessage res = QDBusConnection::systemBus().call(msg);
    if (res.type() == QDBusMessage::ReplyMessage) {
        return res.arguments().at(0).toBool();
    }
    qWarning() << "Call 'com.kylin.systemupgrade' interface method 'CheckHideUpdate' failed" << res.errorMessage();
    return false;
}

void PowerButton::execUpgradeAction(const QString &action)
{
    QDBusInterface interface("com.kylin.systemupgrade", "/com/kylin/systemupgrade",
                             "com.kylin.systemupgrade.interface", QDBusConnection::systemBus());

    if (interface.isValid()) {
        interface.asyncCall("TriggerInstallOnShutdown", action);
    } else {
        qCritical() << "'com.kylin.systemupgrade' interface is invalid!" <<  QDBusConnection::systemBus().lastError().message();
    }
}

QString PowerButton::minutesToHour(int estmateTime)
{
    if (estmateTime > 60) {
        int hour = estmateTime / 60;
        int min = estmateTime % 60;
        return tr("(Estimated %1 hour %2 minutes)").arg(hour).arg(min);
    } else if (estmateTime > 0) {
        return tr("(Estimated %1 minutes)").arg(estmateTime);
    }
    return {};
}
