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
#include "userinfo.h"
#include "common/global_utils.h"
#include "common/definetypes.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QFile>

UserInfo::UserInfo(QObject *parent)
    : QObject(parent)
    , m_isLoggedIn(false)
    , m_uId(-1)
    , m_strBackground(DEFAULT_BACKGROUND_PATH)
    , m_strLang(qgetenv("LANG"))
    , m_strName("")
    , m_strGreeterBackground("")
    , m_isSystemAccount(false)
{
    m_strHeadImage = getDefaultFace();
}

UserInfo::UserInfo(const UserInfo &userInfo)
    : QObject(userInfo.parent())
    , m_isLoggedIn(userInfo.m_isLoggedIn)
    , m_uId(userInfo.m_uId)
    , m_strHeadImage(userInfo.m_strHeadImage)
    , m_strFullName(userInfo.m_strFullName)
    , m_strBackground(userInfo.m_strBackground)
    , m_strLang(userInfo.m_strLang)
    , m_strName(userInfo.m_strName)
    , m_strGreeterBackground(userInfo.m_strGreeterBackground)
    , m_isSystemAccount(userInfo.m_isSystemAccount)
{
}

UserInfo::~UserInfo() {}

bool UserInfo::operator==(const UserInfo &userInfo) const
{
    return userInfo.uid() == m_uId && userInfo.name() == m_strName;
}

void UserInfo::updateLoggedIn(const bool &isLoggedIn)
{
    if (isLoggedIn == m_isLoggedIn) {
        return;
    }
    m_isLoggedIn = isLoggedIn;
}

void UserInfo::updateUid(const uid_t &id)
{
    m_uId = id;
}

void UserInfo::updateHeadImage(const QString &path)
{
    m_strHeadImage = path;
}

void UserInfo::updateFullName(const QString &fullName)
{
    m_strFullName = fullName;
}

void UserInfo::updateBackground(const QString &backGround)
{
    m_strBackground = backGround;
}

void UserInfo::updateLang(const QString &lang)
{
    m_strLang = lang;
}

void UserInfo::updateName(const QString &name)
{
    m_strName = name;
}

void UserInfo::updateGreeterBackground(const QString &background)
{
    m_strGreeterBackground = background;
}

void UserInfo::updateBackgroundColor(const QString &color)
{
    m_strBackgroundColor = color;
}

void UserInfo::updateSystemAccount(const bool &isSystemAccount)
{
    if (isSystemAccount == m_isSystemAccount) {
        return;
    }
    m_isSystemAccount = isSystemAccount;
}

QDebug operator<<(QDebug stream, const UserInfo &userInfo)
{
    stream << "[" << userInfo.uid() << userInfo.name() << userInfo.fullName() << userInfo.headImage()
           << userInfo.backGround() << userInfo.isLoggedIn() << userInfo.lang() << "]";
    return stream;
}

LocalUserInfo::LocalUserInfo(const uid_t &uId, QObject *parent)
    : UserInfo(parent), m_strPath("/org/freedesktop/Accounts/User" + QString::number(uId))
{
    initData();
    initConnections();
}

LocalUserInfo::LocalUserInfo(const QString &strPath, QObject *parent) : UserInfo(parent), m_strPath(strPath)
{
    initData();
    initConnections();
}

LocalUserInfo::LocalUserInfo(const LocalUserInfo &localUser) : UserInfo(localUser), m_strPath(localUser.path())
{
    if (m_strPath.isEmpty()) {
        m_strPath = QString("/org/freedesktop/Accounts/User" + QString::number(localUser.uid()));
    }
    initConnections();
}

