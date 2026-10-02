/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef DBUS_CONNECTOR_H
#define DBUS_CONNECTOR_H

#include <QObject>
#include <QThread>
#include <QDBusInterface>

namespace UkuiQuick {
class DbusConnectorPrivate;
/**
 * @brief DbusConnector是一个用于连接DBus服务的类
 * @details 该类继承自QThread,用于在后台线程中连接DBus服务
 * 连接成功后,会发送ready信号,参数为QDBusInterface指针
 * @note 该类会自动尝试连接DBus服务，当连接超时后，后台线程会保持监听目标服务的注册状态，目标服务注册成功后，会自动尝试连接
 * @note 该类不会管理QDBusInterface指针，需要使用者自行管理生命周期
 */
class DbusConnector : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param service 服务名
     * @param path 路径
     * @param interface 接口
     * @param conn 连接，默认为QDBusConnection::sessionBus()
     * @param parent 父对象
     */
    DbusConnector(const QString &service, const QString &path, const QString &interface, QDBusConnection conn = QDBusConnection::sessionBus(), QObject *parent = nullptr);
    ~DbusConnector() override;

Q_SIGNALS:
    /**
     * @brief 连接成功信号
     * @param interface QDBusInterface指针
     */
    void ready(QDBusInterface*);

protected:
    void run() override;

private:
    DbusConnectorPrivate *d = nullptr;
};
}


#endif //DBUS_CONNECTOR_H
