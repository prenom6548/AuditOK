/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UPMCPUOCCUPY_H
#define UPMCPUOCCUPY_H

#include <QObject>
#include <QTimer>

#define CPU_PROC_PATH        "/proc/"
#define CPU_PROC_STATE        "stat"

#define MIN_OFFLINE_CPU_OCCUPY       25
#define MAX_ONLINE_CPU_OCCUPY        85
#define MIN_CPU_OCCUPY_INTERVAL      10

class UpmCpuOccupy : public QObject
{
    Q_OBJECT
public:
    explicit UpmCpuOccupy(QObject *parent = nullptr);

    void startCalcCpuOccupy(int timeout, int offlineCpuOccupy, int onlineCpuOccupy);
    void stopCalcCpuOccupy();
private:
    QTimer *m_calcCpuOccupyTimer;

    quint64 m_currentTotalTime = 0;
    quint64 m_currentUserTime = 0;
    quint64 m_lastTotalTime = 0;
    quint64 m_lastUserTime = 0;

    int m_offlineCpuOccupy = 0;
    int m_onlineCpuOccupy = 0;
    void getCpuOccupyTime(quint64 &totalTime, quint64 &userTime);

private slots:
    void calcCpuOccupy();

signals:
    void cpuCoreSwitchSignal(bool);
};

#endif // UPMCPUOCCUPY_H