LocalUserInfo::~LocalUserInfo()
{
    if (m_propertiesChangedConnected) {
        QDBusConnection::systemBus().disconnect(
            ACCOUNTS_DBUS_SERVICE,
            m_strPath,
            FD_PROPERTIES_INTERFACE,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        m_propertiesChangedConnected = false;
    }
}

void LocalUserInfo::initData()
{
    QDBusInterface ifaceExtra(ACCOUNTS_DBUS_SERVICE, m_strPath, FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
    QDBusMessage ret = ifaceExtra.call("GetAll", ACCOUNTS_USER_LDM_SERVICE_INTERFACE);
    QList<QVariant> outArgs = ret.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgsExtra = first.value<QDBusArgument>();
    dbusArgsExtra.beginMap();
    while (!dbusArgsExtra.atEnd()) {
        QString key;
        QVariant value;
        dbusArgsExtra.beginMapEntry();
        dbusArgsExtra >> key >> value;
        if (key == "BackgroundFile") {
            QString strBackground = value.toString();
            if (strBackground.isEmpty() || !QFile(strBackground).exists() || !QFile(strBackground).size()) {
                m_strBackground = DEFAULT_BACKGROUND_PATH;
            } else {
                m_strBackground = strBackground;
            }
        }
        dbusArgsExtra.endMapEntry();
    }
    dbusArgsExtra.endMap();

    QDBusInterface iface(ACCOUNTS_DBUS_SERVICE, m_strPath, FD_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
    ret = iface.call("GetAll", ACCOUNTS_USER_DBUS_INTERFACE);
    outArgs = ret.arguments();
    first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();
    dbusArgs.beginMap();
    while (!dbusArgs.atEnd()) {
        QString key;
        QVariant value;
        dbusArgs.beginMapEntry();
        dbusArgs >> key >> value;
        if (key == "UserName") {
            m_strName = value.toString();
        } else if (key == "RealName") {
            m_strFullName = value.toString();
        } else if (key == "IconFile") {
            QString strHeadImage = value.toString();
            if (strHeadImage.isEmpty() || !QFile(strHeadImage).exists() || !QFile(strHeadImage).size()) {
                m_strHeadImage = getDefaultFace();
            } else {
                m_strHeadImage = strHeadImage;
            }
        } else if (key == "Uid") {
            m_uId = value.toUInt();
        } else if (key == "Language") {
            if (isGreeterMode()) {
                m_strLang = value.toString();
            }
        } else if (key == "BackgroundFile") {
            QString strBackground = value.toString();
            if (!strBackground.isEmpty() && QFile(strBackground).exists() && QFile(strBackground).size()) {
                m_strBackground = strBackground;
            }
        } else if (key == "SystemAccount") {
            m_isSystemAccount = value.toBool();
        }
        dbusArgs.endMapEntry();
    }
    dbusArgs.endMap();
    if (m_strFullName.isEmpty()) {
        m_strFullName = m_strName;
    }
}

void LocalUserInfo::initConnections()
{
    if (!m_propertiesChangedConnected) {
        QDBusConnection::systemBus().connect(
            ACCOUNTS_DBUS_SERVICE,
            m_strPath,
            FD_PROPERTIES_INTERFACE,
            "PropertiesChanged",
            this,
            SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        m_propertiesChangedConnected = true;
    }
}

void LocalUserInfo::onPropertiesChanged(
    const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == ACCOUNTS_USER_DBUS_INTERFACE) {
        QVariantMap::const_iterator itVar = changedProperties.constBegin();
        bool isChanged = false;
        for (; itVar != changedProperties.constEnd(); itVar++) {
            QVariant varValue = itVar.value();
            if (itVar.key() == "UserName") {
                m_strName = varValue.toString();
                isChanged = true;
            } else if (itVar.key() == "RealName") {
                m_strFullName = varValue.toString();
                isChanged = true;
            } else if (itVar.key() == "IconFile") {
                QString strHeadImage = varValue.toString();
                if (strHeadImage.isEmpty() || !QFile(strHeadImage).exists() || !QFile(strHeadImage).size()) {
                    m_strHeadImage = getDefaultFace();
                } else {
                    m_strHeadImage = strHeadImage;
                }
                isChanged = true;
            } else if (itVar.key() == "Uid") {
                m_uId = varValue.toUInt();
                isChanged = true;
            } else if (itVar.key() == "Language") {
                if (isGreeterMode()) {
                    m_strLang = varValue.toString();
                    isChanged = true;
                }
            } else if (itVar.key() == "BackgroundFile") {
                QString strBackground = varValue.toString();
                if (!strBackground.isEmpty() && QFile(strBackground).exists() && QFile(strBackground).size()) {
                    m_strBackground = strBackground;
                    isChanged = true;
                }
            }
        }
        if (m_strFullName.isEmpty()) {
            m_strFullName = m_strName;
        }
        if (isChanged)
            Q_EMIT userPropChanged(m_strName);
    }
}

QDebug operator<<(QDebug stream, const LocalUserInfo &userInfo)
{
    stream << "[" << userInfo.path() << userInfo.uid() << userInfo.name() << userInfo.fullName() << userInfo.headImage()
           << userInfo.backGround() << userInfo.isLoggedIn() << userInfo.lang() << "]";
    return stream;
}
