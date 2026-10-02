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
#include "../common/definetypes.h"
#include "freedesktophelper.h"

/* For the type WillLoginUserInfo */
QDBusArgument &operator<<(QDBusArgument &argument, const WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument << willLoginUserInfo.strUserName << willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument >> willLoginUserInfo.strUserName >> willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo)
{
    stream << "WillLogUser [" << willLoginUserInfo.strUserName << willLoginUserInfo.isOneKeyLogin << "]";
    return stream;
}

std::unique_ptr<UniAuthService> UniAuthService::m_instance;
std::once_flag UniAuthService::m_initFlag;
UniAuthService::UniAuthService(QObject *parent)
    : QDBusAbstractInterface(
        UNIAUTH_DBUS_SERVICE, UNIAUTH_DBUS_PATH, UNIAUTH_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
    , m_isActivatable(false)
{
    qRegisterMetaType<WillLoginUserInfo>("WillLoginUserInfo");
    qDBusRegisterMetaType<WillLoginUserInfo>();
    setTimeout(2147483647);
    if (FreedesktopHelper::getInstance(false)->NameHasOwner(UNIAUTH_DBUS_INTERFACE)) {
        m_isActivatable = true;
    }
}

UniAuthService *UniAuthService::instance(QObject *parent)
{
    std::call_once(m_initFlag, [&, parent]() { m_instance.reset(new UniAuthService(parent)); });
    return m_instance.get();
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

// 获取优先级最高的认证类型
int UniAuthService::getDefaultAuthType(QString userName)
{
    QDBusMessage result = call(QStringLiteral("getDefaultAuthType"), userName);
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getDefaultAuthType error:" << result.errorMessage();
        return -1;
    }
    QList<QVariant> varResult = result.arguments();
    if (varResult.size() > 0) {
        int authType = varResult.takeFirst().toInt();
        return authType;
    } else {
        return -1;
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

// 生物特征开关接口
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

void UniAuthService::SwitchToUser(QString strUserName)
{
    QDBusReply<int> reply = call(QStringLiteral("SwitchToUser"), strUserName);
    if (!reply.isValid()) {
        qWarning() << "SwitchToUser error:" << reply.error();
        return;
    }
    return;
}

void UniAuthService::SwitchToGreeterUser(QString strUserName)
{
    QDBusReply<int> reply = call(QStringLiteral("SwitchToGreeterUser"), strUserName);
    if (!reply.isValid()) {
        qWarning() << "SwitchToGreeterUser error:" << reply.error();
        return;
    }
    return;
}

WillLoginUserInfo UniAuthService::getWillSwitchUser()
{
    WillLoginUserInfo willLoginUserInfo;
    QDBusMessage result = call(QStringLiteral("getWillSwitchUser"));
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getWillSwitchUser error:" << result.errorMessage();
        return willLoginUserInfo;
    }
    if (result.arguments().size() > 0) {
        QDBusArgument argInfo = result.arguments().at(0).value<QDBusArgument>();
        argInfo >> willLoginUserInfo;
    }
    qDebug() << "getUserInfo:" << willLoginUserInfo;
    return willLoginUserInfo;
}

bool UniAuthService::isUserNameValid(QString strUserName)
{
    struct passwd *userInfo = nullptr;
    userInfo = getpwnam(strUserName.toLatin1().data());
    if (userInfo) {
        return true;
    }
    return false;
}

int UniAuthService::SaveLastLoginUser(QString strUserName)
{
    QDBusReply<int> reply = call(QStringLiteral("SaveLastLoginUser"), strUserName);
    if (!reply.isValid()) {
        qWarning() << "SaveLastLoginUser error:" << reply.error();
        return -1;
    }
    return reply.value();
}

QString UniAuthService::GetLastLoginUser()
{
    QDBusReply<QString> reply = call(QStringLiteral("GetLastLoginUser"));
    if (!reply.isValid()) {
        qWarning() << "GetLastLoginUser error:" << reply.error();
        return QString("");
    }
    return reply.value();
}

int UniAuthService::SaveQuickLoginUser(QString strUserName)
{
    QDBusReply<int> reply = call(QStringLiteral("SaveQuickLoginUser"), strUserName);
    if (!reply.isValid()) {
        qWarning() << "SaveQuickLoginUser error:" << reply.error();
        return -1;
    }
    return reply.value();
}

int UniAuthService::AsyncQuickLoginUser(QString strUserName)
{
    QDBusPendingCall pendingCall = asyncCall(QStringLiteral("SaveQuickLoginUser"), strUserName);
    Q_UNUSED(pendingCall)
    return 0;
}

QString UniAuthService::GetQuickLoginUser()
{
    QDBusReply<QString> reply = call(QStringLiteral("GetQuickLoginUser"));
    if (!reply.isValid()) {
        qWarning() << "GetQuickLoginUser error:" << reply.error();
        return QString("");
    }
    return reply.value();
}
