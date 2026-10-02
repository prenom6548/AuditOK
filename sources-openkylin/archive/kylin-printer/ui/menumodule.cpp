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

#include <QTime>
#include <QUrl>
#include <QProcess>
#include "menumodule.h"
#include "config_ip_address_dialog.h"
#include "size_para.h"
#include "custom_about_dialog.h"
#include "information_collector.h"

#ifdef USE_SELF_CONTAINED_DEP
    #include "kmenubutton.h"
#else
    #include <kysdk/applications/kmenubutton.h>
#endif // USE_SELF_CONTAINED_DEP

#include "common.h"

MenuModule::MenuModule(QWidget *parent)
  : QMenu(parent),
    m_mainWin(parent)
{
    init();
}

bool checkClassicPrinterInstalled()
{
    QString packageVersion = getPackageVersion("system-config-printer");
    if (packageVersion.isEmpty()) {
        return false;
    }
    return true;
}

void MenuModule::init()
{
    QList<QAction *> actions ;
    kdk::KMenuButton *menuButton = new kdk::KMenuButton();
    menuButton->feedbackAction();

    QAction *actionConfig = new QAction(this);
    actionConfig->setText(tr("Configure"));
    QAction *actionClassicPrinter = new QAction(this);
    actionClassicPrinter->setText(tr("Classic Printer"));
    QAction *actionAbout = new QAction(this);
    actionAbout->setText(tr("About"));
    QAction *actionHelp = new QAction(this);
    actionHelp->setText(tr("Help"));
    QAction *actionQuit = new QAction(this);
    actionQuit->setText(tr("Quit"));
    actions << actionConfig;

    if (checkClassicPrinterInstalled()) {
        actions << actionClassicPrinter;
    }
    
    actions << actionAbout
            << actionHelp
            << menuButton->feedbackAction()
            << actionQuit;

    this->addActions(actions);

    connect(this, &QMenu::triggered, this, &MenuModule::triggerMenu);
}

MenuModule::~MenuModule()
{

}

void MenuModule::triggerMenu(QAction *act)
{
    QString str = act->text();
    if(tr("Quit") == str){
        qDebug() << "从menu中的quit退出程序！";
        emit menuModuleClose();
    }
    else if(tr("About") == str){
        aboutAction();
    }
    else if(tr("Help") == str){
        helpAction();
    }
    else if(tr("Configure") == str){
        ConfigIPAddressDialog configDialog(m_mainWin);
        if(configDialog.getConfigStatus())
        {
            configDialog.show();
            configDialog.exec();
        }
        else
        {
            return;
        }
    }
    else if (tr("Classic Printer") == str) {
        InformationCollector::getInstance().addMessage("exec system-config-printer");
        QString cmd = "system-config-printer";
        QProcess process(this);
        process.startDetached(cmd);
    }
}

void MenuModule::aboutAction()
{
    CustomAboutDialog dialog(m_mainWin);
    dialog.exec();
}

void MenuModule::helpAction()
{
    DaemonIpcDbus *ipcDbus = DaemonIpcDbus::getInstance();
    if(!ipcDbus->daemonIsNotRunning()){
        ipcDbus->showGuide(m_appName);
    }
}
