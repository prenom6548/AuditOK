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
#include "chartviewscrollarea.h"
#include <QDebug>
#include <QGSettings>

#include <kysdk/kysdk-system/libkync.h>
#include <kysdk/kysdk-system/libkyrealtimeinfo.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#include "../src/macro.h"
#include "commoninfo.h"

const int itemSpace = 6;

typedef struct _net1
{
    guint64 last_in, last_out;
    gint64 time;
} net1;
QList<net1> netinfo1;

ChartViewScrollArea::ChartViewScrollArea(ChartItemWidget::ChartViewTypes types, QWidget *parent)
    : m_chartType(types),
      QScrollArea(parent)
{
    m_pCentralWidget = new QWidget(this);
    this->setWidget(m_pCentralWidget);
    this->setFrameShape(QFrame::NoFrame);
    m_mapItemView.clear();
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        QGSettings *m_qtSettings = new QGSettings(idd);
        if (m_qtSettings != nullptr) {
            //监听主题改变
            connect(m_qtSettings, &QGSettings::changed, this, [=](const QString &key) {
                if ("systemFont" == key || "systemFontSize" == key) {
                    fontChanged(QApplication::font());
                }
            });
        }
    }
    initUI();

}

void ChartViewScrollArea::initUI()
{
    switch (m_chartType) {
    case ChartItemWidget::ChartViewTypes::CPU_CHART:
    {
        this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        chartList = cpuInfo::getAllCpuName();
        m_currentChartName = "0";
        setContextMenuPolicy(Qt::CustomContextMenu);
        m_pMenu = new QMenu(this);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pMenu, "ChartViewScrollArea", "", "");
        QAction *showAllUsage = m_pMenu->addAction(tr("Display overall utilization rate"));
        QAction *showSingleUsage = m_pMenu->addAction(tr("Display Logic Processor"));
        showAllUsage->setCheckable(true);
        showSingleUsage->setCheckable(true);
        showAllUsage->setChecked(true);
        connect(showAllUsage, &QAction::triggered, this, [=](){
            showAllCpuUsage = true;
            showAllUsage->setChecked(true);
            showSingleUsage->setChecked(false);
            updateWidgetGeometry();
        });
        connect(showSingleUsage, &QAction::triggered, this, [=](){
            showAllCpuUsage = false;
            showAllUsage->setChecked(false);
            showSingleUsage->setChecked(true);
            updateWidgetGeometry();
        });
        connect(this, &QScrollArea::customContextMenuRequested, this, [=](const QPoint &pos){
            // 显示菜单在点击位置
            QPoint globalPos = this->viewport()->mapToGlobal(pos);
            m_pMenu->popup(globalPos);
        });
        for (int i = 0; i < chartList.size(); i++) {
            if (!m_mapItemView.keys().contains(chartList.at(i))) {
                ChartItemWidget *item = new ChartItemWidget(ChartItemWidget::ChartViewTypes::CPU_CHART, m_pCentralWidget);
                insertionOrder.append(chartList.at(i));
                m_mapItemView.insert(chartList.at(i), item);
                connect(item, &ChartItemWidget::clicked, this, &ChartViewScrollArea::onSetItemActiveStatus);
            }
        }
        updateWidgetGeometry();
        break;
    }
    case ChartItemWidget::ChartViewTypes::MEM_CHART:
        break;
    case ChartItemWidget::ChartViewTypes::NET_CHART:
        chartList = netInfo::getAllCards();
        for (int i = 0; i< chartList.size(); i++) {
            net1 net;
            net.last_in = 0;
            net.last_out = 0;
            net.time = 0;
            netinfo1.insert(i, net);
        }
        onUpdateNetChartData();
        break;
    case ChartItemWidget::ChartViewTypes::BLOCK_CHART:
        chartList = diskInfo::getDiskName();
        qDebug() << "device name list = " << chartList;
        m_currentChartName = chartList.at(0);
        onUpdateDiskChartData();
        break;
    default:
        break;
    }
}


void ChartViewScrollArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateWidgetGeometry();
}

void ChartViewScrollArea::fontChanged(const QFont &font)
{
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        ChartItemWidget *itemView = iter.value();
        itemView->fontChanged(font);
    }
}

QString ChartViewScrollArea::getCurrentChartName()
{
    return m_currentChartName;
}

