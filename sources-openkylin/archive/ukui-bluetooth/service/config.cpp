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

#include "config.h"
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <libkysysinfo.h>

Environment envPC = Environment::NOMAL;

#define CONF_FILE_FULLPATH                      "/etc/bluetooth/ukui-bluetooth.conf"
//#define CONF_FILE_FULLPATH                      "./ukui-bluetooth.conf"

#define CONF_STR_BLUETOOTH_POWER_SWTICH          "switch"
#define CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH   "bluetoothDiscoverableSwitch"
#define CONF_STR_FINALLY_CONNECT_THE_DEVICE      "finallyConnectTheDevice"
#define CONF_STR_ADAPTER_ADDRESS                 "adapterAddress"
#define CONF_STR_FILE_SAVE_PATH                  "fileSavePath"
#define CONF_STR_ADAPTER_ADDRESS_LIST            "adapterAddressList"
#define CONF_STR_ACTIVE_CONNECTION               "activeConnection"
#define CONF_STR_TRAY_SHOW                       "trayShow"
#define CONF_STR_AUDIO_COMBINE                   "audioCombine"
#define CONF_STR_LEAVE_LOCK                      "leaveLock"
#define CONF_STR_FLIGHT_BONDING                  "flightBonding"

#define CONF_LOG_LEVEL                           "loglevel"


Config::Config(QObject *parent)
    :QObject(parent)
{
    m_ukui_bluetooth = new QSettings(CONF_FILE_FULLPATH, QSettings::IniFormat);
    QStringList groups = m_ukui_bluetooth->childGroups();
    KyInfo() << groups;
    if(groups.indexOf("conf") == -1) {
        KyInfo() << "create and init conf";
        m_ukui_bluetooth->beginGroup("conf");
        this->init_conf();
    } else {
        m_ukui_bluetooth->beginGroup("conf");
        this->init_conf();
    }
}

Config::~Config()
{
    delete m_ukui_bluetooth;
    m_ukui_bluetooth = nullptr;
}

void Config::init_conf()
{
    KyInfo();
    if(!m_ukui_bluetooth->contains(CONF_STR_BLUETOOTH_POWER_SWTICH)) {
        KyInfo() << "add conf: " << CONF_STR_BLUETOOTH_POWER_SWTICH;
        m_ukui_bluetooth->setValue(CONF_STR_BLUETOOTH_POWER_SWTICH, false);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH)) {
        KyInfo() << "add conf: " << CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH;
        m_ukui_bluetooth->setValue(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH, true);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_FINALLY_CONNECT_THE_DEVICE)) {
        KyInfo() << "add conf: " << CONF_STR_FINALLY_CONNECT_THE_DEVICE;
        m_ukui_bluetooth->setValue(CONF_STR_FINALLY_CONNECT_THE_DEVICE, "");
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_ADAPTER_ADDRESS)) {
        KyInfo() << "add conf: " << CONF_STR_ADAPTER_ADDRESS;
        m_ukui_bluetooth->setValue(CONF_STR_ADAPTER_ADDRESS, "");
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_FILE_SAVE_PATH)) {
        KyInfo() << "add conf: " << CONF_STR_FILE_SAVE_PATH;
        m_ukui_bluetooth->setValue(CONF_STR_FILE_SAVE_PATH, "");
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_ADAPTER_ADDRESS_LIST)) {
        KyInfo() << "add conf: " << CONF_STR_ADAPTER_ADDRESS_LIST;
        QStringList t;
        t.append("null");
        m_ukui_bluetooth->setValue(CONF_STR_ADAPTER_ADDRESS_LIST, t);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_ACTIVE_CONNECTION)) {
        KyInfo() << "add conf: " << CONF_STR_ACTIVE_CONNECTION;
        m_ukui_bluetooth->setValue(CONF_STR_ACTIVE_CONNECTION, false);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_TRAY_SHOW)) {
        KyInfo() << "add conf: " << CONF_STR_TRAY_SHOW;
        m_ukui_bluetooth->setValue(CONF_STR_TRAY_SHOW, true);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_AUDIO_COMBINE)) {
        KyInfo() << "add conf: " << CONF_STR_AUDIO_COMBINE;
        m_ukui_bluetooth->setValue(CONF_STR_AUDIO_COMBINE, false);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_LEAVE_LOCK)) {
        KyInfo() << "add conf: " << CONF_STR_LEAVE_LOCK;
        m_ukui_bluetooth->setValue(CONF_STR_LEAVE_LOCK, false);
    }

    if(!m_ukui_bluetooth->contains(CONF_LOG_LEVEL)) {
        KyInfo() << "add conf: " << CONF_LOG_LEVEL;
        m_ukui_bluetooth->setValue(CONF_LOG_LEVEL, m_loglevel);
    }

    if(!m_ukui_bluetooth->contains(CONF_STR_FLIGHT_BONDING)) {
        KyInfo() << "add conf: " << CONF_STR_FLIGHT_BONDING;
        m_ukui_bluetooth->setValue(CONF_STR_FLIGHT_BONDING, true);
    }
}

