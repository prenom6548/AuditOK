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
#ifndef KLEFTWIDGET_H
#define KLEFTWIDGET_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <qgsettings.h>
#include <QTimer>
#include <QScrollArea>

#include "sysresource/commoninfo.h"
#include "sysresource/cpuhistorywidget.h"
#include "sysresource/memhistorywidget.h"
#include "sysresource/nethistorywidget.h"
#include "sysresource/diskhistorywidget.h"

#include <kysdk/applications/knavigationbar.h>
using namespace kdk;

class KLeftWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KLeftWidget(QWidget *parent = nullptr);
    virtual ~KLeftWidget();

    void initUI();
    void initConnections();
    int currentIndex();

public slots:
    void onSwichPanel(int nIndex);
    void onResourceUpdate();

signals:
    void switchPage(int nIndex);
    void updateCpuStatus(QVector<double>);
    void updateMemStatus(qreal memUsed, qreal memTotal);
    void updateSwapStatus(qreal swapUsed, qreal swapTotal);
    void onUpdateNetStatus(qreal sendTotalBytes, qreal sendRate, qreal recvTotalBytes, qreal recvRate);
    void onUpdateDiskStatus();

private:
    bool getSwaponData(unsigned long long &swapSize, unsigned long long &swapUsed);
    void setleftClickedStatus(int nIndex);

private:
    QButtonGroup *m_buttonGroup = nullptr;
    QScrollArea *m_scrollOptions = nullptr;
    QFrame *m_frameOptions = nullptr;
    QVBoxLayout *m_layoutOptions = nullptr;

    KNavigationBar *m_buttonBar = nullptr;

    CpuHistoryWidget *m_cpuHisWidget = nullptr;
    MemHistoryWidget *m_memHisWidget = nullptr;
    NetHistoryWidget *m_netHisWidget = nullptr;
    DiskHistoryWidget *m_pDiskHisWidget = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_itemsLayout = nullptr;
    QVBoxLayout *m_resLayout = nullptr;

    QTimer *m_updateStatusTimer = nullptr;
    //cpu
    QVector<CpuUsageData> m_prevCpuData;

    //network
    unsigned long long int m_totalRecvBytes = 0;
    unsigned long long int m_totalSentBytes = 0;
    unsigned long long int m_rateRecvBytes = 0;
    unsigned long long int m_rateSentBytes = 0;

public:
    CpuHistoryWidget *getCpuHisWidget() {
        return m_cpuHisWidget;
    }
    MemHistoryWidget *getMemHisWidget() {
        return m_memHisWidget;
    }
    NetHistoryWidget *getNetHisWidget() {
        return m_netHisWidget;
    }
};

#endif // KLEFTWIDGET_H
