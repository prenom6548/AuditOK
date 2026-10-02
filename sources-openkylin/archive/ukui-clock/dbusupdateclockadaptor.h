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
#ifndef DBUSUPDATECLOCKADAPTOR_H
#define DBUSUPDATECLOCKADAPTOR_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include "clock.h"
class DbusUpdateClockAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    //声明它正在导出哪个接口
    Q_CLASSINFO("D-Bus Interface", CLOCK_DBUS_SERVICE_NAME)
public:
    explicit DbusUpdateClockAdaptor(QObject *parent = nullptr,Clock * currentClock = nullptr);

signals:
public slots:
    QString updateClockByIdRpc(QString param);
private:
    Clock * m_clock;
};

#endif // DBUSUPDATECLOCKADAPTOR_H
