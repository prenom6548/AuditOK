/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "datetimeinterface.h"
#include <libintl.h>
#include <polkit-qt5-1/polkitqt1-authority.h>

DatetimeInterface::DatetimeInterface():UKUI::ServiceObject(nullptr)
{
    timeDateInterface = new QDBusInterface("org.freedesktop.timedate1",
                                       "/org/freedesktop/timedate1",
                                       "org.freedesktop.timedate1",
                                       QDBusConnection::systemBus());


    QDBusConnection::systemBus().connect("com.control.center.qt.systemdbus",
                                          "/",
                                          "com.control.center.interface",
                                          "changed",
                                          this,
                                          SLOT(timezoneChanged(QString)));
}


DatetimeInterface::~DatetimeInterface()
{

}

bool DatetimeInterface::timezoneChanged(QString key)
{
    if (key == "timezone") {
        Q_EMIT changed("timezone");
    }
    return true;
}

QStringList DatetimeInterface::getTimezone()
{
    QStringList list = {};
    QDBusInterface interface("org.freedesktop.timedate1",
                             "/org/freedesktop/timedate1",
                             "org.freedesktop.timedate1",
                             QDBusConnection::systemBus());

    QString timezoneT = interface.property("Timezone").toString(); // 系统配置时区
    QDBusReply<QString> reply = UniversalInterface::self()->ukccSystemDbus()->call("getShowTimezone"); //显示的时区 ./etc/kytimezone
    QString showTimezone = reply.value();
    if (!reply.isValid() || reply.value() != timezoneT) {
        // showTimezone 异常,重新设置
        if (!LINK_SHANGHAI_TIMEZONE.contains(showTimezone) || showTimezone.isEmpty()) {
            showTimezone = timezoneT;
            UniversalInterface::self()->ukccSystemDbus()->call("setShowTimezone", showTimezone);
        }
    }

    list.append(timezoneT);
    QStringList zone = timezoneName(showTimezone);
    if (zone.size() > 1) {
        list.append(zone.at(1));
    } else {
        list.append(timezoneT);
    }
    list.append(showTimezone);
    return list;
}

QStringList DatetimeInterface::timezoneName(QString timezone)
{
    QStringList result = {};
    result.append(timezone);

    const char kTimezoneDomain[] = "installer-timezones";
    const QString locale = QLocale::system().name();
    (void) setlocale(LC_ALL, QString(locale + ".UTF-8").toStdString().c_str());
    const QString local_name(dgettext(kTimezoneDomain,
                                      timezone.toStdString().c_str()));
    int index = local_name.lastIndexOf('/');
    if (index == -1) {
      // Some translations of locale name contains non-standard char.
      index = local_name.lastIndexOf("∕");
    }
    // Reset locale.
    (void) setlocale(LC_ALL, "en_US.UTF-8");
    result.append((index > -1) ? local_name.mid(index + 1) : local_name);
    return result;
}

QString DatetimeInterface::getTimeMode()
{
    bool syncFlag = timeDateInterface->property("NTP").toBool();
    return syncFlag ? QString("automatic") : QString ("manual");
}

bool DatetimeInterface::setTimezone(QString timezone)
{
    QDBusReply<bool> reply = UniversalInterface::self()->ukccSystemDbus()->call("setShowTimezone", timezone);
    return reply.isValid() && reply.value();
}

bool DatetimeInterface::setTime(qint64 timeData)
{
    const QDBusMessage reply = timeDateInterface->call("SetTime", timeData, false, true);
    return reply.type() == QDBusMessage::ReplyMessage;
}

bool DatetimeInterface::setTimeMode(QString mode)
{
    QDBusMessage retDBus;
    if (mode == "manual") {
        retDBus = timeDateInterface->call("SetNTP", false, true);
    } else {
        retDBus = timeDateInterface->call("SetNTP", true, true);
    }
    if (retDBus.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    Q_EMIT changed("timeMode");
    return true;
}

QStringList DatetimeInterface::getNtpServers()
{
    return QStringList()                    <<
           QString("pool.ntp.org")          <<
           QString("cn.ntp.org.cn")         <<
           QString("cn.pool.ntp.org")       <<
           QString("ntp.aliyun.com")        <<
           QString("0.debian.pool.ntp.org") <<
           QString("1.debian.pool.ntp.org") <<
           QString("0.arch.pool.ntp.org")   <<
           QString("1.arch.pool.ntp.org")   <<
           QString("0.fedora.pool.ntp.org") <<
           QString("1.fedora.pool.ntp.org");
}

QString DatetimeInterface::getNtpServer()
{
    QString ntpServer = "";
    const QString ntpFileName = "/etc/systemd/timesyncd.conf.d/kylin.conf";
    QFile ntpFile(ntpFileName);
    if (!ntpFile.exists()) {    // 系统默认
        ntpServer = "";
    } else {
        QSettings readFile(ntpFileName, QSettings::IniFormat);
        ntpServer = readFile.value("Time/NTP").toString();
    }
    return ntpServer;
}

bool DatetimeInterface::setNtpServer(QString server)
{
    PolkitQt1::Authority::Result result;
    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.ntp",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes) {
        return true;
    }
    return false;
}

QStringList DatetimeInterface::getOtherTimezones()
{
    if (UniversalInterface::self()->ukccPanelGsettings()->keys().contains(TIMEZONES_KEY)) {
        return UniversalInterface::self()->ukccPanelGsettings()->get(TIMEZONES_KEY).toStringList();
    } else {
        return QStringList();
    }
}

bool DatetimeInterface::setOtherTimezones(QStringList list)
{
    if (UniversalInterface::self()->ukccPanelGsettings()->keys().contains(TIMEZONES_KEY)) {
        UniversalInterface::self()->ukccPanelGsettings()->set(TIMEZONES_KEY, list);
        Q_EMIT changed("otherTimezones");
        return true;
    } else {
        return false;
    }
}