int Config::init()
{
    if(m_ukui_bluetooth->contains(CONF_LOG_LEVEL)) {
        m_loglevel = m_ukui_bluetooth->value(CONF_LOG_LEVEL).toString();
        KyInfo() << CONF_LOG_LEVEL << m_loglevel;
        set_logLevel(m_loglevel);
    }

    if(m_ukui_bluetooth->contains(CONF_STR_BLUETOOTH_POWER_SWTICH)) {
        m_power_switch = m_ukui_bluetooth->value(CONF_STR_BLUETOOTH_POWER_SWTICH).toBool();
        KyInfo() << CONF_STR_BLUETOOTH_POWER_SWTICH << m_power_switch;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH)) {
        m_discoverable_switch = m_ukui_bluetooth->value(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH).toBool();
        KyInfo() << CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH << m_discoverable_switch;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_FINALLY_CONNECT_THE_DEVICE)) {
        m_finally_audiodevices = m_ukui_bluetooth->value(CONF_STR_FINALLY_CONNECT_THE_DEVICE).toStringList();
        KyInfo() << CONF_STR_FINALLY_CONNECT_THE_DEVICE << m_finally_audiodevices;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_ADAPTER_ADDRESS)) {
        m_adapter_addr = m_ukui_bluetooth->value(CONF_STR_ADAPTER_ADDRESS).toString();
        KyInfo() << CONF_STR_ADAPTER_ADDRESS << m_adapter_addr;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_FILE_SAVE_PATH)) {
        m_file_save_path = m_ukui_bluetooth->value(CONF_STR_FILE_SAVE_PATH).toString();
        KyInfo() << CONF_STR_FILE_SAVE_PATH << m_file_save_path;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_ACTIVE_CONNECTION)) {
        m_activeconnection = m_ukui_bluetooth->value(CONF_STR_ACTIVE_CONNECTION).toBool();
        KyInfo() << CONF_STR_ACTIVE_CONNECTION << m_activeconnection;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_TRAY_SHOW)) {
        m_tray_show = m_ukui_bluetooth->value(CONF_STR_TRAY_SHOW).toBool();
        KyInfo() << CONF_STR_TRAY_SHOW << m_tray_show;
    }

    if(m_ukui_bluetooth->contains(CONF_LOG_LEVEL)) {
        m_loglevel = m_ukui_bluetooth->value(CONF_LOG_LEVEL).toString();
        KyInfo() << CONF_LOG_LEVEL << m_loglevel;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_AUDIO_COMBINE)) {
        m_audioCombine = m_ukui_bluetooth->value(CONF_STR_AUDIO_COMBINE).toBool();
        KyInfo() << CONF_STR_AUDIO_COMBINE << m_audioCombine;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_LEAVE_LOCK)) {
        m_leaveLock = m_ukui_bluetooth->value(CONF_STR_LEAVE_LOCK).toBool();
        KyInfo() << CONF_STR_LEAVE_LOCK << m_audioCombine;
    }

    if(m_ukui_bluetooth->contains(CONF_STR_FLIGHT_BONDING)) {
        m_flightBonding = m_ukui_bluetooth->value(CONF_STR_FLIGHT_BONDING).toBool();
        KyInfo() << CONF_STR_FLIGHT_BONDING << m_flightBonding;
    }

    m_active_user = this->getActiveUser();
    m_flightMode = this->getFlightMode();

    KyInfo()<< "active user: "<< m_active_user << " flightMode: " << m_flightMode;

    this->init_rename();
    return 0;
}

void Config::power_switch(bool v)
{
    m_power_switch = v;
    m_ukui_bluetooth->setValue(CONF_STR_BLUETOOTH_POWER_SWTICH, v);
}

void Config::discoverable_switch(bool v)
{
    m_discoverable_switch = v;
    m_ukui_bluetooth->setValue(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH, v);
}

void Config::finally_audiodevice(QString v)
{
    if(!m_finally_audiodevices.contains(v)) {
        while (m_finally_audiodevices.size() >= 2) {
            m_finally_audiodevices.removeFirst();
        }
        m_finally_audiodevices.append(v);
        m_ukui_bluetooth->setValue(CONF_STR_FINALLY_CONNECT_THE_DEVICE, m_finally_audiodevices);
    }
}

