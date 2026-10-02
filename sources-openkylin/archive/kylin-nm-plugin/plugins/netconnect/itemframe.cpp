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

#include "itemframe.h"

#define LAYOUT_MARGINS 4,0,12,0
#define MAIN_LAYOUT_MARGINS 0,0,0,0
ItemFrame::ItemFrame(QString devName, QWidget *parent)
    : m_deviceName(devName), QFrame(parent)
{
    deviceLanLayout = new QVBoxLayout(this);
    deviceLanLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    lanItemFrame = new QFrame(this);
    lanItemFrame->setFrameShape(QFrame::Shape::NoFrame);

    lanItemLayout = new QVBoxLayout(this);
    lanItemLayout->setContentsMargins(LAYOUT_MARGINS);
    lanItemLayout->setSpacing(0);

    deviceLanLayout->setSpacing(0);
    setLayout(deviceLanLayout);
    lanItemFrame->setLayout(lanItemLayout);

    deviceFrame = new DeviceFrame(devName, this);

    m_divider = new Divider(this);
    deviceLanLayout->addWidget(m_divider);
    deviceLanLayout->addWidget(deviceFrame);
    deviceLanLayout->addWidget(lanItemFrame);

    m_updateNetSpeedThread = new KyNetLoadRateThread(devName);
}

ItemFrame::~ItemFrame()
{
    if (m_updateNetSpeedThread) {
        m_updateNetSpeedThread->quit();
        m_updateNetSpeedThread->wait();
        delete m_updateNetSpeedThread;
        m_updateNetSpeedThread = nullptr;
    }
}

void ItemFrame::startNetSpeedThread()
{
    m_updateNetSpeedThread->start();
}

void ItemFrame::stopNetSpeedThread()
{
    if (m_updateNetSpeedThread->isRunning()) {
        m_updateNetSpeedThread->quit();
    }
}

void ItemFrame::updateNetSpeed()
{
    if (!m_updateNetSpeedThread->isRunning() || lanItemLayout->count() <= 0) {
        return;
    }

    long int rcvRate;
    long int txRate;
    LanItem *lanItem = nullptr;
    lanItem = (LanItem *)lanItemLayout->itemAt(0)->widget();
    if (lanItem->getStatus()) {
        m_updateNetSpeedThread->getNetSpeed(rcvRate, txRate);
        setNetSpeed(rcvRate, txRate, lanItem);
    }
}

void ItemFrame::setNetSpeed(long rcvRate, long txRate, LanItem *lanItem)
{
    if (!lanItem->getStatus()) {
        return;
    }

    int rcvNum = rcvRate;
    int txNum = txRate;

    QString strRcv = "0";
    QString strTx = "0";
    if (rcvNum < 1024) {
        strRcv = QString::number(rcvNum) + "KB/s";
    } else {
        int remainder;
        if (rcvNum % 1024 < 100) {
            remainder = 0;
        } else {
            remainder = (rcvNum % 1024) / 100;
        }
        strRcv = QString::number(rcvNum / 1024) + "."  + QString::number(remainder) + "MB/s";
    }

    if (txNum < 1024) {
        strTx = QString::number(txNum) + "KB/s";
    } else {
        int remainder;
        if (txNum % 1024 < 100) {
            remainder = 0;
        } else {
            remainder = (txNum % 1024)/100;
        }
        strTx = QString::number(txNum / 1024) + "."  + QString::number(remainder) + "MB/s";
    }
    lanItem->setNetSpeed(strRcv, strTx);
}
