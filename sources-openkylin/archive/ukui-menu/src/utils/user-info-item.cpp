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

#include "user-info-item.h"

#include <QDBusReply>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

#define KYLIN_ACCOUNT_INFORMATION_NAME         "org.freedesktop.Accounts"
#define KYLIN_ACCOUNT_INFORMATION_PATH         "/org/freedesktop/Accounts"
#define KYLIN_ACCOUNT_INFORMATION_INTERFACE    "org.freedesktop.Accounts"
#define DEFAULT_USER_ICON_FILE        ":/res/icon/default-community-image.png"

using namespace UkuiMenu;

UkuiMenu::UserInfoItem::UserInfoItem(QString objectPath, QObject *parent) : QObject(parent)
{
    m_objPath = objectPath;
    initUserInfo();
    createCircularIcon();
    QDBusConnection::systemBus().connect(KYLIN_ACCOUNT_INFORMATION_NAME, objectPath,
                                         "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                         this, SLOT(userInfoUpdateSlot(QString, QMap<QString, QVariant>, QStringList)));

}

void UserInfoItem::initUserInfo()
{
    m_isCurrentUser = false;
    m_isLogged = false;
    m_autoLogin = false;

    QDBusInterface userInterface(KYLIN_ACCOUNT_INFORMATION_NAME,
                                 m_objPath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > reply = userInterface.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        m_userName = propertyMap.find("UserName").value().toString();
        m_realName = propertyMap.find("RealName").value().toString();
        m_accountType = propertyMap.find("AccountType").value().toInt();
        m_iconFile = propertyMap.find("IconFile").value().toString();
        m_passwdType = propertyMap.find("PasswordMode").value().toInt();
        m_uid = propertyMap.find("Uid").value().toInt();
        m_autoLogin = propertyMap.find("AutomaticLogin").value().toBool();

        if (m_userName == QString(qgetenv("USER"))) {
            m_isCurrentUser = true;
            m_isLogged = true;
        }
    } else {
        qWarning() << "UserInfoItem::initUserInfo: 'org.freedesktop.Accounts.User' dbus reply is invalid";
    }

}

void UserInfoItem::createCircularIcon()
{
    QPixmap sourcePixmap(m_iconFile);

    if (sourcePixmap.isNull()) {
        qWarning() << "UserInfoItem::createCircularIcon: Can't load icon file: " << m_iconFile;
        return;
    }

    int diameter = std::min(sourcePixmap.height(), sourcePixmap.width());
    QPixmap circularPixmap(diameter, diameter);
    circularPixmap.fill(Qt::transparent);
    QPainter painter(&circularPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, diameter, diameter);
    painter.setClipPath(path);

    QRect targetRect, sourceRect;
    targetRect = QRect(0, 0, diameter, diameter);
    if (sourcePixmap.width() > sourcePixmap.height()) {
        sourceRect = QRect((sourcePixmap.width() - diameter) / 2, 0, diameter, diameter);
    } else {
        sourceRect = QRect(0, (sourcePixmap.height() - diameter) / 2, diameter, diameter);
    }
    painter.drawPixmap(targetRect, sourcePixmap, sourceRect);
    m_circularIcon =  QIcon(circularPixmap);
}

void UserInfoItem::userInfoUpdateSlot(const QString &property, const QMap<QString, QVariant> &propertyMap, const QStringList &propertyList)
{
    Q_UNUSED(property)
    Q_UNUSED(propertyList)
    QStringList keys = propertyMap.keys();

    if (keys.contains("IconFile")) {
        m_iconFile = propertyMap.value("IconFile").toString();
        createCircularIcon();
    }

    if (keys.contains("UserName")) {
        m_userName = propertyMap.value("UserName").toString();
    }

    if (keys.contains("RealName")) {
        m_realName = propertyMap.value("RealName").toString();
    }

    if (keys.contains("AccountType")) {
        m_accountType = propertyMap.value("AccountType").toInt();
    }

    Q_EMIT userInfoChanged();
}
