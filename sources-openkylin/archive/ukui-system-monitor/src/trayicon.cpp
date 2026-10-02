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
#include "trayicon.h"

#include <KWindowSystem>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

TrayIcon::TrayIcon(CpuHistoryWidget*cpu, MemHistoryWidget*mem, NetHistoryWidget*net, FileSystemWidget *disk, QWidget *parent)
    : m_cpuHisWidget(cpu),
      m_memHisWidget(mem),
      m_netHisWidget(net),
      m_fileSystemWidget(disk),
      QWidget(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint)
{
    initTrayIcon();
    initMenu();
    initConnect();
    mSystemTray->setVisible(true);
}


TrayIcon::~TrayIcon()
{
}

void TrayIcon::initTrayIcon()
{
    mSystemTray = new QSystemTrayIcon(this);
    mSystemTray->setIcon(QIcon::fromTheme("ukui-monitor-data-symbolic"));
    mSystemTray->setToolTip(tr("System Monitor"));
    m_pTimer = new QTimer();
    connect(m_pTimer, &QTimer::timeout, this, &TrayIcon::updateToolTips);

    m_pTimer->start(1000);
}

void TrayIcon::initMenu()
{
    /* 创建菜单和行为实例 */
    mMenu = new QMenu(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mMenu, "TrayIcon", "", "");
    mQuitAction = new QAction(mMenu);    //退出
    KDK_EXTEND_OBJ_INFO_FORMAT(mQuitAction, "TrayIcon", "");
    mOpenAction = new QAction(mMenu);
    KDK_EXTEND_OBJ_INFO_FORMAT(mOpenAction, "TrayIcon", "");

    mQuitAction->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mQuitAction->setText(tr("Close"));
    mOpenAction->setIcon(QIcon::fromTheme("ukui-system-monitor"));
    mOpenAction->setText(tr("Open monitor"));

    mMenu->addAction(mOpenAction);
    mMenu->addSeparator();
    mMenu->addAction(mQuitAction);

    mSystemTray->setContextMenu(mMenu);
}

void TrayIcon::initConnect()
{
    connect(mQuitAction, &QAction::triggered, this, [=](){
        Q_EMIT needSaveSetting();
        qApp->exit();
    });

    connect(mOpenAction, &QAction::triggered, this, [=](){
        emit openMonitor(QString());
    });

    connect(mSystemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(ActiveTray(QSystemTrayIcon::ActivationReason)));
}

void TrayIcon::ActiveTray(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
        case QSystemTrayIcon::DoubleClick:
            qDebug()<<">>>>>>> 鼠标双击~";
            break;
        case QSystemTrayIcon::Trigger:
            qDebug()<<">>>>>>> 鼠标左键点击~";
            emit openMonitor(QString("trayicon"));
            break;
        case QSystemTrayIcon::MiddleClick:
            qDebug()<<">>>>>>> 鼠标中键点击~";
            break;
        case QSystemTrayIcon::Context:
            qDebug()<<">>>>>>> 鼠标右键点击~";
            break;
        default:
            break;
    }
}

void TrayIcon::updateToolTips()
{
    QString network = tr("NET: ") + m_netHisWidget->getNetSendSpeed() + "    "+ m_netHisWidget->getNetLoadSpeed();
    QString cpuAndNet = tr("CPU: ") + m_cpuHisWidget->getCpuOccupancy();
    QString memoryAndNetSend = tr("RAM: ") + m_memHisWidget->getMemOccupancy();
    QString diskOccupancy = m_fileSystemWidget->getDiskOccupancy();

    QString diskAndNetLoad = tr("Disk: ") + diskOccupancy;
    mSystemTray->setToolTip(cpuAndNet + "\n" + memoryAndNetSend + "\n" + diskAndNetLoad + "\n" + network);
}
