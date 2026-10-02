/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "globaldateformat.h"
#include "libkydate.h"
#include "globalutils.h"
#include <QDateTime>
#include <QDebug>

GlobalDateFormat::GlobalDateFormat()
{
    init();
}

GlobalDateFormat::~GlobalDateFormat()
{
    if (m_dateFormatSessionDbus)
        delete m_dateFormatSessionDbus;
}

GlobalDateFormat *GlobalDateFormat::getInstance()
{
    static GlobalDateFormat *s_globalDateFormat;
    if (s_globalDateFormat == nullptr) {
        s_globalDateFormat = new GlobalDateFormat;
        s_globalDateFormat->setObjectName("kylin-ipmsg_globaldateformat_globalDateFormat");
    }

    return s_globalDateFormat;
}

struct tm GlobalDateFormat::QDateTimeToCtm(const QDateTime &datetime)
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

QString GlobalDateFormat::TranslateDateFormat(const QDateTime &datetime)
{
    QDateTime currentTime;
    kdk_timeinfo *kdk_time;
    QString result;
    QString time;

    currentTime = QDateTime::currentDateTime();
    struct tm c_time = QDateTimeToCtm(datetime);

    if (datetime.daysTo(currentTime) >= 1) {
        result = kdk_system_shortformat_transform(&c_time);
        result += " ";
    } else {
        result = "";
    }
    kdk_time = kdk_system_timeformat_transform(&c_time);
    if (kdk_time) {
        result += kdk_time->time;
        // qDebug() << "time = " <<  kdk_time->time;
        kdk_free_timeinfo(kdk_time);
    }

    return result;
}


/**
 * @brief "日期短格式 时间"的字符串通过转为QDateTime再转为另一种格式"日期短格式 时间"字符串
 * @param dateTime "日期短格式 时间"字符串，如 2023-6-26 10:30:30
 * @param strToDateTimeFormat 一种转化格式(用于将字符串转为QDateTime), 如 yyyy-MM-dd hh:mm:ss
 * @param dateTimeToStrFormat 另一种转化格式(用于将QDateTime转为字符串) 如 yyyy-MM-dd ap hh:mm:ss
 * @return 需要特定格式的"日期短格式 时间"字符串，如2023-6-26 上午 10:30:30
 */
QString GlobalDateFormat::TranslateDateAndTimeFormat(QString dateTime, QString strToDateTimeFormat, QString dateTimeToStrFormat)
{
    QString newDateTimeStr;
    QDateTime qDateTime;

    qDateTime = QDateTime::fromString(dateTime, strToDateTimeFormat);
    qDebug() << "( GlobalDateFormat::TranslateDateAndTimeFormat ) 字符串dateTime = " << dateTime << "使用格式：" << strToDateTimeFormat << "转化为QDateTime：" << qDateTime;

    if (!qDateTime.isValid()) {
        qWarning() << "( GlobalDateFormat::TranslateDateAndTimeFormat )  translate fail, using format :" << strToDateTimeFormat;
        return dateTime;
    }

    // 处理显示数据
    newDateTimeStr = qDateTime.toString(dateTimeToStrFormat);
    qDebug() << "( GlobalDateFormat::TranslateDateAndTimeFormat ) qDateTime = " << qDateTime << "使用格式：" << dateTimeToStrFormat << "转化为newDateTimeStr：" << newDateTimeStr;
    return newDateTimeStr;
}

void GlobalDateFormat::init()
{
    // 初始化变量
    m_shortDateFormat = QString(kdk_system_get_shortformat());
    m_timeSystem = QString();
    if (m_shortDateFormat.isEmpty()) {
        qWarning() << "( GlobalDateFormat::init ) get short date format fail from kdk_system_get_shortformat()";
        m_shortDateFormat = "yyyy-MM-dd";
    }

    // 连接ukui-control-center的DBus服务
    m_dateFormatSessionDbus = new QDBusInterface("com.kylin.kysdk.DateServer",
                                           "/com/kylin/kysdk/Date",
                                           "com.kylin.kysdk.DateInterface",
                                           QDBusConnection::sessionBus(),
                                           this);
    m_dateFormatSessionDbus->setObjectName("kylin-ipmsg_QDBusInterface_dateFormatSessionDbus");
    if (m_dateFormatSessionDbus->isValid()) {
        qInfo() << "( GlobalDateFormat::init ) dbus connect success";
        connect(m_dateFormatSessionDbus, SIGNAL(DateSignal(QString)), this, SLOT(slotDateFormatChange(QString)));
        connect(m_dateFormatSessionDbus, SIGNAL(ShortDateSignal(QString)), this, SLOT(slotShortDateFormatChange(QString)));
        connect(m_dateFormatSessionDbus, SIGNAL(LongDateSignal(QString)), this, SLOT(slotLongDataFormatChange(QString)));
        connect(m_dateFormatSessionDbus, SIGNAL(TimeSignal(QString)), this, SLOT(slotTimeFormatChange(QString)));
    } else {
        qWarning() << "( GlobalDateFormat::init ) dbus connect fail";
    }
}

void GlobalDateFormat::slotDateFormatChange(QString str)
{

}

void GlobalDateFormat::slotShortDateFormatChange(QString str)
{

}

void GlobalDateFormat::slotLongDataFormatChange(QString str)
{

}

void GlobalDateFormat::slotTimeFormatChange(QString str)
{
    m_shortDateFormat =  QString(kdk_system_get_shortformat());
    m_timeSystem = QString(kdk_system_get_now_timeformat());
    emit sigTimeFormatChange(str);
}


