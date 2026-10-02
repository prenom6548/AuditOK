/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "uniauthservice.h"
#include <QDebug>
#include <pwd.h>
#include "definetypes.h"
#include "freedesktophelper.h"

UniAuthService::UniAuthService(QObject *parent)
    : QDBusAbstractInterface(
        UNIAUTH_DBUS_SERVICE, UNIAUTH_DBUS_PATH, UNIAUTH_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
    , m_isActivatable(false)
{
    setTimeout(2147483647);
    FreedesktopHelper *systemFdHelper = new FreedesktopHelper(false);
    if (systemFdHelper) {
        if (systemFdHelper->isServiceActivable(UNIAUTH_DBUS_INTERFACE)) {
            m_isActivatable = true;
        }
        delete systemFdHelper;
    }
}

// 设置默认设备
void UniAuthService::setDefaultDevice(int bioDevType, QString deviceName)
{
    qDebug() << " bioType : " << bioDevType << "deviceName : " << deviceName;
    QDBusMessage result = call(QStringLiteral("setDefaultDevice"), bioDevType, deviceName);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setDefaultDevice error:" << result.errorMessage();
        return;
    }
    return;
}

// 获取默认设备
QString UniAuthService::getDefaultDevice(QString userName, int bioDevType)
{
    QDBusMessage result = call(QStringLiteral("getDefaultDevice"), userName, bioDevType);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getDefaultDevice error:" << result.errorMessage();
        return "";
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        QString strDefDevice = varResult.takeFirst().toString();
        return strDefDevice;
    } else {
        return "";
    }
}

// 获取所有默认设备
QStringList UniAuthService::getAllDefaultDevice(QString userName)
{
    QStringList listDefDevices;
    QDBusReply<QStringList> result = call(QStringLiteral("getAllDefaultDevice"), userName);
    if (!result.isValid()) {
        qWarning() << "getAllDefaultDevice error:" << result.error().message();
    } else {
        listDefDevices = result.value();
    }
    return listDefDevices;
}

//生物特征开关接口
bool UniAuthService::getBioAuthStatus(QString userName, int bioAuthType)
{
    QDBusMessage bioResult = call(QStringLiteral("getBioAuthStatus"), userName, bioAuthType);
    if (bioResult.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getBioStatus error:" << bioResult.errorMessage();
        return false;
    }
    QList<QVariant> varResult = bioResult.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

void UniAuthService::setBioAuthStatus(int bioAuthType, bool status)
{
    qDebug() << "setBioAuthStatus bioAuthType : " << bioAuthType << "status : " << status;
    QDBusMessage result = call(QStringLiteral("setBioAuthStatus"), bioAuthType, status);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "setBioAuthStatus error:" << result.errorMessage();
        return;
    }
    return;
}

// 获取最大失败次数
int UniAuthService::getMaxFailedTimes()
{
    QDBusMessage result = call(QStringLiteral("getMaxFailedTimes"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getMaxFailedTimes error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toInt();
    } else {
        return 3;
    }
}

// 获取是否使能微信扫码登录
bool UniAuthService::getQRCodeEnable()
{
    QDBusMessage result = call(QStringLiteral("getQRCodeEnable"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getQRCodeEnable error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 获取是否双认证
bool UniAuthService::getDoubleAuth()
{
    QDBusMessage result = call(QStringLiteral("getDoubleAuth"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getDoubleAuth error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 获取用户绑定
bool UniAuthService::getUserBind()
{
    QDBusMessage result = call(QStringLiteral("getUserBind"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getUserBind error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 获取是否在控制面板显示
bool UniAuthService::getIsShownInControlCenter()
{
    QDBusMessage result = call(QStringLiteral("getIsShownInControlCenter"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getIsShownInControlCenter error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 获取是否使用第一个设备
bool UniAuthService::getUseFirstDevice()
{
    QDBusMessage result = call(QStringLiteral("getUseFirstDevice"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getUseFirstDevice error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

// 获取是否隐藏切换按钮
bool UniAuthService::getHiddenSwitchButton()
{
    QDBusMessage result = call(QStringLiteral("getHiddenSwitchButton"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getHiddenSwitchButton error:" << result.errorMessage();
        return false;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toBool();
    } else {
        return false;
    }
}

bool UniAuthService::isActivatable()
{
    return m_isActivatable;
}

int UniAuthService::getFTimeoutTimes()
{
    QDBusMessage result = call(QStringLiteral("getFTimeoutTimes"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getFTimeoutTimes error:" << result.errorMessage();
        return 1;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        return varResult.takeFirst().toInt();
    } else {
        return 1;
    }
}
