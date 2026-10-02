/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "greeterconfiginterface.h"
#include "dbusaccesscontrol.h"
#include <QDBusError>
#include <QFile>
#include <QTextStream>
#include <QDBusConnectionInterface>
#include <qdbusconnection.h>
#include <pwd.h>

#define GROUP_FUNCTIONS "Functions"
#define KEY_USER_SWITCH_ENABLE "UserSwitchEnable"
#define KEY_VIRTUAL_KEYBOARD_ENABLE "VirtualKeyBoardEnable"
#define KEY_NETWORK_ENABLE "NetworkEnable"
#define KEY_POWERMANAGER_ENABLE "PowerManagerEnable"
#define KEY_LOCKSCREEN_SHOWDATETIME_ENABLE "LockScreenShowDatetimeEnable"
#define KEY_SCREENSAVER_SHOWDATETIME_ENABLE "ScreensaverShowDatetimeEnable"
#define KEY_LOGIN_DISPLAYED_USER_COUNT "LoginDisplayedUserCount"
#define KEY_SELECT_MANUAL_LOGIN_DEFAULT "SelectManualLoginDefault"
#define GROUP_GREETER "Greeter"

GreeterConfigInterface::GreeterConfigInterface(QObject *parent) : QObject(parent)
{
    m_config = new GreeterConfig(this);
    connect(m_config, &GreeterConfig::changed, this, &GreeterConfigInterface::changed);
    connect(m_config, &GreeterConfig::changedForUser, this, &GreeterConfigInterface::userConfigChanged);
    connect(m_config, &GreeterConfig::changed, this, &GreeterConfigInterface::onChanged);
    connect(m_config, &GreeterConfig::changedForUser, this, &GreeterConfigInterface::onChangedForUser);
}

void GreeterConfigInterface::onChanged(const QString &group, const QString &key)
{
    if (group == GROUP_FUNCTIONS) {
        if (key == KEY_VIRTUAL_KEYBOARD_ENABLE) {
            Q_EMIT virtualKeyboardEnableChanged(getVirtualKeyboardEnable());
        } else if (key == KEY_NETWORK_ENABLE) {
            Q_EMIT networkEnableChanged(getNetworkEnable());
        } else if (key == KEY_USER_SWITCH_ENABLE) {
            Q_EMIT userSwitchEnableChanged(getUserSwitchEnable());
        } else if (key == KEY_POWERMANAGER_ENABLE) {
            Q_EMIT powerManagerEnableChanged(getPowerManagerEnable());
        } else if (key == KEY_LOGIN_DISPLAYED_USER_COUNT) {
            Q_EMIT loginDisplayedUserCountChanged(getLoginDisplayedUserCount());
        }
    }
}

void GreeterConfigInterface::onChangedForUser(const QString &group, const QString &key, const QString &strUserName)
{
    if (group == GROUP_FUNCTIONS) {
        if (key == KEY_LOCKSCREEN_SHOWDATETIME_ENABLE) {
            Q_EMIT lockScreenDatetimeEnableChanged(getLockScreenDatetimeEnable(strUserName), strUserName);
        } else if (key == KEY_SCREENSAVER_SHOWDATETIME_ENABLE) {
            Q_EMIT screensaverDatetimeChanged(getScreensaverDatetimeEnable(strUserName), strUserName);
        }
    }
}

void GreeterConfigInterface::setLoginDisplayedUserCount(int count)
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return;
    }
    qDebug() << "setLoginDisplayedUserCount:" << count;
    m_config->setValue(GROUP_FUNCTIONS, KEY_LOGIN_DISPLAYED_USER_COUNT, count);
}

int GreeterConfigInterface::getLoginDisplayedUserCount()
{
    if (!ensureGlobalConfigReadAllowed()) {
        return -1;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_LOGIN_DISPLAYED_USER_COUNT, -1).toInt();
}

bool GreeterConfigInterface::resetLoginDisplayedUserCount()
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return false;
    }
    qDebug() << "resetLoginDisplayedUserCount!";
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_LOGIN_DISPLAYED_USER_COUNT);
}

