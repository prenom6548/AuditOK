/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Ukui2TrayIcon.h"

#include <QProcess>
#include <iostream>

#include "IMainWindow.h"

UkuiVersionTwoTrayIcon::UkuiVersionTwoTrayIcon(UkmediaTrayIcon* trayIcon, IMainWindow* window)
    : ISoundSystemTrayIcon(trayIcon), m_pMainWid(window)
{

}

void UkuiVersionTwoTrayIcon::initTray()
{
    m_pMenu = new QMenu();
    m_pMenu->setAccessibleName(tr("Sound tray menu"));
    m_pMenu->setAttribute(Qt::WA_NoMouseReplay);
    QAction* soundPreferenceAction = new QAction(tr("Sound preference ukui 2"), this);
    soundPreferenceAction->setIcon(QIcon::fromTheme("document-page-setup-symbolic"));
    QAction* muteAction = new QAction(tr("Mute 2"), this);

    m_pMenu->addAction(muteAction);
    m_pMenu->addAction(soundPreferenceAction);

    m_pTrayIcon->setContextMenu(m_pMenu);
    connect(soundPreferenceAction, SIGNAL(triggered()), this, SLOT(soundPreferenceActionTriggered()));
    connect(muteAction, SIGNAL(triggered()), this, SLOT(muteActionTriggered()));
}

void UkuiVersionTwoTrayIcon::initSlot()
{
    connect(m_pTrayIcon, SIGNAL(wheelRollEventSignal(bool)), this, SLOT(trayWheelRollEventSlot(bool)));

    connect(m_pTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),\
            this, SLOT(activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason)));
}

void UkuiVersionTwoTrayIcon::setVisible(bool visible)
{
    m_pTrayIcon->setVisible(visible);
}

void UkuiVersionTwoTrayIcon::setIcon(const QIcon& icon)
{
    m_pTrayIcon->setIcon(icon);
}


void UkuiVersionTwoTrayIcon::soundPreferenceActionTriggered()
{
    QProcess::startDetached(QString("ukui-control-center -m Audio"));
}

void UkuiVersionTwoTrayIcon::activatedSystemTrayIconSlot(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
    // 鼠标中间键点击图标
    case QSystemTrayIcon::MiddleClick: {
        break;
    }
    // 鼠标左键点击图标
    case QSystemTrayIcon::Trigger: {
        m_pMainWid->triggerWidget();
        std::cout << "QSystemTrayIcon::Trigger" << std::endl;
        break;
    }
    // 鼠标左键双击图标
    case QSystemTrayIcon::DoubleClick: {
        std::cout << "QSystemTrayIcon::DoubleClick" << std::endl;
    }
    // 右键菜单
    case QSystemTrayIcon::Context:{
        std::cout << "QSystemTrayIcon::Context" << std::endl;
        break;
    }
    default:
        break;
    }
}
