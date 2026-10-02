/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "netviewscrollarea.h"
#include <QDebug>

#include "commoninfo.h"

#include "kysdk/kysdk-system/libkync.h"
#include <kysdk/kysdk-system/libkyrealtimeinfo.h>

const int itemSpace = 6;

#include <glibtop/netload.h>
typedef struct _net1
{
    guint64 last_in, last_out;
    gint64 time;
} net1;
QList<net1> netinfo1;

NetViewScrollArea::NetViewScrollArea(QWidget *parent) : QScrollArea(parent)
{
    m_centralWidget = new QWidget(this);
    this->setWidget(m_centralWidget);
    this->setFrameShape(QFrame::NoFrame);
    cardlist = netInfo::getAllCards();
    for (int i = 0; i< cardlist.size(); i++) {
        net1 net;
        net.last_in = 0;
        net.last_out = 0;
        net.time = 0;
        netinfo1.insert(i, net);
    }
    onModelUpdate();
}

void NetViewScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateWidgetGeometry();
}

void NetViewScrollArea::fontChanged(const QFont &font)
{
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetItemWidget *itemView = iter.value();
        itemView->fontChanged(font);
    }
}

QString NetViewScrollArea::getCurrentNCName()
{
    return m_currentMac;
}

void NetViewScrollArea::onModelUpdate()
{
    for (int i = 0; i < cardlist.count(); i++) {
        const QString &mac = cardlist.at(i);
        std::string stdstr = mac.toStdString();
        const char *ncName = stdstr.c_str();
//        float send = kdk_real_get_net_speed(ncName);
//        float recv = kdk_real_get_if_speed(ncName);

        guint64 in = 0, out = 0;
        gint64 time;
        guint64 din = 0, dout = 0;
        glibtop_netload netload;
        glibtop_get_netload(&netload, ncName);

        in  = netload.bytes_in;
        out = netload.bytes_out;

        time = g_get_real_time();

        if (in >= netinfo1[i].last_in && out >= netinfo1[i].last_out && netinfo1[i].time != 0) {
            float dtime;
            dtime = (double)(time - netinfo1[i].time) / G_USEC_PER_SEC;
            din   = static_cast<guint64>((in  - netinfo1[i].last_in)  / dtime);
            dout  = static_cast<guint64>((out - netinfo1[i].last_out) / dtime);
        } else {
            /* Don't calc anything if new data is less than old (interface
               removed, counters reset, ...) or if it is the first time */
            din  = 0;
            dout = 0;
        }
        netinfo1[i].last_in  = in;
        netinfo1[i].last_out = out;
        netinfo1[i].time     = time;

        NetifInfo netifInfo;
        netifInfo.receiveBytes = din;
        netifInfo.sendBytes = dout;
        netifInfo.netName = mac;
        if (!m_mapItemView.contains(mac)) {
            NetItemWidget *itemWidget = new NetItemWidget(m_centralWidget, netifInfo.netName);
            connect(itemWidget, &NetItemWidget::clicked, this, &NetViewScrollArea::onSetItemActiveStatus);
            itemWidget->updateData(netifInfo);
            m_mapItemView.insert(mac, itemWidget);
        } else {
            NetItemWidget *itemWidget = m_mapItemView.value(mac);
            itemWidget->updateData(netifInfo);
        }
    }

    updateWidgetGeometry();

    int netifCnt = cardlist.size();
    if (!m_initStatus && netifCnt > 0) {
        m_initStatus = true;
        if (netifCnt > 1) {
            onSetItemActiveStatus(m_mapItemView.begin().key());
        } else {
            emit netifItemClicked(m_mapItemView.begin().key());
        }
        m_currentMac = m_mapItemView.begin().key();
    }

    if (netifCnt > 0 && !cardlist.contains(m_currentMac)) {
        if (cardlist.size()  == 1) {
            m_mapItemView.value(m_mapItemView.begin().key())->updateActiveStatus(false);
        } else {
            m_mapItemView.value(m_mapItemView.begin().key())->updateActiveStatus(true);
        }
        m_currentMac = m_mapItemView.begin().key();
        emit netifItemClicked(m_mapItemView.begin().key());
        return ;
    }
}

void NetViewScrollArea::onSetItemActiveStatus(const QString &mac)
{
    int netCount  = cardlist.size();
    m_currentMac = mac.toUtf8();

    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetItemWidget *itemView = iter.value();
        if (netCount > 1 && iter.key() == mac) {
            itemView->updateActiveStatus(true);
            emit netifItemClicked(mac);
        } else {
            itemView->updateActiveStatus(false);
        }
    }
}

void NetViewScrollArea::updateWidgetGeometry()
{
    int netCount  = cardlist.size();

    if (netCount == 1)
        showItemOnlyeOne();
    else if (netCount == 2)
        showItemDouble();
    else if (netCount > 2)
        showItemLgDouble();
}

void NetViewScrollArea::showItemOnlyeOne()
{
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetItemWidget *itemView = iter.value();
        if (cardlist.contains(iter.key())) {
            itemView->show();
            itemView->setMode(NetItemWidget::TITLE_HORIZONTAL);
            itemView->setGeometry(0, 0, this->width(), this->height());
            itemView->updateActiveStatus(false);
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }

    m_centralWidget->setFixedSize(this->width(), this->height());
}

void NetViewScrollArea::showItemDouble()
{
    int itemOffsetX = 0;
    int itemHeight  = this->height();
    int itemWidth   = (this->width() - itemSpace) / 2;

    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetItemWidget *itemView = iter.value();
        if (cardlist.contains(iter.key())) {
            itemView->show();
            itemView->setMode(NetItemWidget::TITLE_VERTICAL);
            itemView->setGeometry(itemOffsetX, 0, itemWidth, itemHeight);
            itemOffsetX = itemOffsetX + itemWidth + itemSpace;
            if (iter.key() == m_currentMac) {
                itemView->updateActiveStatus(true);
            }
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }

    m_centralWidget->setFixedSize(this->width(), this->height());
}

void NetViewScrollArea::showItemLgDouble()
{
    int itemHeight  = this->height() / 2;
    int itemWidth   = this->width() / 2 - itemSpace*2;
    int itemCurrentCnt = -1;
    int page = 0;
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        NetItemWidget *itemView = iter.value();
        if (cardlist.contains(iter.key())) {
            itemCurrentCnt++;
            itemView->setMode(NetItemWidget::TITLE_VERTICAL);
            itemView->show();
            page = itemCurrentCnt / 4;
            if (itemCurrentCnt % 4 == 0) {
                itemView->setGeometry(0, page * this->height(), itemWidth, itemHeight);
            } else if (itemCurrentCnt % 4 == 1) {
                itemView->setGeometry(0, page * this->height() + itemHeight, itemWidth, itemHeight);
            } else if (itemCurrentCnt % 4 == 2) {
                itemView->setGeometry(itemWidth + itemSpace, page * this->height(), itemWidth, itemHeight);
            } else if (itemCurrentCnt % 4 == 3) {
                itemView->setGeometry(itemWidth + itemSpace, page * this->height() + itemHeight, itemWidth, itemHeight);
            }
            if (iter.key() == m_currentMac) {
                itemView->updateActiveStatus(true);
            }
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }
    m_centralWidget->setFixedSize(this->width(), this->height() + this->height() * page);
}

