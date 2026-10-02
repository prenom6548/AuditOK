/*
 * UKUI Framework
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "datetime.h"




DateTime::DateTime(QObject *parent)
    : QObject{parent}
{
    m_areaDBusManager = new QDBusInterface("org.ukui.ukcc.session",
                                           "/Area",
                                           "org.ukui.ukcc.session.Area",
                                           QDBusConnection::sessionBus(),
                                           this);

    m_dateDBusManager = new QDBusInterface("org.ukui.ukcc.session",
                                           "/Datetime",
                                           "org.ukui.ukcc.session.Datetime",
                                           QDBusConnection::sessionBus(),
                                           this);
}

DateTime::~DateTime()
{

}

const QString DateTime::GetTimeZone() const
{
    return m_dateDBusManager->property("timezone").toStringList().first();
}

void DateTime::SetTimeZone(const QString &timezone)
{
    m_dateDBusManager->call("setTimezone", timezone);
    Q_EMIT TimeZoneChanged();
}

const QString DateTime::GetFormatCountry() const
{
    return m_areaDBusManager->property("formatCountry").toString();
}

void DateTime::SetFormatCountry(const QString &formatCountry)
{
    m_areaDBusManager->call("setFormatCountry", formatCountry);
    Q_EMIT FormatCountryChanged();
}

const QString DateTime::GetCalendar() const
{
    return m_areaDBusManager->property("calendar").toString();
}

void DateTime::SetCalendar(const QString &calendar)
{
    m_areaDBusManager->call("setCalendar", calendar);
    Q_EMIT CalendarChanged();
}

const QString DateTime::GetFirstDay() const
{
    return m_areaDBusManager->property("firstDay").toString();
}

void DateTime::SetFirstDay(const QString &firstDay)
{
    m_areaDBusManager->call("setFirstDay", firstDay);
    Q_EMIT FirstDayChanged();
}

const QString DateTime::GetTimeFormat() const
{
    return m_areaDBusManager->property("timeFormat").toString();
}

void DateTime::SetTimeFormat(const QString &timeFormat)
{
    m_areaDBusManager->call("setTimeFormat", timeFormat);
    Q_EMIT TimeFormatChanged();
}