void ChartViewScrollArea::onUpdateCpuChartData(QVector<double> usage)
{
    for (int i = 0; i < usage.size(); i++) {
        if (!m_mapItemView.keys().contains(chartList.at(i))) {
            ChartItemWidget *item = new ChartItemWidget(ChartItemWidget::ChartViewTypes::CPU_CHART, m_pCentralWidget);
            insertionOrder.append(chartList.at(i));
            m_mapItemView.insert(chartList.at(i), item);
            connect(item, &ChartItemWidget::clicked, this, &ChartViewScrollArea::onSetItemActiveStatus);
        } else {
            ChartItemWidget *itemWidget = m_mapItemView.value(chartList.at(i));
            itemWidget->updateCpuData(chartList.at(i), usage[i]);
        }
    }
    updateWidgetGeometry();
}

void ChartViewScrollArea::onUpdateNetChartData()
{
    for (int i = 0; i < chartList.count(); i++) {
        const QString &mac = chartList.at(i);
        std::string stdstr = mac.toStdString();
        const char *ncName = stdstr.c_str();
//        float send = kdk_real_get_net_speed(ncName);
//        float recv = kdk_real_get_if_speed(ncName);

        guint64 in = 0, out = 0;
        gint64 time;
        guint64 din = 0, dout = 0;
        in  += kdk_nc_get_rx_bytes(ncName);
        out += kdk_nc_get_tx_bytes(ncName);

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
            ChartItemWidget *itemWidget = new ChartItemWidget(ChartItemWidget::ChartViewTypes::NET_CHART, m_pCentralWidget, netifInfo.netName);
            connect(itemWidget, &ChartItemWidget::clicked, this, &ChartViewScrollArea::onSetItemActiveStatus);
            itemWidget->updateData(netifInfo);
            m_mapItemView.insert(mac, itemWidget);
            insertionOrder.append(mac);
        } else {
            ChartItemWidget *itemWidget = m_mapItemView.value(mac);
            itemWidget->updateData(netifInfo);
        }
    }

    updateWidgetGeometry();

    int netifCnt = chartList.size();
    if (!m_initStatus && netifCnt > 0) {
        m_initStatus = true;
        if (netifCnt > 1) {
            onSetItemActiveStatus(m_mapItemView.begin().key());
        } else {
            emit chartItemClicked(m_mapItemView.begin().key());
        }
        m_currentChartName = m_mapItemView.begin().key();
    }

    if (netifCnt > 0 && !chartList.contains(m_currentChartName)) {
        if (chartList.size()  == 1) {
            m_mapItemView.value(m_mapItemView.begin().key())->updateActiveStatus(false);
        } else {
            m_mapItemView.value(m_mapItemView.begin().key())->updateActiveStatus(true);
        }
        m_currentChartName = m_mapItemView.begin().key();
        emit chartItemClicked(m_mapItemView.begin().key());
        return ;
    }
}

void ChartViewScrollArea::onUpdateDiskChartData()
{
    chartList = diskInfo::getDiskName();
    qDebug() << "device name list = " << chartList << m_currentChartName;

    // 如果新获取的磁盘列表不包含当前选中的磁盘，设置当前磁盘为第一个
    if (!chartList.contains(m_currentChartName)) {
        onSetItemActiveStatus(chartList.at(0));
    }

    foreach (QString name, preChartList) {
        if (!chartList.contains(name)) {
            if (m_mapItemView.keys().contains(name)) {
                ChartItemWidget *itemWidget = m_mapItemView.value(name);
                m_mapItemView.remove(name, itemWidget);
                itemWidget->deleteLater();
            }
        }
    }

    int deviceCount = chartList.size();
    for (int i = 0 ; i < deviceCount; i++) {
        if (!m_mapItemView.keys().contains(chartList.at(i))) {
            ChartItemWidget *item = new ChartItemWidget(ChartItemWidget::ChartViewTypes::BLOCK_CHART, m_pCentralWidget);
            insertionOrder.append(chartList.at(i));
            m_mapItemView.insert(chartList.at(i), item);
            connect(item, &ChartItemWidget::clicked, this, &ChartViewScrollArea::onSetItemActiveStatus);
        } else {
            ChartItemWidget *itemWidget = m_mapItemView.value(chartList.at(i));
            itemWidget->updateData(chartList.at(i));
        }
    }
    updateWidgetGeometry();

    preChartList = chartList;
}

void ChartViewScrollArea::onSetItemActiveStatus(const QString &name)
{
    int chartCount  = chartList.size();
    m_currentChartName = name.toUtf8();

    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        ChartItemWidget *itemView = iter.value();
        if (chartCount > 1 && iter.key() == name) {
            itemView->updateActiveStatus(true);
        } else {
            itemView->updateActiveStatus(false);
        }
        if (iter.key() == name) {
            emit chartItemClicked(name);
        }
    }
}

