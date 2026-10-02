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

#include "kynetloadratethread.h"

#include <QDebug>
#define UPDATE_LOADRATE_TIMER 1000

KyNetLoadRateThread::KyNetLoadRateThread(QString dev, QObject *parent)
    :m_deviceName(dev), QThread(parent)
{

}

KyNetLoadRateThread::~KyNetLoadRateThread()
{

}

void KyNetLoadRateThread::run()
{
    m_updateRatesTimer = new QTimer(this);
    m_updateRatesTimer->setTimerType(Qt::PreciseTimer);
    connect(m_updateRatesTimer, &QTimer::timeout,  [&]() {
        updateLoadRate();
    });

    m_updateRatesTimer->start(UPDATE_LOADRATE_TIMER);
    exec();
}

void KyNetLoadRateThread::updateLoadRate()
{
    if (getCurrentLoadRate(m_deviceName, &m_startRcvRate, &m_startTxRate) == -1) {
        m_startRcvRate = m_endRcvRate;
        return;
    }

    m_deltaRcv = (m_startRcvRate - m_endRcvRate) / 1024;
    m_deltaTx = (m_startTxRate - m_endTxRate) / 1024;

    //简易滤波
    if (m_deltaRcv < 0 || m_deltaTx < 0) {
        m_deltaRcv = 0;
        m_deltaTx = 0;
    }
    else if (m_endRcvRate == 0 || m_endTxRate == 0){
        m_deltaRcv = 0;
        m_deltaTx = 0;
    }

    m_endRcvRate = m_startRcvRate;
    m_endTxRate = m_startTxRate;
}

int KyNetLoadRateThread::getCurrentLoadRate(QString dev, long *saveRate, long *txRate)
{
    FILE * netDevFile; //文件指针
    char buffer[1024]; //文件中的内容暂存在字符缓冲区里
    //size_t bytes_read; //实际读取的内容大小
    char * match; //用以保存所匹配字符串及之后的内容
    char * device;//将QString转为Char *
    QByteArray ba = dev.toLatin1(); // must
    device = ba.data();
    int counter = 0;
    unsigned int buffSize=0;
    char tmpValue[128];

    if ((NULL == device) || (NULL == saveRate) || (NULL == txRate)) {
        qDebug() << "LOG_FLAG" << "parameter pass error" ;
        return -1;
    }

    if ((netDevFile = fopen("/proc/net/dev", "r")) == NULL) {
        //打开文件/pro/net/dev/，从中读取流量数据
        qDebug() << "LOG_FLAG" << "error occurred when try to open file /proc/net/dev/";
        return -1;
    }
    memset(buffer, 0, sizeof(buffer));

    while (fgets(buffer, sizeof(buffer), netDevFile) != NULL) {
        match = strstr(buffer, device);

        if (NULL == match) {
            // qDebug()<<"No eth0 keyword to find!";
            continue;
        } else {
            match = match + qstrnlen(device,1024) + qstrnlen(":",1024); //地址偏移到冒号
            sscanf(match, "%ld ", saveRate);
            memset(tmpValue, 0, sizeof(tmpValue));
            sscanf(match, "%s ", tmpValue);
            match = match + qstrnlen(tmpValue,128);
            buffSize=qstrnlen(buffer,1024);
            for (size_t i = 0; i < buffSize; ++i) {
                if (0x20 == *match) {
                    match++;
                } else {
                    if (8 == counter) {
                        sscanf(match, "%ld ", txRate);
                    }
                    memset(tmpValue, 0, sizeof(tmpValue));
                    sscanf(match, "%s ", tmpValue);
                    match = match + qstrnlen(tmpValue,128);
                    counter++;
                }
            }
        }
    }

    fclose(netDevFile);
    netDevFile = nullptr;

    return 0; //返回成功
}
