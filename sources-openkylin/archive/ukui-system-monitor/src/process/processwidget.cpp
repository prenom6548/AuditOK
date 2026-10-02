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
#include "processwidget.h"

#include <QPainterPath>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
#include <QDebug>

#include "../style/usmproxystyle.h"
#include "../macro.h"
#include "../util.h"

#include "../process/process_monitor.h"
using namespace sysmonitor::process;

ProcessWidget::ProcessWidget(QSettings *proSettings, QWidget *parent)
    : QWidget(parent)
    , m_proSettings(proSettings)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initHistoryMode();
    initConnections();
    onSwitchProcType(m_nWhichNum);
}

ProcessWidget::~ProcessWidget()
{
    if (m_ifSettings) {
        delete m_ifSettings;
        m_ifSettings = nullptr;
    }
}

void ProcessWidget::onWndClose()
{
    if (m_procTabView) {
        m_procTabView->onWndClose();
    }
}

void ProcessWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(1);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);

    painter.drawPath(path);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ProcessWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(12,0,0,0);
    m_mainLayout->setSpacing(0);
    m_topBtnsLayout = new QHBoxLayout();
    m_topBtnsLayout->setContentsMargins(0,0,0,16);
    m_topBtnsLayout->setSpacing(0);
    m_topBtnsLayout->setAlignment(Qt::AlignHCenter);
    m_procTableLayout = new QVBoxLayout();
    m_procTableLayout->setContentsMargins(0,0,0,0);
    m_procTableLayout->setSpacing(0);

    // 初始化控件
    processTabBar = new kdk::KTabBar(kdk::SegmentDark);
    processTabBar->setFixedWidth(NORMALWIDTH*3);
    processTabBar->addTab(tr("Applications"));
    processTabBar->addTab(tr("My Processes"));
    processTabBar->addTab(tr("All Process"));
    processTabBar->setAccessibleTabName(0, "Applications");
    processTabBar->setAccessibleTabName(1, "My Processes");
    processTabBar->setAccessibleTabName(2, "All Process");
    m_topBtnsLayout->addWidget(processTabBar);
    
    //初始化进程列表
    m_procTabView = new ProcessTableView(m_proSettings);
    m_procTabView->installEventFilter(this);
    m_procTableLayout->addWidget(m_procTabView);

    m_mainLayout->addLayout(m_topBtnsLayout);
    m_mainLayout->addLayout(m_procTableLayout);
    this->setLayout(m_mainLayout);
}

void ProcessWidget::initConnections()
{
    connect(this,SIGNAL(changeProcessItemDialog(int)), m_procTabView, SLOT(onChangeProcessFilter(int)));
    connect(processTabBar, SIGNAL(currentChanged(int)), this, SLOT(onSwitchProcType(int)));
    connect(this, SIGNAL(searchSignal(QString)), m_procTabView, SLOT(onSearch(QString)), Qt::QueuedConnection);

    auto *monitor = ProcessMonitor::instance();
    //update process count from model's cache on process processupdated signal
    connect(monitor->processList(), &ProcessList::procCountUpdated, this, &ProcessWidget::updateProcCount);
}

void ProcessWidget::updateProcCount(int appCount, int myCount, int AllCount)
{
    processTabBar->setTabText(0, tr("Applications(%1)").arg(appCount));
    processTabBar->setTabText(1, tr("My Processes(%1)").arg(myCount));
    processTabBar->setTabText(2, tr("All Process(%1)").arg(AllCount));
}

void ProcessWidget::initHistoryMode()
{
    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid)) {
        m_ifSettings = new QGSettings(ifid);
    }
    if (m_ifSettings && m_ifSettings->keys().contains(WHICH_MENU_KEY)) {
        auto whichNum = m_ifSettings->get(WHICH_MENU);
        if (whichNum.isValid()) {
            m_nWhichNum =  whichNum.toInt();
        }
    }

    // set current tab
    if (m_nWhichNum == PROCESSTYPE_ACTIVE) {
        m_nWhichNum = PROCESSTYPE_APPLICATION;
    }
    if (m_nWhichNum > PROCESSTYPE_APPLICATION || m_nWhichNum < PROCESSTYPE_ACTIVE) {
        m_nWhichNum = PROCESSTYPE_APPLICATION;
    }

    processTabBar->blockSignals(true);
    if (m_nWhichNum == PROCESSTYPE_APPLICATION) {
           processTabBar->setCurrentIndex(0);
       } else {
           processTabBar->setCurrentIndex(m_nWhichNum);
       }
    processTabBar->blockSignals(false);

    ProcessMonitor::instance()->processList()->setScanFilter(m_nWhichNum);
}

void ProcessWidget::onSwitchProcType(int nType)
{
    qDebug() << Q_FUNC_INFO << nType;
    if (nType == PROCESSTYPE_ACTIVE) {
        nType = PROCESSTYPE_APPLICATION;
    }
    if (nType > PROCESSTYPE_APPLICATION || nType < PROCESSTYPE_ACTIVE) {
        nType = PROCESSTYPE_APPLICATION;
    }

    Q_EMIT changeProcessItemDialog(nType);
    if (m_ifSettings) {
        m_ifSettings->set(WHICH_MENU, nType);
    }
}
