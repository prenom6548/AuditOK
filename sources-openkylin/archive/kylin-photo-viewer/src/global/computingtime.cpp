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
#include "computingtime.h"

ComputingTime *ComputingTime::m_computingTime(nullptr);

ComputingTime *ComputingTime::getInstance()
{
    if (m_computingTime == nullptr) {
        m_computingTime = new ComputingTime();
    }
    return m_computingTime;
}

void ComputingTime::start()
{
    m_timer->start(10);
}

QString ComputingTime::getTime(const QString &tag)
{
    QString str = QString::number(m_time, 'g', 2);
    QString out = tag + ":" + str + "\n";
    //    printf(out.toLocal8Bit().data());
    return str;
}

ComputingTime::ComputingTime()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ComputingTime::computingTime);
}

void ComputingTime::computingTime()
{
    m_time += 0.01;
}
