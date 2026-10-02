/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tray.h"
#include "kmrewindow.h"
#include "appsettings.h"
#include "kmreenv.h"
#include "sessionsettings.h"

#include <QAction>
#include <QSystemTrayIcon>
#include <QProcess>
#include <QMenu>


Tray::Tray(KmreWindow* window)
    : QObject(window)
    , mMainWindow(window)
{
    //Attention: Wayland下设置hide()后再立即设置show()会导致Qt窗口异常退出,故wayland下不设置托盘
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        return;
    }

    AppSettings::AppConfig appConfig = AppSettings::getInstance().getAppConfig(window->getPackageName());
    if (QSystemTrayIcon::isSystemTrayAvailable() && appConfig.trayEnabled) {

        QAction *openWindowAction = new QAction(tr("Open App"), this);
        connect(openWindowAction, &QAction::triggered, window, &KmreWindow::showMainWindow);

        QAction *prefAction = new QAction(tr("Preference"), this);
        connect(prefAction, &QAction::triggered, window, [] {
            if (QFile::exists("/usr/bin/kylin-kmre-settings")) {
                QProcess::startDetached("/usr/bin/kylin-kmre-settings");
            }
        });

        QAction *quitAction = new QAction(tr("Quit App"), this);
        connect(quitAction, &QAction::triggered, window, [&] () {
            if (!mMainWindow->isScreenLocked()) {
                mMainWindow->closeWindow(true);
            }
        });

        mTrayIconMenu = new QMenu(window);
        mTrayIconMenu->addAction(openWindowAction);
        mTrayIconMenu->addAction(prefAction);
        mTrayIconMenu->addSeparator();
        mTrayIconMenu->addAction(quitAction);

        QIcon trayIcon = QIcon(window->getIconPath());
        if (trayIcon.isNull()) {
            trayIcon = QIcon::fromTheme("kmre");
        }
        mSystemTrayIcon = new QSystemTrayIcon(this);
        mSystemTrayIcon->setIcon(trayIcon);
        mSystemTrayIcon->show();

        mSystemTrayIcon->setToolTip(QString("%1 (KMRE)").arg(window->windowTitle()));
        mSystemTrayIcon->setContextMenu(mTrayIconMenu);
        connect(mSystemTrayIcon, &QSystemTrayIcon::activated, [&] (QSystemTrayIcon::ActivationReason reason) {
            switch (reason) {
                case QSystemTrayIcon::Trigger:
                {
                    if (mMainWindow->isMinimized() || mMainWindow->isHidden()) {
                        mMainWindow->showMainWindow();
                    }
                    else {
                        if (!mMainWindow->isActiveWindow()) {
                            mMainWindow->activateWindow();
                        }
                        else {
                            mMainWindow->minimizeWindow();
                        }
                    }
                    break;
                }
                case QSystemTrayIcon::DoubleClick:
                    break;
                case QSystemTrayIcon::MiddleClick:
                    break;
                default:
                    break;
            }
        });
    }
}

void Tray::setIcon(QIcon &icon)
{
    if (mSystemTrayIcon) {
        mSystemTrayIcon->setIcon(icon);
    }
}
