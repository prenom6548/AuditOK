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
#include "greeterconfig.h"
#include <QDebug>
#include "../common/definetypes.h"

std::unique_ptr<GreeterConfig> GreeterConfig::m_instance;
std::once_flag GreeterConfig::m_initFlag;
GreeterConfig::GreeterConfig(QObject *parent)
    : QDBusAbstractInterface(
        UNIAUTH_DBUS_SERVICE,
        UNIAUTH_GREETERCONFIG_DBUS_PATH,
        UNIAUTH_DBUS_INTERFACE,
        QDBusConnection::systemBus(),
        parent)
{
    setTimeout(2147483647);
}

GreeterConfig *GreeterConfig::instance(QObject *parent)
{
    std::call_once(m_initFlag, [&, parent]() { m_instance.reset(new GreeterConfig(parent)); });
    return m_instance.get();
}

// 设置/获取虚拟键盘使能
void GreeterConfig::setVirtualKeyboardEnable(bool isEnable)
{
    QDBusMessage result = call(QStringLiteral("setVirtualKeyboardEnable"), isEnable);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setVirtualKeyboardEnable error:" << result.errorMessage();
    }
}

bool GreeterConfig::getVirtualKeyboardEnable()
{
    QDBusMessage result = call(QStringLiteral("getVirtualKeyboardEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getVirtualKeyboardEnable error:" << result.errorMessage();
        return true;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return true;
    }
}

bool GreeterConfig::resetVirtualKeyboardEnable()
{
    QDBusMessage result = call(QStringLiteral("resetVirtualKeyboardEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "resetVirtualKeyboardEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 设置/获取用户切换使能
void GreeterConfig::setUserSwitchEnable(bool isEnable)
{
    QDBusMessage result = call(QStringLiteral("setUserSwitchEnable"), isEnable);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setUserSwitchEnable error:" << result.errorMessage();
    }
}

bool GreeterConfig::getUserSwitchEnable()
{
    QDBusMessage result = call(QStringLiteral("getUserSwitchEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getUserSwitchEnable error:" << result.errorMessage();
        return true;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return true;
    }
}

bool GreeterConfig::resetUserSwitchEnable()
{
    QDBusMessage result = call(QStringLiteral("resetUserSwitchEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "resetUserSwitchEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 设置/获取网络使能
void GreeterConfig::setNetworkEnable(bool isEnable)
{
    QDBusMessage result = call(QStringLiteral("setNetworkEnable"), isEnable);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setNetworkEnable error:" << result.errorMessage();
    }
}

bool GreeterConfig::getNetworkEnable()
{
    QDBusMessage result = call(QStringLiteral("getNetworkEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getNetworkEnable error:" << result.errorMessage();
        return true;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return true;
    }
}

bool GreeterConfig::resetNetworkEnable()
{
    QDBusMessage result = call(QStringLiteral("resetNetworkEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "resetNetworkEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 设置/获取电源管理使能
void GreeterConfig::setPowerManagerEnable(bool isEnable)
{
    QDBusMessage result = call(QStringLiteral("setPowerManagerEnable"), isEnable);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setPowerManagerEnable error:" << result.errorMessage();
    }
}

bool GreeterConfig::getPowerManagerEnable()
{
    QDBusMessage result = call(QStringLiteral("getPowerManagerEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getPowerManagerEnable error:" << result.errorMessage();
        return true;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return true;
    }
}

bool GreeterConfig::resetPowerManagerEnable()
{
    QDBusMessage result = call(QStringLiteral("resetPowerManagerEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "resetPowerManagerEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 锁屏日期时间使能
void GreeterConfig::setLockScreenDatetimeEnable(bool isEnable)
{
    QDBusMessage result = call(QStringLiteral("setLockScreenDatetimeEnable"), isEnable);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setLockScreenDatetimeEnable error:" << result.errorMessage();
    }
}

bool GreeterConfig::getLockScreenDatetimeEnable(QString strUserName)
{
    QDBusMessage result = call(QStringLiteral("getLockScreenDatetimeEnable"), strUserName);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getLockScreenDatetimeEnable error:" << result.errorMessage();
        return true;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return true;
    }
}

bool GreeterConfig::resetLockScreenDatetimeEnable()
{
    QDBusMessage result = call(QStringLiteral("resetLockScreenDatetimeEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "resetLockScreenDatetimeEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 屏保时间日期使能
void GreeterConfig::setScreensaverDatetimeEnable(bool isEnable)
{
    QDBusMessage result = call(QStringLiteral("setScreensaverDatetimeEnable"), isEnable);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setScreensaverDatetimeEnable error:" << result.errorMessage();
    }
}

bool GreeterConfig::getScreensaverDatetimeEnable(QString strUserName)
{
    QDBusMessage result = call(QStringLiteral("getScreensaverDatetimeEnable"), strUserName);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getScreensaverDatetimeEnable error:" << result.errorMessage();
        return true;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return true;
    }
}

bool GreeterConfig::resetScreensaverDatetimeEnable()
{
    QDBusMessage result = call(QStringLiteral("resetScreensaverDatetimeEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "resetScreensaverDatetimeEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

bool GreeterConfig::getSelectManualLoginDefault()
{
    QDBusMessage result = call(QStringLiteral("getSelectManualLoginDefault"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getSelectManualLoginDefault error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}
