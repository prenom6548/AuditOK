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
#ifndef DBUSSERVERMANAGER_H
#define DBUSSERVERMANAGER_H

#include <QObject>
#include <QDBusInterface>

class DBusServerManager : public QObject
{
    Q_OBJECT
private:
    explicit DBusServerManager(QObject *parent = nullptr);

public:
    /**
     * @brief 单实例
     *
     * @param parent 父指针
     * @return DBusServerManager 单实例对象指针
     */
    static DBusServerManager *instance(QObject *parent = nullptr);

    bool serviceExists(const QString &strService);

public Q_SLOTS:
    void onDBusNameOwnerChanged(const QString &name,
                                const QString &oldOwner,
                                const QString &newOwner);

Q_SIGNALS:
    void dbusServerActiveChanged(const QString &strServerName, bool isActive);

private:
    void init();

private:
    class MemCollector{
    public:
        ~MemCollector(){
            if (DBusServerManager::instance()) {
                delete DBusServerManager::instance();
                DBusServerManager::m_instance = nullptr;
            }
        }
    };
    static DBusServerManager *m_instance; /** 统一认证配置服务单实例指针 */
    static MemCollector m_memCollector;
    QDBusInterface *m_dbusService = nullptr;
};

#endif // DBUSSERVERMANAGER_H
