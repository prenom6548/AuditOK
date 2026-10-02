/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QtDBus>
#include "tray_win.h"
#include "job_manager_window.h"

TrayWin::TrayWin(QWidget *parent) : QWidget(parent)
{
    createActions();
    createTrayIcon();

    m_trayIcon->show();
}
void TrayWin::showNotify(QStringList notify)
{
    QIcon icon(QIcon::fromTheme("kylin-printer"));
    m_trayIcon->showMessage(notify.at(0), notify.at(1), icon,
                          10);
}
void TrayWin::createTrayIcon()
{
    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(m_minimizeAction);
    m_trayIconMenu->addAction(m_maximizeAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);

    m_trayIcon->setIcon(QIcon::fromTheme("kylin-printer"));
}
void TrayWin::createActions()
{
    m_minimizeAction = new QAction(tr("Minimize"), this);
    connect(m_minimizeAction, &QAction::triggered, this,[=](){
        JobManagerWindow *w = JobManagerWindow::getInstance();
        w->hideThis();
    });

    m_maximizeAction = new QAction(tr("Maximize"), this);
    connect(m_maximizeAction, &QAction::triggered, this,[=](){
        JobManagerWindow *w = JobManagerWindow::getInstance();
        w->showThis();
    });

    m_quitAction = new QAction(tr("Quit"), this);
    connect(m_quitAction, &QAction::triggered, this,[=](){
        JobManagerWindow *w = JobManagerWindow::getInstance();
        if (w->isActiveWindow())
        {
            w->close();
        }
        else
        {
            return;
        }
    });
}