void Config::remove_finally_audiodevice(QString v)
{
    if(m_finally_audiodevices.contains(v)) {
        m_finally_audiodevices.removeOne(v);
        m_ukui_bluetooth->setValue(CONF_STR_FINALLY_CONNECT_THE_DEVICE, m_finally_audiodevices);
    }
}

void Config::adapter_addr(QString v)
{
    m_adapter_addr = v;
    m_ukui_bluetooth->setValue(CONF_STR_ADAPTER_ADDRESS, v);
}

void Config::file_save_path(QString v)
{
    m_file_save_path = v;
    m_ukui_bluetooth->setValue(CONF_STR_FILE_SAVE_PATH, v);
}

void Config::activeconnection(bool v)
{
    m_activeconnection = v;
    m_ukui_bluetooth->setValue(CONF_STR_ACTIVE_CONNECTION, v);
}

void Config::trayShow(bool v)
{
    m_tray_show = v;
    m_ukui_bluetooth->setValue(CONF_STR_TRAY_SHOW, v);
}

void Config::loglevel(QString v)
{
    set_logLevel(v);
    m_loglevel = v;
    m_ukui_bluetooth->setValue(CONF_LOG_LEVEL, m_loglevel);
}

void Config::loglevel(int v)
{
    set_logLevel(v);

    if (0 == v) {
        m_loglevel = "debug";
    } else if ( 1== v) {
        m_loglevel = "info";
    } else if (2 == v) {
        m_loglevel = "warning";
    }

    m_ukui_bluetooth->setValue(CONF_LOG_LEVEL, m_loglevel);
}

QString Config::active_user()
{
    if (m_active_user.length() == 0) {
        m_active_user = this->getActiveUser();
    }
    return m_active_user;
}

void Config::leaveLock(bool v)
{
    m_leaveLock = v;
    m_ukui_bluetooth->setValue(CONF_STR_LEAVE_LOCK, v);
}

int Config::flightMode()
{
    return m_flightMode;
}

void Config::flightMode(int v)
{
    KyInfo() << v;
    m_flightMode = v;
}

void Config::flightBonding(bool v)
{
    KyInfo() << v;
    m_flightBonding = v;
    m_ukui_bluetooth->setValue(CONF_STR_FLIGHT_BONDING, v);
}

bool Config::getProperFlightMode()
{
    bool ret = false;
    if (m_bluetooth_block) {
        ret = true;
        goto END;
    }

    if (m_flightBonding) {
        if (1 == m_flightMode) {
            ret = true;
        }
    }

END:
    KyInfo() << ret;
    return ret;
}


void Config::set_rename(QString addr, QString name)
{
    m_rename[addr] = name;
    QSettings s(CONF_FILE_FULLPATH, QSettings::IniFormat);
    s.beginGroup("rename");
    s.setValue(addr, name);
    s.endGroup();
}

QString Config::get_rename(QString v)
{
    if(m_rename.contains(v)) {
        return m_rename[v];
    }
    return "";
}

void Config::delete_rename(QString addr)
{
    if(m_rename.contains(addr)) {
        QSettings s(CONF_FILE_FULLPATH, QSettings::IniFormat);
        s.beginGroup("rename");
        s.remove(addr);
        s.endGroup();
        m_rename.remove(addr);
    }
}

void Config::AudioCombine(bool v)
{
    m_audioCombine = v;
    m_ukui_bluetooth->setValue(CONF_STR_AUDIO_COMBINE, m_audioCombine);
}


void Config::init_rename()
{
    QSettings s(CONF_FILE_FULLPATH, QSettings::IniFormat);
    s.beginGroup("rename");
    QStringList keys = s.allKeys();
    for(auto iter : keys) {
        m_rename[iter] = s.value(iter).toString();
    }
    KyDebug() << m_rename;
    s.endGroup();
}


QString Config::getActiveUser()
{
    QString user;
    char * euser = kdk_system_get_eUser();
    if (NULL != euser) {
        user = euser;
        free(euser);
        euser = NULL;
    }

    KyInfo() << "active user: " << user;

    return user;
}

int Config::getFlightMode()
{
    int ret = -1;
    QDBusInterface iface(USDDBUSSERVICE, USDDBUSPATH, USDDBUSINTERFACE, QDBusConnection::systemBus());
    QDBusMessage res = iface.call("GetFlightModeState");
    if (res.type() == QDBusMessage::ReplyMessage) {
        if(res.arguments().size() > 0) {
            ret = res.arguments().takeFirst().toInt();
        }
    } else {
        qWarning()<< res.errorName() << ": "<< res.errorMessage();
    }

    KyInfo() << "FlightMode: " << ret;

    return ret;
}









