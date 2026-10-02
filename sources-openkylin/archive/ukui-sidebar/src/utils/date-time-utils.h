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

#ifndef DATETIMEUTILS_H
#define DATETIMEUTILS_H

#include <QObject>
#include <QGSettings>
#include <QTimer>
#include <QDateTime>

namespace Sidebar {

class DateTimeUtils : public QObject
{
    Q_OBJECT
public:
    explicit DateTimeUtils(QObject *parent = nullptr);
    ~DateTimeUtils();
    Q_INVOKABLE QString currentTime();
    Q_INVOKABLE QString currentDate();
    Q_INVOKABLE QString currentWeekDay();
    /**
     * 计算源时间与现在时刻的时间差值
     * @brief computeTimeOut
     * @param srcDate
     * @return
     */
    Q_INVOKABLE QString computeTimeOut(QDateTime timeStamp);

Q_SIGNALS:
    void timeUpdate(const QString&);
    void dateUpdate(const QString&);
    void weekDayUpdate(const QString&);
    void timeRefresh();

private Q_SLOTS:
    void timeSignalSlot();
    void setShortDateFormat(QString shortDateFormat = "");

private:
    void initGsettings();

    QString m_dataFormat;
    QString m_hourSystem;
    QString m_shortDateFormat;
    QGSettings *m_timeGsettings = nullptr;
    QTimer *m_timer = nullptr;

signals:

};
}
#endif // DATETIMEUTILS_H