void ChartViewScrollArea::updateWidgetGeometry()
{
    int chartCount  = chartList.size();

    if (chartCount <= 0) {
        return ;
    }

    if (chartCount == 1 || (m_chartType == ChartItemWidget::ChartViewTypes::CPU_CHART && showAllCpuUsage)) {
        showItemOne();
    } else if (chartCount == 2) {
        showItemTwo();
    } else if (chartCount > 2) {
        showItemLgTwo(chartCount);
    }
}

void ChartViewScrollArea::showItemOne()
{
    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        ChartItemWidget *itemView = iter.value();
        if (chartList.contains(iter.key())) {
            // 处理器显示模式为显示总体利用率，隐藏cpu0-n的曲线图
            if (m_chartType == ChartItemWidget::ChartViewTypes::CPU_CHART && iter.key() != "cpu" && showAllCpuUsage) {
                itemView->hide();
                itemView->updateActiveStatus(false);
                continue;
            }
            itemView->show();
            itemView->setMode(ChartItemWidget::TITLE_VERTICAL);
            itemView->setGeometry(0, 0, this->width(), this->height());
            itemView->updateActiveStatus(false);
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }

    m_pCentralWidget->setFixedSize(this->width(), this->height());
}

void ChartViewScrollArea::showItemTwo()
{
    int itemOffsetX = 0;
    int itemHeight  = this->height();
    int itemWidth   = (this->width() - itemSpace) / 2;

    for (auto iter = m_mapItemView.begin(); iter != m_mapItemView.end(); iter++) {
        ChartItemWidget *itemView = iter.value();
        if (chartList.contains(iter.key())) {
            itemView->show();
            itemView->setMode(ChartItemWidget::TITLE_VERTICAL);
            itemView->setGeometry(itemOffsetX, 0, itemWidth, itemHeight);
            itemOffsetX = itemOffsetX + itemWidth + itemSpace;
            if (iter.key() == m_currentChartName) {
                itemView->updateActiveStatus(true);
            }
        } else {
            itemView->hide();
            itemView->updateActiveStatus(false);
        }
    }

    m_pCentralWidget->setFixedSize(this->width(), this->height());
}

void ChartViewScrollArea::showItemLgTwo(int count)
{
    int itemHeight  = this->height() / 2;
    int itemWidth   = this->width() / 2 - itemSpace*2;
    int itemCurrentCnt = -1;
    int page = 0;

    // 顺序遍历
    for (const QString &key : insertionOrder) {
        // 判断创建的曲线图列表中是否存在该key
        if (m_mapItemView.contains(key)) {
            ChartItemWidget *itemView = m_mapItemView.value(key);
            // 判断实际获取的设备、网卡、cpu是否包含该key，不包含隐藏显示
            if (chartList.contains(key)) {
                // 处理器显示模式为显示逻辑利用率，隐藏cpu的曲线图
                if (key == "cpu" && !showAllCpuUsage) {
                    itemView->hide();
                    itemView->updateActiveStatus(false);
                    continue;
                }
                itemCurrentCnt++;
                itemView->setMode(ChartItemWidget::TITLE_VERTICAL);
                itemView->show();
                page = itemCurrentCnt / 4;
                if (itemCurrentCnt % 4 == 0) {
                    itemView->setGeometry(0, page * this->height(), itemWidth, itemHeight);
                } else if (itemCurrentCnt % 4 == 1) {
                    itemView->setGeometry(itemWidth + itemSpace, page * this->height(), itemWidth, itemHeight);
                } else if (itemCurrentCnt % 4 == 2) {
                    itemView->setGeometry(0, page * this->height() + itemHeight, itemWidth, itemHeight);
                } else if (itemCurrentCnt % 4 == 3) {
                    itemView->setGeometry(itemWidth + itemSpace, page * this->height() + itemHeight, itemWidth, itemHeight);
                }
                if (key == m_currentChartName) {
                    itemView->updateActiveStatus(true);
                }
            } else {
                itemView->hide();
                itemView->updateActiveStatus(false);
            }
        }
    }
    if (itemCurrentCnt >= 4) {
        m_pCentralWidget->setFixedSize(this->width() - 16, this->height() + this->height() * page);
    } else {
        m_pCentralWidget->setFixedSize(this->width(), this->height() + this->height() * page);
    }
}
