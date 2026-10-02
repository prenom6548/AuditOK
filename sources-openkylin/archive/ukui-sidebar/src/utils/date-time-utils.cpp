/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#include "date-time-utils.h"
#include "libkydate.h"

#include <QVariant>
#include <QDBusConnection>
#include <QDebug>
using namespace Sidebar;

#define HOUR_SYSTEM_CONTROL "org.ukui.control-center.panel.plugins"
#define DATA_FORMAT     "date"          //日期格式：yyyy/MM/dd、yyyy-MM-dd
#define TIME_FORMAT     "hoursystem"    //时间格式：12小时制、24小时制
#define KYSDK_TIMERSERVER "com.kylin.kysdk.TimeServer"
#define KYSDK_TIMERPATH "/com/kylin/kysdk/Timer"
#define KYSDK_TIMERINTERFACE "com.kylin.kysdk.TimeInterface"

DateTimeUtils::DateTimeUtils(QObject *parent) :
               QObject(parent),
               m_dataFormat("cn"),
               m_hourSystem("24")
{
    initGsettings();
    //使用系统提供的sdk刷新时间显示
    QDBusConnection::systemBus().connect(KYSDK_TIMERSERVER,
                                         KYSDK_TIMERPATH,
                                         KYSDK_TIMERINTERFACE,
                                         "TimeChangeSignal",
                                         this,
                                         SLOT(timeSignalSlot()));
    bool success = false;
    success = QDBusConnection::systemBus().connect(KYSDK_TIMERSERVER,
                                                   KYSDK_TIMERPATH,
                                                   KYSDK_TIMERINTERFACE,
                                                   "TimeSignal",
                                                   this,
                                                   SLOT(timeSignalSlot()));
    if(!success) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);
        connect(m_timer, &QTimer::timeout, this, &DateTimeUtils::timeSignalSlot);
        m_timer->start();
    }

    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "ShortDateSignal",
                                          this, SLOT(setShortDateFormat(QString)));
    setShortDateFormat();
}

DateTimeUtils::~DateTimeUtils()
{
    if(m_timeGsettings) {
        delete m_timeGsettings;
        m_timeGsettings = nullptr;
    }
}

QString DateTimeUtils::currentTime()
{
    if(m_hourSystem == "24") {
       return QDateTime::currentDateTime().toString("HH:mm");
    } else {
        return QDateTime::currentDateTime().toString("AP hh:mm");
    }
}

QString DateTimeUtils::currentDate()
{
    if(m_dataFormat == "cn") {
        return QDateTime::currentDateTime().toString("MM/dd");
    } else {
        return QDateTime::currentDateTime().toString("MM-dd");
    }
}

QString DateTimeUtils::currentWeekDay()
{
    return QDateTime::currentDateTime().toString("ddd");
}

void DateTimeUtils::timeSignalSlot()
{
    Q_EMIT timeUpdate(currentTime());
    Q_EMIT dateUpdate(currentDate());
    Q_EMIT weekDayUpdate(currentWeekDay());

    Q_EMIT timeRefresh();
}

void DateTimeUtils::setShortDateFormat(QString shortDateFormat)
{
    if (shortDateFormat == "") {
        char* date = kdk_system_get_shortformat();
        shortDateFormat = date;
        free(date);
    }

    if (shortDateFormat == "") {
        shortDateFormat = "yyyy/MM/dd";
    }
    if (shortDateFormat == m_shortDateFormat) return;

    m_shortDateFormat = shortDateFormat;
    Q_EMIT timeRefresh();
}

QString DateTimeUtils::computeTimeOut(QDateTime timeStamp)
{
    QDateTime currentDateTime(QDateTime::currentDateTime());
    int dateDiff = currentDateTime.date().toJulianDay() - timeStamp.date().toJulianDay();
    // 一分钟之内
    if (timeStamp <= currentDateTime && currentDateTime < timeStamp.addSecs(60)) {
        return QString(tr("Now"));
        // 一天之内
    } else if (dateDiff == 0 && timeStamp <= currentDateTime ) {
        if (m_hourSystem == "24") {
            return timeStamp.toString("HH:mm");
        } else {
            return timeStamp.toString("AP h:mm");
        }
        // 一天
    } else if (dateDiff == 1) {
        if (m_hourSystem == "24") {
            return QString(tr("Yesterday ")).append(timeStamp.toString("HH:mm"));
        } else {
            return QString(tr("Yesterday ")).append(timeStamp.toString("AP h:mm"));
        }
        // 大于一天且在一周之内
    } else if (1 < dateDiff && 7 > dateDiff) {
        if(m_hourSystem == "24") {
            return timeStamp.toString("ddd HH:mm");
        } else {
            return timeStamp.toString("ddd AP h:mm");
        }
        // 超过一周
    } else if (dateDiff >= 7) {
        return timeStamp.toString(m_shortDateFormat);
        // 时间设置为过去
    } else if (currentDateTime < timeStamp) {
        return timeStamp.toString(m_shortDateFormat);
        // 其他情况
    } else {
        if (m_hourSystem == "24") {
            return timeStamp.toString(m_shortDateFormat + " HH:mm");
        } else {
            return timeStamp.toString(m_shortDateFormat + " AP h:mm");
        }
    }
}

void DateTimeUtils::initGsettings()
{
    const QByteArray id(HOUR_SYSTEM_CONTROL);
    if (QGSettings::isSchemaInstalled(id)) {
        m_timeGsettings = new QGSettings(id);
        connect(m_timeGsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == DATA_FORMAT) {
                m_dataFormat = m_timeGsettings->get(DATA_FORMAT).toString();
                Q_EMIT dateUpdate(currentDate());
            } else if(key == TIME_FORMAT) {
                m_hourSystem = m_timeGsettings->get(TIME_FORMAT).toString();
                Q_EMIT timeUpdate(currentTime());
            }
        });

        QStringList ketList = m_timeGsettings->keys();
        if(ketList.contains(DATA_FORMAT))
            m_dataFormat = m_timeGsettings->get(DATA_FORMAT).toString();
        if(ketList.contains(TIME_FORMAT))
            m_hourSystem = m_timeGsettings->get(TIME_FORMAT).toString();
    }
}
