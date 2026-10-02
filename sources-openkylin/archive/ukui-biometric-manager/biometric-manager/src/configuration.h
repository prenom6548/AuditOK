/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>

class Configuration : public QObject
{
    Q_OBJECT
private:
    explicit Configuration(QObject *parent = nullptr);
    Configuration(const Configuration &config) = delete;
    Configuration &operator=(const Configuration &rhs) = delete;

public:
    static QString configFile;

    static Configuration *instance();
    QString getDefaultDevice();
    bool getLoginSwitch();
    bool getSaverSwitch();
    bool getSystemSwitch();
    void setDefaultDevice(const QString &deviceName);
    void setLoginSwitch(bool status);
    void setSystemSwitch(bool status);
    void setSaverSwitch(bool status);
    QString get_current_version();
    QString get_same_str_start(QString &str1, QString &str2);
    bool isNum(QString str);

private:
    static Configuration *instance_;

signals:
    void defaultDeviceChanged(const QString &deviceName);
};

#endif // CONFIGURATION_H
