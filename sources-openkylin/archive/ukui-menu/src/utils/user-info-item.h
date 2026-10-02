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

#ifndef USERINFOITEM_H
#define USERINFOITEM_H

#include <QObject>
#include <QDBusInterface>
#include <QMenu>
#include <QIcon>

namespace UkuiMenu {
class UserInfoItem : public QObject
{
    Q_OBJECT

public:
    explicit UserInfoItem(QString objectPath, QObject *parent = nullptr);
    ~UserInfoItem() = default;

    void initUserInfo();
    void createCircularIcon();

    bool isCurrentUser() {return m_isCurrentUser;};
    bool isLogged() {return m_isLogged;};
    bool autoLogin() {return m_autoLogin;};
    bool noPwdLogin() {return m_noPwdLogin;};

    int accountType() {return m_accountType;};
    int passwdType() {return m_passwdType;};
    qint64 uid() {return m_uid;};

    QString objPath() {return m_objPath;};
    QString userName() {return m_userName;};
    QString realName() {return m_realName;};
    QString iconFile() {return m_iconFile;};
    QString passwd() {return m_passwd;};
    QIcon circularIcon() {return m_circularIcon;};


private Q_SLOTS:
    void userInfoUpdateSlot(const QString &property, const QMap<QString, QVariant> &propertyMap,
                            const QStringList &propertyList);

Q_SIGNALS:
    void userInfoChanged();

private:
    QString m_objectPath;

    bool m_isCurrentUser;
    bool m_isLogged;
    bool m_autoLogin;
    bool m_noPwdLogin;

    int m_accountType;
    int m_passwdType;
    qint64 m_uid;

    QString m_objPath;
    QString m_userName;
    QString m_realName;
    QString m_iconFile;
    QString m_passwd;
    QIcon m_circularIcon;
};
}

#endif // USERINFOITEM_H
