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

#include <QDebug>
#include <QApplication>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QFile>
#include <QtGui>
#include <QtCore>
#include <QWidget>
#include <QTimer>
#include <unistd.h>
#include "main_controller.h"
#include "ipc/ipc_dbus.h"

// #include "xatom-helper.h"
#include "ukuistylehelper/ukuistylehelper.h"

MainController* MainController::mSelf = 0;
MainController* MainController::self()
{
    if (!mSelf) {
        mSelf = new MainController;
    }
    return mSelf;
}

bool MainController::isSwArchitecture() {
    QFile cpuInfoFile("/proc/cpuinfo");
    if (!cpuInfoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开 /proc/cpuinfo 文件";
        return false;
    }

    QTextStream in(&cpuInfoFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("sw", Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}


MainController::MainController()
{
    qDebug() << Q_FUNC_INFO << QString::fromLocal8Bit(qgetenv("XDG_RUNTIME_DIR")) << " gStartShowApp = " << gStartShowApp;
    ipcDbus = new IpcDbus;
    ipcDbus->init();
    guideWidget = new GuideWidget;
    kdk::UkuiStyleHelper::self()->removeHeader(guideWidget);
    if (gStartShowApp == "") {
        guideWidget->show();
    }

    startShowApp();
}

void MainController::startShowApp()
{
    qDebug() << Q_FUNC_INFO;
    uid_t user;
    user = geteuid();
    if(0 == user){
        showGuide(gStartShowApp);
    }
    QTimer::singleShot(800, this, SLOT(showGuide()));//消息循环起来后打通js和qt通信在执行跳转
}

void MainController::showGuide(QString appName)
{
    if(appName!="")
        guideWidget->jump_app(appName);

    QList<QScreen *> list = QGuiApplication::screens();
    if(guideWidget->isMinimized()){
        if(guideWidget->size() == QSize(list.at(0)->availableGeometry().width(),list.at(0)->availableGeometry().height()))
            guideWidget->showMaximized();
        else
            guideWidget->showNormal();
    }

    flags = guideWidget->windowFlags();
    guideWidget->setWindowFlags(flags|Qt::WindowStaysOnTopHint);
    QTimer::singleShot(10, this, SLOT(setwindowFlags()));
    guideWidget->show();
}

void MainController::showGuide()
{   
    if(gStartShowApp!="")
    {
        qDebug() << Q_FUNC_INFO;
        guideWidget->jump_app(gStartShowApp);
        gStartShowApp = "";
    }
    guideWidget->activateWindow();
    guideWidget->show();
}

void MainController::setwindowFlags()
{
    guideWidget->setWindowFlags(flags);
    guideWidget->show();
}

MainController::~MainController()
{

}

