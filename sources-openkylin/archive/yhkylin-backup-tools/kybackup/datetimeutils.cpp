/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "datetimeutils.h"
#include <QDBusReply>
#include <QDebug>

DateTimeUtils::DateTimeUtils(QObject *parent /*= nullptr*/) :
    QObject(parent)
{
    //监听系统时间格式转换
    m_dateSessionDbus = new QDBusInterface("com.kylin.kysdk.DateServer",
                                             "/com/kylin/kysdk/Date",
                                             "com.kylin.kysdk.DateInterface",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (m_dateSessionDbus->isValid()) {
        connect(m_dateSessionDbus, SIGNAL(DateSignal(QString)), this, SLOT(DateFormatChange(QString)));
        connect(m_dateSessionDbus, SIGNAL(ShortDateSignal(QString)), this, SLOT(DateFormatChange(QString)));
        connect(m_dateSessionDbus, SIGNAL(LongDateSignal(QString)), this, SLOT(DateFormatChange(QString)));
        connect(m_dateSessionDbus, SIGNAL(TimeSignal(QString)), this, SLOT(TimeFormatChange(QString)));
    }
    m_dateFormatOld = "yyyy-MM-dd";
    m_dateFormatNow = kdk_system_get_shortformat();
    m_timeFormatOld = "hh:mm:ss";
    QString newTimeFormat(kdk_system_get_now_timeformat());
    if (newTimeFormat.contains("12")) {
        m_timeFormatNow = "ap hh:mm:ss";
    } else {
        m_timeFormatNow = "hh:mm:ss";
    }
}

void DateTimeUtils::DateFormatChange(QString dateFormat)
{
    Q_UNUSED(dateFormat)
    QString shortDateFormat(kdk_system_get_shortformat());
    if (m_dateFormatNow != shortDateFormat) {
        m_dateFormatOld = m_dateFormatNow;
        m_dateFormatNow = shortDateFormat;
        emit this->ShortDateSignal();
    }
}

void DateTimeUtils::TimeFormatChange(QString timeFormat)
{
    QString newTimeFormat;
    if (timeFormat.contains("12")) {
        newTimeFormat = "ap hh:mm:ss";
    } else {
        newTimeFormat = "hh:mm:ss";
    }

    if (newTimeFormat != m_timeFormatNow) {
        m_timeFormatOld = m_timeFormatNow;
        m_timeFormatNow = newTimeFormat;
        emit this->ShortDateSignal();
    }
}

QString DateTimeUtils::TranslateDateFormat(const QString &dateTime, const QString &oldFormat /*= "yyyy-MM-dd hh:mm:ss"*/)
{
    QDateTime qDateTime = QDateTime::fromString(dateTime, oldFormat);
    if (!qDateTime.isValid())
        return dateTime;

    QString newFormat = m_dateFormatNow + " " + m_timeFormatNow;
    return qDateTime.toString(newFormat);
}

struct tm DateTimeUtils::QDateTimeToCtm(const QDateTime &datetime)
{
    struct tm c_time;
    c_time.tm_year = datetime.date().year();
    c_time.tm_mon = datetime.date().month();
    c_time.tm_mday = datetime.date().day();
    c_time.tm_hour = datetime.time().hour();
    c_time.tm_min = datetime.time().minute();
    c_time.tm_sec = datetime.time().second();

    return c_time;
}

QString DateTimeUtils::TranslateDateFormat(const QDateTime &datetime)
{
    struct tm c_time = QDateTimeToCtm(datetime);
    QString result = kdk_system_shortformat_transform(&c_time);
    result += " ";
    // result += datetime.time().toString(m_timeFormatNow);
    kdk_timeinfo * kdk_time = kdk_system_timeformat_transform(&c_time);
    QString time;
    if (kdk_time) {
        result += kdk_time->time;
        kdk_free_timeinfo(kdk_time);
    }

    return result;
}

DateTimeUtils::~DateTimeUtils()
{
    if (m_dateSessionDbus)
        delete m_dateSessionDbus;
}

