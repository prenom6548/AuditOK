/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef DBUSNOTICESHOWAGAINADAPTOR_H
#define DBUSNOTICESHOWAGAINADAPTOR_H

#include <QObject>
#include "dbusbaseclockadaptor.h"
#include "noticealarmpoolutil.h"
#include "ClockInterface.h"
class DbusNoticeShowAgainAdaptor : public DbusBaseClockAdaptor
{
    Q_OBJECT
    //声明它正在导出哪个接口
    Q_CLASSINFO("D-Bus Interface", CLOCK_DBUS_SERVICE_NAME)
public:
    explicit DbusNoticeShowAgainAdaptor(QObject *parent = nullptr,NoticeAlarmPoolUtil * currentNotice = nullptr);

signals:
public slots:
    QString noticeShowAgain(QString param);
private:
    NoticeAlarmPoolUtil * m_notice;

};

#endif // DBUSNOTICESHOWAGAINADAPTOR_H