void GreeterConfigInterface::setVirtualKeyboardEnable(bool isEnable)
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return;
    }
    qDebug() << "setVirtualKeyboardEnable:" << isEnable;
    m_config->setValue(GROUP_FUNCTIONS, KEY_VIRTUAL_KEYBOARD_ENABLE, isEnable);
}

bool GreeterConfigInterface::getVirtualKeyboardEnable()
{
    if (!ensureGlobalConfigReadAllowed()) {
        return false;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_VIRTUAL_KEYBOARD_ENABLE, true).toBool();
}

bool GreeterConfigInterface::resetVirtualKeyboardEnable()
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return false;
    }
    qDebug() << "resetVirtualKeyboardEnable!";
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_VIRTUAL_KEYBOARD_ENABLE);
}

void GreeterConfigInterface::setUserSwitchEnable(bool isEnable)
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return;
    }
    qDebug() << "setUserSwitchEnable:" << isEnable;
    m_config->setValue(GROUP_FUNCTIONS, KEY_USER_SWITCH_ENABLE, isEnable);
}

bool GreeterConfigInterface::getUserSwitchEnable()
{
    if (!ensureGlobalConfigReadAllowed()) {
        return false;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_USER_SWITCH_ENABLE, true).toBool();
}

bool GreeterConfigInterface::resetUserSwitchEnable()
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return false;
    }
    qDebug() << "resetUserSwitchEnable!";
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_USER_SWITCH_ENABLE);
}

void GreeterConfigInterface::setNetworkEnable(bool isEnable)
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return;
    }
    qDebug() << "setNetworkEnable:" << isEnable;
    m_config->setValue(GROUP_FUNCTIONS, KEY_NETWORK_ENABLE, isEnable);
}

bool GreeterConfigInterface::getNetworkEnable()
{
    if (!ensureGlobalConfigReadAllowed()) {
        return false;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_NETWORK_ENABLE, true).toBool();
}

bool GreeterConfigInterface::resetNetworkEnable()
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return false;
    }
    qDebug() << "resetNetworkEnable!";
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_NETWORK_ENABLE);
}

void GreeterConfigInterface::setPowerManagerEnable(bool isEnable)
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return;
    }
    qDebug() << "setPowerManagerEnable:" << isEnable;
    m_config->setValue(GROUP_FUNCTIONS, KEY_POWERMANAGER_ENABLE, isEnable);
}

bool GreeterConfigInterface::getPowerManagerEnable()
{
    if (!ensureGlobalConfigReadAllowed()) {
        return false;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_POWERMANAGER_ENABLE, true).toBool();
}

bool GreeterConfigInterface::resetPowerManagerEnable()
{
    if (!ensureGlobalConfigWriteAllowed()) {
        return false;
    }
    qDebug() << "resetPowerManagerEnable!";
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_POWERMANAGER_ENABLE);
}

// lock screen datetime enable
void GreeterConfigInterface::setLockScreenDatetimeEnable(bool isEnable)
{
    QString userName = getUserNameFromCaller();
    if (!ensureUserConfigAccessAllowed(userName)) {
        return;
    }
    qDebug() << "setLockScreenDatetimeEnable:" << isEnable << "," << userName;
    m_config->setValue(GROUP_FUNCTIONS, KEY_LOCKSCREEN_SHOWDATETIME_ENABLE, isEnable, userName);
}

bool GreeterConfigInterface::getLockScreenDatetimeEnable(QString strUserName)
{
    if (!ensureUserConfigAccessAllowed(strUserName)) {
        return false;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_LOCKSCREEN_SHOWDATETIME_ENABLE, true, strUserName).toBool();
}

bool GreeterConfigInterface::resetLockScreenDatetimeEnable()
{
    QString userName = getUserNameFromCaller();
    if (!ensureUserConfigAccessAllowed(userName)) {
        return false;
    }
    qDebug() << "resetLockScreenDatetimeEnable:" << userName;
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_LOCKSCREEN_SHOWDATETIME_ENABLE, userName);
}

// screensaver datetime enable
void GreeterConfigInterface::setScreensaverDatetimeEnable(bool isEnable)
{
    QString userName = getUserNameFromCaller();
    if (!ensureUserConfigAccessAllowed(userName)) {
        return;
    }
    qDebug() << "setScreensaverDatetimeEnable:" << isEnable << "," << userName;
    m_config->setValue(GROUP_FUNCTIONS, KEY_SCREENSAVER_SHOWDATETIME_ENABLE, isEnable, userName);
}

