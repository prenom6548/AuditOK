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

#ifndef TIMESHUTDOWNDAEMON_H
#define TIMESHUTDOWNDAEMON_H

#include <QObject>
#include <QDebug>
#include <QVariant>
#include <QGSettings>
#include <QTimer>
#include <QDateTime>
#include <QProcess>

#define UKUITIMESHUTDOWN "org.ukui.time-shutdown.settings"

class TimeShutdownDaemon : public QObject
{
    Q_OBJECT
public:
    explicit TimeShutdownDaemon(QObject *parent = nullptr);

private:
    enum SelectSection{
        ONLY_ONCE = 1,
        EVERYDAY = 2
    };

    //获取是否有定时关机设置。
    bool getGsettingState();

    //获取定时关机选择的关机日期。
    void getSelectedWeek();

    //获取定时关机选择的关机时间。
    void getShutdownTime();

    bool isTodayShutdown();

    //开始轮询
    void threadSlots();

    QGSettings  *m_gsettings = nullptr;
    QTimer *m_timer = nullptr;
    QList<int>  m_selectWeek;
    int m_hour;
    int m_minite;

signals:

};

#endif // TIMESHUTDOWNDAEMON_H
