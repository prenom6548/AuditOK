/*
 * Ukui-shutdown-timer
 *
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
 *
 * Authors:  zhangtianze <zhangtianze@kylinos.cn>
 *
 */

#include "timeshutdowndaemon.h"

TimeShutdownDaemon::TimeShutdownDaemon(QObject *parent) : QObject(parent)
{
    //判断关机状态
    if (getGsettingState()) {

        //获取gsetting中的星期设置
        getSelectedWeek();

        //获取gsetting中关机时间设置
        getShutdownTime();

        m_timer = new QTimer();
        m_timer->start(1000);
        connect(m_timer, &QTimer::timeout, this, &TimeShutdownDaemon::threadSlots);
        connect(m_gsettings, &QGSettings::changed, this, [ = ](const QString &key) {
            if (key == "timeshutdown") {
                exit(0);
            }
        });
    } else {
        exit(0);
    }
}

bool TimeShutdownDaemon::getGsettingState()
{
    if (QGSettings::isSchemaInstalled(UKUITIMESHUTDOWN)) {
        m_gsettings = new QGSettings(UKUITIMESHUTDOWN);
    }

    if (m_gsettings) {
        QStringList keyList = m_gsettings->keys();
        if (keyList.contains("timeshutdown") && m_gsettings->get("timeshutdown").toBool()) {
            return true;
        }
    }
    return false;
}

void TimeShutdownDaemon::getSelectedWeek()
{
    QStringList keyList = m_gsettings->keys();
    if (keyList.contains("shutdownfrequency")) {
        QStringList frequencyList = m_gsettings->get("shutdownfrequency").toString().split(" ");
        m_selectWeek.clear();
        for (int i = 0; i < frequencyList.count(); ++i) {
            m_selectWeek.append(QString(frequencyList.at(i)).toInt());
        }
    }
}

void TimeShutdownDaemon::getShutdownTime()
{
    QStringList keyList = m_gsettings->keys();
    if (keyList.contains("shutdowntime")) {
        QStringList timeList = m_gsettings->get("shutdowntime").toString().split(":");
        m_hour = QString(timeList.at(0)).toInt();
        m_minite = QString(timeList.at(1)).toInt();
    }
}

bool TimeShutdownDaemon::isTodayShutdown()
{
    QDate date = QDateTime::currentDateTime().date();
    if (m_selectWeek.contains(EVERYDAY) || m_selectWeek.contains(ONLY_ONCE)) {
        return true;
    }

    if (m_selectWeek.contains(date.dayOfWeek() + 2)) {
        return true;
    }
    return false;
}


void TimeShutdownDaemon::threadSlots()
{
    if (isTodayShutdown()) {
        QTime currentTime = QTime::currentTime();
        int currentSecond = currentTime.second();
        int currentMinute = currentTime.minute();
        int currentHour = currentTime.hour();

        bool isNeedShutdown = m_gsettings->get("timeshutdown").toBool();

        if (isNeedShutdown && m_hour == currentHour && m_minite == currentMinute && currentSecond <= 2) {
            if (m_selectWeek.contains(ONLY_ONCE)) {
                m_gsettings->set("timeshutdown", false);
            }
            QProcess p(0);
            p.startDetached("ukui-session-tools --shutdown");
            p.waitForStarted();
        }
    }
}
