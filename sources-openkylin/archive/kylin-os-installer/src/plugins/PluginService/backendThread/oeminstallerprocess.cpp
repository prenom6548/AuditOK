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

#include "oeminstallerprocess.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/kdir.h"
#include <QDebug>

using namespace KServer;
namespace KInstaller {
OemInstallerProcess::OemInstallerProcess(QObject *parent) : QObject(parent)
{

}
void OemInstallerProcess::exec()
{
    QString error = "";
    QString output =  "";
    int exitcode = 0;
    QString ScriptPath =  KServer::GetKylinInstallPath() + "/scripts" ;
    bool isOK = false;
    isOK = KServer::KCommand::getInstance()->RunScripCommand("sudo", {"/bin/bash","oem.sh"}, ScriptPath,  output, error, exitcode);
    if(!isOK) {
        emit signalProcessStatus(exitcode);//tr("An exception occurred during script execution!")
    } /*else {
        emit signalProcessStatus(exitcode);
    }*/
    qDebug() << "error:" << error;
}
}
