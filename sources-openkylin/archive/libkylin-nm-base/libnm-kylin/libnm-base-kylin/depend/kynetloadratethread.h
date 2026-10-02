/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef KYNETLOADRATETHREAD_H
#define KYNETLOADRATETHREAD_H

#include <QThread>
#include <QString>
#include <QTimer>

class KyNetLoadRateThread : public QThread
{
public:
    KyNetLoadRateThread(QString dev, QObject *parent = nullptr);
    ~KyNetLoadRateThread();

    void getNetSpeed(long int &rcvRate, long int &txRate) {
        rcvRate = m_deltaRcv;
        txRate = m_deltaTx;
    }

protected:
    virtual void run();

private:
    void updateLoadRate();
    int  getCurrentLoadRate(QString dev, long *saveRate, long *txRate);

    QString m_deviceName;
    QTimer * m_updateRatesTimer = nullptr;
    long int m_startRcvRate = 0;  //保存开始时的流量计数
    long int m_endRcvRate = 0;  //保存结束时的流量计数
    long int m_startTxRate = 0;  //保存开始时的流量计数
    long int m_endTxRate = 0;  //保存结束时的流量计数
    long int m_deltaRcv = 0;
    long int m_deltaTx = 0;

Q_SIGNALS:
    void loadRateUpdate();
};

#endif // KYNETLOADRATETHREAD_H
