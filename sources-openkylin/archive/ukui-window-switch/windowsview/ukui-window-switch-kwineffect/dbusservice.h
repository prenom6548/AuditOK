/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QDBusConnection>
#include <QObject>
#include <QString>

// 前置声明。解决dbusservice.h和multitaskviewmanager.h
// 相互引用引发QtCreator提示DBusService类型找不到的问题
class MultitaskViewManager;

class DBusService : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.KWin.MultitaskView")

public:
    DBusService(MultitaskViewManager* viewManager,
                QObject *parent = nullptr);
    ~DBusService();

public slots:
    void show();
    void hide();

private:
    bool startService();
    bool stopService();

private:
    MultitaskViewManager* m_viewManager = nullptr;
    QString m_serviceName = "org.kde.KWin";
    QString m_servicePath = "/MultitaskView";
    QDBusConnection m_connection;
};


#endif // DBUSSERVICE_H
