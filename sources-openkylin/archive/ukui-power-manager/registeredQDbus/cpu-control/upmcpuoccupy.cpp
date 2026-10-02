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
#include "upmcpuoccupy.h"
#include "../upmcommonfile.h"

#include <QDebug>

UpmCpuOccupy::UpmCpuOccupy(QObject *parent) : QObject(parent)
{
    m_calcCpuOccupyTimer = new QTimer(this);
    connect(m_calcCpuOccupyTimer, &QTimer::timeout, this, &UpmCpuOccupy::calcCpuOccupy);
}

void UpmCpuOccupy::startCalcCpuOccupy(int timeout, int offlineCpuOccupy, int onlineCpuOccupy)
{
    if (timeout < 10000
    || offlineCpuOccupy < MIN_OFFLINE_CPU_OCCUPY
    || onlineCpuOccupy > MAX_ONLINE_CPU_OCCUPY) {
        return ;
    }

    if (offlineCpuOccupy > onlineCpuOccupy
    || onlineCpuOccupy - offlineCpuOccupy < MIN_CPU_OCCUPY_INTERVAL) {
        return ;
    }

    qDebug() << "start calc cpu occupy:" << timeout << offlineCpuOccupy << onlineCpuOccupy;
    m_offlineCpuOccupy = offlineCpuOccupy;
    m_onlineCpuOccupy = onlineCpuOccupy;
    getCpuOccupyTime(m_lastTotalTime, m_lastUserTime);
    qDebug() << "init cpu time:" << m_lastTotalTime << m_lastUserTime;
    m_calcCpuOccupyTimer->start(timeout);
}

void UpmCpuOccupy::stopCalcCpuOccupy()
{
    if (m_calcCpuOccupyTimer->isActive()) {
        m_calcCpuOccupyTimer->stop();
    }
}

void UpmCpuOccupy::calcCpuOccupy()
{
    getCpuOccupyTime(m_currentTotalTime, m_currentUserTime);
    qDebug() << "current cpu time:" << m_currentTotalTime << m_currentUserTime;

    int cpuOccupy = (m_currentUserTime - m_lastUserTime) * 100/(m_currentTotalTime - m_lastTotalTime);
    qDebug() << "cpu occupy:" << cpuOccupy;

    m_lastTotalTime = m_currentTotalTime;
    m_lastUserTime = m_currentUserTime;

    if (cpuOccupy < m_offlineCpuOccupy) {
        emit cpuCoreSwitchSignal(false);
    } else if (cpuOccupy > m_onlineCpuOccupy){
        emit cpuCoreSwitchSignal(true);
    } else {
        ;
    }
}

void UpmCpuOccupy::getCpuOccupyTime(quint64 &totalTime, quint64 &userTime)
{
    QString result;
    quint64 total = 0;
    QStringList cpuState;

    if (true == UpmCommonFile::read(CPU_PROC_PATH, CPU_PROC_STATE, result)) {
        if (result.contains("cpu")) {
            cpuState = result.split(" ");
//            qDebug() << "cpu time list:" << cpuState;
            for (int i = 2; i < cpuState.size(); ++i) {
                total += cpuState[i].toULong();
            }
        }
    }

    totalTime = total;
    userTime = total - cpuState[5].toULong();
}

