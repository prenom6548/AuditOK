/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "systemtrayicon.h"

#include <QMenu>
#include <QAction>
#include "global/globalsignal.h"

SystemTrayIcon::SystemTrayIcon(QObject *parent) :
    QSystemTrayIcon(parent)
{
    initMenu();
    initIcon();
    setToolTip(tr("Video Player"));
}

SystemTrayIcon::~SystemTrayIcon()
{

}

void SystemTrayIcon::initIcon()
{
    setIcon(QIcon::fromTheme("kylin-video"));
}

void SystemTrayIcon::initMenu()
{
    menu = new QMenu;
    menu->setAccessibleName("kylin-video_systemtrayicon_QMenu_menu");
    actQuit = new QAction(tr("Quit"));
    actQuit->setIcon(QIcon::fromTheme("window-close-symbolic"));
    actQuit->setObjectName("kylin-video_QAction_actQuit");
    connect(actQuit, &QAction::triggered, [&](){emit sigQuit();});

    m_title = new QAction;
    m_title->setObjectName("kylin-video_systemtrayicon_QMenu_m_title");
    m_title->setIcon(QIcon::fromTheme("kylin-video"));
    m_title->setText(tr("Open Video"));

    connect(m_title, &QAction::triggered, [&](){
        g_user_signal->openApp();
    });

    menu->addAction(m_title);
    menu->addAction(actQuit);
    setContextMenu(menu);
}
