/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef DBUS-ADAPTOR_H
#define DBUS-ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
//#include "ukmedia_main_widget.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface org.ukui.media
 */
class MediaAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.media")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.ukui.media\">\n"
"    <method name=\"getDefaultOutputDevice\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"getDefaultInputDevice\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"getAllOutputDevices\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"getAllInputDevices\">\n"
"      <arg direction=\"out\" type=\"as\"/>\n"
"    </method>\n"
"    <method name=\"getDefaultOutputVolume\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"getDefaultInputVolume\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"getDefaultOutputMuteState\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"getDefaultInputMuteState\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"setDefaultOutputVolume\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"setDefaultInputVolume\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"value\"/>\n"
"    </method>\n"
"    <method name=\"setDefaultOutputMuteState\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"mute\"/>\n"
"    </method>\n"
"    <method name=\"setDefaultInputMuteState\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"b\" name=\"mute\"/>\n"
"    </method>\n"
"    <method name=\"setDefaultOutputDevice\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"deviceName\"/>\n"
"    </method>\n"
"    <method name=\"setDefaultInputDevice\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"deviceName\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.ukui.media.bluetooth\">\n"
"    <method name=\"getBatteryLevel\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"deviceName\"/>\n"
"    </method>\n"
"    <signal name=\"batteryChanged\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"device\"/>\n"
"      <arg direction=\"in\" type=\"i\" name=\"battery\"/>\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    MediaAdaptor(QObject *parent);
    virtual ~MediaAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QStringList getAllInputDevices();
    QStringList getAllOutputDevices();
    QString getDefaultInputDevice();
    bool getDefaultInputMuteState();
    int getDefaultInputVolume();
    QString getDefaultOutputDevice();
    bool getDefaultOutputMuteState();
    int getDefaultOutputVolume();
    bool setDefaultInputDevice(const QString &deviceName);
    bool setDefaultInputMuteState(bool mute);
    bool setDefaultInputVolume(int value);
    bool setDefaultOutputDevice(const QString &deviceName);
    bool setDefaultOutputMuteState(bool mute);
    bool setDefaultOutputVolume(int value);
    int getBatteryLevel(const QString &deviceName);
Q_SIGNALS: // SIGNALS
    void batteryChanged(const QString &device, int battery);
};

#endif
