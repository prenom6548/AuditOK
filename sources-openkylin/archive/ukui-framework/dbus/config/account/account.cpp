/*
 * UKUI Framework
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */

#include "account.h"
#include <QDateTime>
#include <QDebug>

#undef signals
#include <gio/gio.h>

Account::Account(QObject *parent)
    : QObject{parent}
{
}

Account::~Account()
{

}

const QString Account::GetIconFile(const int uid) const
{
    QString iconfile;
    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            QString("/org/freedesktop/Accounts/User%1").arg(uid),
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        iconfile = propertyMap.find("IconFile").value().toString();
    }

    return iconfile;
}

QString Account::renameFaceName(QString fullFilename, QString username)
{
    // 拷贝文件名 原名-用户名-时间戳
    QDateTime time = QDateTime::currentDateTime();
    QString filename = fullFilename.split("/").at(fullFilename.split("/").count()-1);
    QString suffix = "." + filename.split(".").at(filename.split(".").count() - 1);
    filename = filename.left(filename.count() - suffix.count());
    qDebug() << filename;

    return filename + "-" + username + "-" + time.toString("yyyyMMddhhmmss") + suffix;
}

void Account::SetIconFile(const int uid, const QString &filename)
{
    // 根据uid获取用户名
    QString username;
    QDBusInterface *iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            QString("/org/freedesktop/Accounts/User%1").arg(uid),
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        username = propertyMap.find("UserName").value().toString();
    }

    QString reFilename = renameFaceName(filename, username);
    // 系统头像或历史头像不进行重命名，直接拷贝
    bool systemFaces = filename.startsWith(FACEPATH);
    if (systemFaces || filename.contains(CUSTOMFACEPATH)) {
        reFilename = filename.split("/").at(filename.split("/").count() - 1);
    }

    qDebug() << "renamed filename = " << reFilename;
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                          QString("/org/freedesktop/Accounts/User%1").arg(uid),
                                                          "org.freedesktop.Accounts.User",
                                                          "SetIconFileRename");
    message << filename << reFilename;
    QDBusMessage response = QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ErrorMessage){
        qDebug() << "setIconFileRename failed" << filename;
    } else {
        qDebug() << "setIconFileRename success" << filename;
        Q_EMIT IconFileChanged();
    }
}
