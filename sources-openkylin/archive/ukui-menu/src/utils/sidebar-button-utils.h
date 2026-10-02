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
#ifndef SIDEBARBUTTONUTILS_H
#define SIDEBARBUTTONUTILS_H

#include "user-info-item.h"
#include <QObject>
#include <QDBusInterface>
#include <QMenu>
#include <QPointer>

namespace UkuiMenu {

class SidebarButtonUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ getUsername NOTIFY userInfoChanged)
    Q_PROPERTY(QString realName READ getRealName NOTIFY userInfoChanged)
    Q_PROPERTY(QString iconFile READ getIconFile NOTIFY userInfoChanged)
public:
    explicit SidebarButtonUtils(QObject *parent = nullptr);
    static void defineModule(const char *uri, int versionMajor, int versionMinor);

    Q_INVOKABLE QString getUsername();
    Q_INVOKABLE QString getRealName();
    Q_INVOKABLE QString getIconFile();
    Q_INVOKABLE void openUserCenter();
    Q_INVOKABLE void openPeonyComputer();
    Q_INVOKABLE void openControlCenter();
    Q_INVOKABLE void openUsersInfoMenu(int menuX, int menuY, bool isFullScreen);

private:
    void registeredAccountsDbus();
    void initUserInfo();
    QStringList getUserObjectPath();
    void switchToUser(QString userName);

private Q_SLOTS:
    void onUserAdded(const QDBusObjectPath &op);
    void onUserRemoved(const QDBusObjectPath &op);

Q_SIGNALS:
    void userInfoChanged();

private:
    QDBusInterface *m_systemUserIFace = nullptr;
    QDBusInterface *m_currentUserIFace = nullptr;

    std::shared_ptr<UserInfoItem> m_currentUserInfo;
    QString m_administrator;
    QString m_standardUser;
    QList<std::shared_ptr<UserInfoItem>> m_userInfoItemList;
};

} // UkuiMenu

#endif // SIDEBARBUTTONUTILS_H
