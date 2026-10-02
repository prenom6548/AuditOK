/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QTimer>
#include <QDBusReply>
#include <QSettings>
#include <QFileSystemWatcher>

extern "C"
{
    #include <string>
    #include <glib.h>
    #include <glib/gprintf.h>
}

#include "CSingleton.h"

//全局变量，是否是华为机器, 默认false, 在config.cpp文件定义
enum Environment
{
    NOMAL = 0,
    HUAWEI,
    LAIKA,
    MAVIS
};

//gsetting name define


#define GSETTING_SCHEMA_UKUIBLUETOOH "org.ukui.bluetooth"
#define GSETTING_SCHEMA_UKCC "org.ukui.control-center.plugins"
#define GSETTING_PACH_UKCC "/org/ukui/control-center/plugins/Bluetooth/"


#define USDDBUSSERVICE             "com.kylin.ukui.SettingsDaemon"
#define USDDBUSPATH                "/globalconfig"
#define USDDBUSINTERFACE           "com.kylin.ukui.SettingsDaemon.interface"

extern Environment envPC;

class Config : public QObject
{
public:
    Config(QObject *parent = nullptr);
    ~Config();

    int init(void);

    bool power_switch(void){ return m_power_switch; }
    void power_switch(bool);

    bool discoverable_switch(void){ return m_discoverable_switch; }
    void discoverable_switch(bool);

    QStringList finally_audiodevice(void){ return m_finally_audiodevices; }
    void finally_audiodevice(QString);
    void remove_finally_audiodevice(QString);

    QString adapter_addr(void){ return m_adapter_addr; }
    void adapter_addr(QString);

    QString file_save_path(void){ return m_file_save_path; }
    void file_save_path(QString);

    bool activeconnection(void){ return m_activeconnection; }
    void activeconnection(bool);

    bool trayShow(void){ return m_tray_show; }
    void trayShow(bool);

    QString loglevel(void);
    void loglevel(QString);
    void loglevel(int);

    bool leaveLock(void) { return m_leaveLock; }
    void leaveLock(bool);

    int flightMode(void);
    void flightMode(int);

    bool flightBonding() { return m_flightBonding; }
    void flightBonding(bool);

    bool getProperFlightMode(void);

    /////////////////////////////////////////////////////////

    bool bluetooth_block(void){ return m_bluetooth_block; }
    void bluetooth_block(bool v){ m_bluetooth_block = v; }

    bool systemSleepFlag(void){ return m_systemSleepFlag; }
    void systemSleepFlag(bool v){ m_systemSleepFlag = v; }

    bool showBluetooth(void){ return m_showBluetooth; }
    void showBluetooth(bool v){ m_showBluetooth = v; }

    QString get_active_user(void) { return m_active_user; }
    QString active_user(void);
    void active_user(QString v){ m_active_user = v; }

    void set_rename(QString, QString);
    QString get_rename(QString);
    void delete_rename(QString);


    bool AudioCombine(void) {return m_audioCombine; }
    void AudioCombine(bool v);

protected slots:

private:
    void init_rename(void);

    void init_conf(void);

protected:
    QString getActiveUser(void);

    int getFlightMode(void);

    QSettings * m_ukui_bluetooth = nullptr;

private:
    bool m_power_switch = false;

    bool m_discoverable_switch = true;

    QStringList m_finally_audiodevices;

    QString m_adapter_addr;

    QString m_file_save_path;

    bool m_activeconnection = false;

    bool m_tray_show = true;

    bool m_audioCombine = false;

    //debug, info, warning
    QString m_loglevel = "info";

    bool m_leaveLock = false;

    //内部属性
    bool m_bluetooth_block = false;

    bool m_systemSleepFlag = false;

    bool m_showBluetooth = false;

    QString m_active_user;

    //飞行模式, 1 飞行模式开启，  0： 飞行模式关闭， -1 不支持飞行模式
    int m_flightMode = -1;

    bool m_flightBonding = true;

    //别名
    QMap<QString, QString> m_rename;

    friend class SingleTon<Config>;
};

typedef SingleTon<Config>  CONFIG;
#endif // CONFIG_H
