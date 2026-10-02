/*
* Copyright (C) Copyright 2021 KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301, USA.
**/

#include "simpledeskapplication.h"
#include <QDebug>
#include <QDir>

simpleDeskApplication::simpleDeskApplication(int &argc, char **argv) : QApplication(argc, argv)
{
    qputenv("XDG_CURRENT_DESKTOP", "UKUI");
    QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
    desktop_paths << "/usr/share/ukui/ukui-session-manager/simpledesk";

    XdgDesktopFileList all_file_list = XdgAutoStart::desktopFileList(desktop_paths, true);
    if (xdg_session_type == "wayland") {
        for (const auto &desktopFile : all_file_list) {
            QString execStr = desktopFile.value(QLatin1String("Exec")).toString();
            if (execStr.contains("kwin_x11")) {
                all_file_list.removeAll(desktopFile);
                break;
            }
        }
    }
    qDebug() << all_file_list.count();

    for (XdgDesktopFileList::const_iterator i = all_file_list.constBegin(); i != all_file_list.constEnd(); ++i) {
        startProcess(*i, true);
    }
}

simpleDeskApplication::~simpleDeskApplication() {}

void simpleDeskApplication::startProcess(const XdgDesktopFile &file, bool required)
{
    QStringList args = file.expandExecString();
    if (args.isEmpty()) {
        qWarning() << "Wrong desktop file: " << file.fileName();
        return;
    }

    QString name = QFileInfo(file.fileName()).fileName();
    if (!mNameMap.contains(name)) {
        UkuiModule *proc = new UkuiModule(file, this);
        // connect(proc, &UkuiModule::moduleStateChanged, this, &ModuleManager::moduleStateChanged);
        proc->startUKUIModule();

        mNameMap[name] = proc;

        if (required || autoRestart(file)) {
            connect(
                proc,
                SIGNAL(finished(int, QProcess::ExitStatus)),
                this,
                SLOT(restartModules(int, QProcess::ExitStatus)));
        }
    }
}

bool simpleDeskApplication::autoRestart(const XdgDesktopFile &file)
{
    QString auto_restart = "X-UKUI-AutoRestart";
    return file.value(auto_restart).toBool();
}

void simpleDeskApplication::restartModules(int /*exitCode*/, QProcess::ExitStatus exitStatus)
{
    UkuiModule *proc = qobject_cast<UkuiModule *>(sender());

    if (nullptr == proc) {
        qWarning() << "Got an invalid (null) module to restart, Ignoring it";
        return;
    }

    // 需要做出修改
    if (proc->restartNum > 10) {
        mNameMap.remove(proc->fileName);
        disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), nullptr, nullptr);
        if (proc->fileName == "ukui-screensaver-backend") {
            qDebug() << "ukui-screensaver-backend restart times out!";
        }
        proc->deleteLater();
        return;
    }

    if (!proc->isTerminating()) {
        // 根据退出码来判断程序是否属于异常退出。
        QString procName = proc->getProcName();
        if (proc->exitCode() == 0) {
            qDebug() << "Process" << procName << "(" << proc << ") exited correctly. "
                     << "With the exitcode = " << proc->exitCode() << ",exitStatus = " << exitStatus;
        } else {
            qDebug() << "Process" << procName << "(" << proc << ") has to be restarted. "
                     << "With the exitcode = " << proc->exitCode() << ",exitStatus = " << exitStatus;
            proc->startUKUIModule();
            proc->restartNum++;
            return;
        }
    }
    mNameMap.remove(proc->fileName);
    proc->deleteLater();
}
