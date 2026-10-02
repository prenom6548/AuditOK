/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "installermainwidget_auto.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QDebug>
#include "../PluginService/ui_unit/messagebox.h"
#include <QToolTip>
#include <QDesktopWidget>
#include "ksetting.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../plugins/VirtualKeyboard/src/virtualkeyboard.h"
#include "../plugins/VirtualKeyboard/src/keyboardwidget.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/kthread.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/kdir.h"
#include <QTranslator>
using namespace KServer;
//桌面背景色
#define BGRGB qRgb(33, 83, 156)
namespace KInstaller {

InstallerMainWidget_auto::InstallerMainWidget_auto(bool oemflag, QWidget *parent)
    : QWidget(parent),
       bloemflag(oemflag),
       m_backQuickInstallThread(new QThread(this)),
       m_backquickProcess(new QuickInsatllerProcess)
{
    initUI();
    initBackground();
    addStyleSheet();
    initAllConnect();

    //切换语言
    QString tmp = KServer::ReadSettingIni("config", "language");
    if(tmp.isEmpty())
    {
        tmp = "zh_CN";
        WriteSettingToIni("config", "language", tmp);
    }
     QTranslator* translator1 = new QTranslator;
     if(translator1->load(KServer::GetKylinInstallPath() + "/language/" + tmp + ".qm"))
        qApp->installTranslator(translator1);
     QTranslator* translator2 = new QTranslator;
     if(translator2->load(KServer::GetKylinInstallPath() + "/language/qt_" + tmp + ".qm"))
        qApp->installTranslator(translator2);
     unsetenv("LANG");
     setenv("LANG", tmp.toStdString().c_str(),1);

    if(!bloemflag) {
        showInstallingFrame();
        m_backquickProcess->moveToThread(m_backQuickInstallThread);
        m_backQuickInstallThread->start();
    }

}

void InstallerMainWidget_auto::initAllConnect()
{
    connect(m_backQuickInstallThread, &QThread::started, m_backquickProcess, &QuickInsatllerProcess::exec);
    connect(m_backquickProcess, &QuickInsatllerProcess::signalProcessStatus, [=](int exitcode){
        if(exitcode == 0) {
            showFinishedFrame();
        } else {
            showInstallingErrorFrame();
        }
    });
    connect(m_backQuickInstallThread, &QThread::finished, m_backquickProcess, &QuickInsatllerProcess::deleteLater);
}


InstallerMainWidget_auto::~InstallerMainWidget_auto()
{
   quitThreadRun(m_backQuickInstallThread);
}
void InstallerMainWidget_auto::initUI()
{
    this->setObjectName("InstallerMainWidget_auto");

    QGridLayout* gLayoutl = new QGridLayout();
    gLayoutl->setContentsMargins(40, 140, 40, 80);


    //加载中间窗口显示
    m_stackWidget = new QStackedWidget(this);
//    gLayoutl->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0, 1, 1);
    gLayoutl->addWidget(m_stackWidget, 0, 0, 1, 1, Qt::AlignCenter);
    if(!bloemflag) {
        m_installingFrame = new InstallingFrame();
        m_installingFrame->show();
        m_stackWidget->addWidget(m_installingFrame);
        m_stackWidget->setCurrentWidget(m_installingFrame);
    } else {
        m_oemFrame = new InstallingOEMConfigFrame;
        m_oemFrame->setParent(this);
        m_oemFrame->show();
        m_stackWidget->addWidget(m_oemFrame);
        m_stackWidget->setCurrentWidget(m_oemFrame);
    }

    this->setLayout(gLayoutl);


}
void InstallerMainWidget_auto::initBackground()
{
    // 无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //全屏
    showFullScreen();
    //设置背景blue色
    QRect rect = KServer::GetScreenRect();
    QPixmap pixmap = QPixmap(":/data/png/openkylin-bk.jpg").scaled(rect.width(), rect.height());
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}


void InstallerMainWidget_auto::addStyleSheet()
{
    QFile file(":/data/qss/installermainwidget.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void InstallerMainWidget_auto::showInstallingFrame()
{
    connect(m_installingFrame, &InstallingFrame::signalFinished, this, &InstallerMainWidget_auto::showFinishedFrame);
    connect(this, &InstallerMainWidget_auto::signalScriptFinished, m_installingFrame, &InstallingFrame::findFile);
    connect(this, &InstallerMainWidget_auto::signalInstallingClose, m_installingFrame, &InstallingFrame::closeW);

    connect(this, &InstallerMainWidget_auto::signalError, m_installingFrame, &InstallingFrame::showError);
}

void InstallerMainWidget_auto::showInstallingErrorFrame()
{
    emit signalError();
    emit signalInstallingClose();
   // m_installingFrame->showErrorFrame();
}

void InstallerMainWidget_auto::showFinishedFrame()
{
    emit signalInstallingClose();
    //QProcess::execute("systemctl reboot");
    int var = KServer::AutoReadSettingIni("config", "reboot").toInt();
    if(var == 1) {
        QProcess::execute("reboot");
    } else if(var == 0) {
        QProcess::execute("poweroff");
    }

}


}