bool GreeterConfigInterface::getScreensaverDatetimeEnable(QString strUserName)
{
    if (!ensureUserConfigAccessAllowed(strUserName)) {
        return false;
    }
    return m_config->getValue(GROUP_FUNCTIONS, KEY_SCREENSAVER_SHOWDATETIME_ENABLE, true, strUserName).toBool();
}

bool GreeterConfigInterface::resetScreensaverDatetimeEnable()
{
    QString userName = getUserNameFromCaller();
    if (!ensureUserConfigAccessAllowed(userName)) {
        return false;
    }
    qDebug() << "resetScreensaverDatetimeEnable:" << userName;
    return m_config->resetValue(GROUP_FUNCTIONS, KEY_SCREENSAVER_SHOWDATETIME_ENABLE, userName);
}

bool GreeterConfigInterface::getSelectManualLoginDefault()
{
    if (!ensureGlobalConfigReadAllowed()) {
        return false;
    }
    return m_config->getValue(GROUP_GREETER, KEY_SELECT_MANUAL_LOGIN_DEFAULT, false).toBool();
}

QString GreeterConfigInterface::printConfig(int nType, QString strUserName)
{
    if (strUserName.isEmpty()) {
        if (!ensureGlobalConfigReadAllowed()) {
            return QString();
        }
    } else {
        if (!ensureUserConfigAccessAllowed(strUserName)) {
            return QString();
        }
    }
    return m_config->printConfig(nType, strUserName);
}

bool GreeterConfigInterface::checkEnviron(int pid)
{
    QString filePath = QString("/proc/%1/environ").arg(pid);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "file.open failed" << filePath;
        return false;
    }
    QTextStream in(&file);
    QString data = in.readAll();
    file.close();
    data.replace('\0', '\n');
    QStringList environVars = data.split('\n');
    for (const QString &var : environVars) {
        if (!var.isEmpty()) {
            int pos = var.indexOf('=');
            if (pos != -1) {
                QString key = var.left(pos);
                if (key == "LD_PRELOAD" || key == "LD_LIBRARY_PATH" || key == "LD_AUDIT") {
                    return false;
                }
            }
        }
    }
    return true;
}

bool GreeterConfigInterface::ensureGlobalConfigWriteAllowed()
{
    if (!calledFromDBus()) {
        return true;
    }
    if (UniauthDBusAccess::isCallerRoot(connection(), message())) {
        return true;
    }
    sendErrorReply(QDBusError::AccessDenied, QStringLiteral("Access denied"));
    return false;
}

bool GreeterConfigInterface::ensureGlobalConfigReadAllowed()
{
    if (!calledFromDBus()) {
        return true;
    }
    if (UniauthDBusAccess::isCallerRoot(connection(), message())
        || UniauthDBusAccess::isCallerProcessWhitelisted(connection(), message())) {
        return true;
    }
    sendErrorReply(QDBusError::AccessDenied, QStringLiteral("Access denied"));
    return false;
}

bool GreeterConfigInterface::ensureUserConfigAccessAllowed(const QString &targetUserName)
{
    if (!calledFromDBus()) {
        return true;
    }
    if (UniauthDBusAccess::isCallerRoot(connection(), message())
        || UniauthDBusAccess::isCallerProcessWhitelisted(connection(), message())) {
        return true;
    }
    if (targetUserName.isEmpty()) {
        sendErrorReply(QDBusError::AccessDenied, QStringLiteral("User name required"));
        return false;
    }
    const QString callerUser = getUserNameFromCaller();
    if (!callerUser.isEmpty() && callerUser == targetUserName) {
        return true;
    }
    sendErrorReply(QDBusError::AccessDenied, QStringLiteral("Access denied"));
    return false;
}

QString GreeterConfigInterface::getUserNameFromCaller()
{
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    int uid = intf->serviceUid(clientService).value();
    struct passwd *pwinfo = getpwuid(uid);
    if (pwinfo && pwinfo->pw_name) {
        return QString(pwinfo->pw_name);
    } else {
        return QString("");
    }
}
