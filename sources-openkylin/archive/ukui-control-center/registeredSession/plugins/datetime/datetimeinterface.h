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
#ifndef DATETIMEINTERFACE_H
#define DATETIMEINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QtDBus>
#include "../../universalinterface.h"

class DatetimeInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Datetime")
    Q_PROPERTY(QStringList timezone READ getTimezone)
    Q_PROPERTY(QString timeMode READ getTimeMode)
    Q_PROPERTY(QStringList ntpServers READ getNtpServers)
    Q_PROPERTY(QString ntpServer READ getNtpServer)
    Q_PROPERTY(QStringList otherTimezones READ getOtherTimezones)

public:
    DatetimeInterface();
    ~DatetimeInterface();
    QStringList getTimezone();
    QString getTimeMode();
    QStringList getNtpServers();
    QString getNtpServer();
    QStringList getOtherTimezones();

public Q_SLOTS:
    bool setTimezone(QString zone);
    bool setTime(qint64 timeData);
    bool setTimeMode(QString mode);
    bool setNtpServer(QString server);
    bool setOtherTimezones(QStringList list);
    QStringList timezoneName(QString timezone);

private Q_SLOTS:
    bool timezoneChanged(QString key);

private:
    QDBusInterface *timeDateInterface = nullptr;
    const QStringList LINK_SHANGHAI_TIMEZONE = {
        "Asia/Beijing",
        "Asia/Urumqi"
    };
    const QString TIMEZONES_KEY = QString("timezones");

Q_SIGNALS:
    void changed(QString key);
};

#endif // DATETIMEINTERFACE_H
