/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QDBusVariant>
#include <QDBusContext>
#include <QSettings>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QDir>

#include "clib-syslog.h"
#include "usd_base_class.h"
#include "dbus-security.h"

class GlobalConfig : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface",SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE)

public:
    explicit GlobalConfig(QObject *parent = nullptr);
    ~GlobalConfig();

public Q_SLOTS:
    /**
     * @brief getGlobalConf 获取全局配置
     * @param schema 主题
     * @param key 键值
     * @return
     */
    Q_SCRIPTABLE QDBusVariant getGlobalConf(QString schema, QString key);
    /**
     * @brief setGlobalConf 设置全局配置
     * @param schema 主题
     * @param key 键值
     * @param value 值
     * @return
     */
    Q_SCRIPTABLE bool setGlobalConf(QString schema, QString key, QDBusVariant value);

    /**
     * @brief writeGlobalConfig
     * @param path,路径以及文件名
     * @param info,文件内容
     */
    Q_SCRIPTABLE bool writeGlobalConfig(QString path, QString fileName, QByteArray info);

    /**
     * @brief writeGlobalConfig
     * @param path,路径以及文件名
     * @param info,文件内容
     */
    Q_SCRIPTABLE QByteArray readGlobalConfig(QString path);

    /**
     * @brief writeGlobalConfig
     * @param path,路径以及文件名
     * @param info,文件内容
     */
    Q_SCRIPTABLE bool globalConfigExists(QString configPath);

    /**
     * @brief getGlobalConf 获取全局配置相关主题的全部配置
     * @param schema 主题
     * @return
     */
    QVariantMap getSchemaGlobalConf(QString schema);
    /**
     * @brief getGlobalConf 设置全局配置相关主题的多项配置
     * @param schema 主题
     * @return
     */
    bool setSchemaGlobalConf(QString schema, QVariantMap &map);

    /**
     * @brief IsDeviceSupportFlightMode
     * @return 设备是否支持飞行模式;
     */
    Q_INVOKABLE bool IsSupportFlightMode();
    Q_INVOKABLE void SetFlightModeState(int state);
    Q_INVOKABLE int GetFlightModeState();

Q_SIGNALS:
    /**
     * @brief globalConfChanged
     * @param schema
     * @param key
     * @param value
     * 是否需要此信号？？或许多用户时需要。。
     */
    void globalConfChanged(QString schema, QString key, QDBusVariant value);

    void FlightModeChanged(int state);

private:
    void ensureDirectoryExists(QString dir);

    void ensureGlobalDirectory();

    void initFlghtMode();

    QVariant getGlobalConfig(QString schema, QString key);

private:
    int FlightModeState = -1;

    Q_PROPERTY(int FlightModeState READ GetFlightModeState)
};

#endif // GLOBALSETTINGS_H
