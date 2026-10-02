/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "utils.h"
#include "libkysysinfo.h"
#include "src/po/translation.h"

#include <QStringList>
#include <kmessagebox.h>

QString SystemInfo::m_os;
QString SystemInfo::m_arch;
bool SystemInfo::m_isWayland;

QString Utils::getArch()
{
    char *cArch;
    QString sArch;
    cArch = kdk_system_get_architecture();
    if (cArch) {
        sArch = QString(cArch);
        free(cArch);
    }
    return sArch;
}

QString Utils::getOs()
{
    char *cSystem;
    QString sSystem;
    cSystem = kdk_system_get_systemName();
    if (cSystem) {
        sSystem = QString(cSystem);
        free(cSystem);
    }
    return sSystem;
}

bool Utils::isWayland()
{
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "ukui-shell");
        return true;
    } else {
        return false;
    }
}

void Utils::initSystemInfo()
{
    //SystemInfo::m_os = Utils::getOs();
    //SystemInfo::m_arch = Utils::getArch();
    SystemInfo::m_isWayland = Utils::isWayland();
}

bool MyMessageBox::showIsoReplaceMessageBox(QString fileName)
{
#ifndef UNIT_TEST
    kdk::KMessageBox msgBox;
    msgBox.setText(_("There is already have the iso file named %1,"
                     "replace it or not?").arg(fileName));
    msgBox.setIconPixmap(QIcon::fromTheme("dialog-question").pixmap(25,25));
    QPushButton *btnOk = msgBox.addButton(_("OK"), kdk::KMessageBox::ButtonRole::AcceptRole);
    QPushButton *btnCancel = msgBox.addButton(_("Cancel"), kdk::KMessageBox::ButtonRole::RejectRole);
    msgBox.exec();

    if (msgBox.clickedButton() == btnOk) {
        return true;
    } else if (msgBox.clickedButton() == btnCancel) {
        return false;
    }
#endif
    return false;
}
