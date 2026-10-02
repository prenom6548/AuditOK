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

#include "kcommand.h"
#include <QDebug>
#include <QProcess>
#include <QDir>
#include "klogger.h"

namespace KServer {
const QString runLogPath = "/var/log/installer/runscripcommand.log";
KCommand* KCommand::m_cmdInstance = nullptr;

KCommand::KCommand(QObject* parent ):QObject(parent)
{
    //Q_ASSERT(m_cmdInstance);
    m_cmdInstance = this;

}

KCommand* KCommand::initCmd()
{
    if(!m_cmdInstance) {
        qCritical() << QObject::tr("KCommand::m_cmdInstance is not init.");
        return new KCommand();
    }
    return m_cmdInstance;
}

KCommand::~KCommand()
{

}

KCommand* KCommand::getInstance()
{
    return m_cmdInstance;
}

bool KCommand::RunScripCommand(const QString& program,
                               const QStringList& args,
                               QString& workingPath,
                               QString& output,
                               QString& error,
                               int& exitCode)
{
//    if(args.isEmpty()) {
//        qDebug() << "脚本文件为空,不能继续执行 \n";
//        return false;
//    }
    QProcess process;
    process.setProgram(program);
    process.setArguments(args);

    qDebug() << "QProcess program:" << program;
    qDebug() << "QProcess args:" << args;
    if(!workingPath.isEmpty()) {
        if(QDir(workingPath).exists()) {
            process.setWorkingDirectory(workingPath);
        } else {
            qWarning() << QObject::tr("WorkingPath is not found. \n");
        }
    }

    process.start();
    qDebug()<< "KCommand::RunScripCommand   output:" << process.readLine();
    process.waitForFinished(-1);

    output = process.readAllStandardOutput();
    error = process.readAllStandardError();
    WriteLogFile(runLogPath, output);
    exitCode = process.exitCode();
    qDebug() << error<< process.exitCode() ;
     return (process.exitStatus() == QProcess::NormalExit &&
             process.exitCode() == 0);

}
bool KCommand::RunScripCommand(const QString& args,
                               QString& workingPath,
                               QString& output,
                               QString& error,
                               int& exitCode)
{
    if(args.isEmpty()) {
        qDebug() << QObject::tr("Shell file is empty, does not continue. \n");
        return false;
    }
    QProcess process;
    if(!workingPath.isEmpty()) {
        if(QDir(workingPath).exists()) {
            process.setWorkingDirectory(workingPath);
        } else {
            qWarning() << QObject::tr("WorkingPath is not found. \n");
        }
    }
    qDebug() << "QProcess args:" << args;
    process.start(args);
    qDebug()<< "KCommand::RunScripCommand   output:"  << process.readLine();
    qDebug() << "命令:" << args;
//    if(!process.waitForStarted()) {
//        qWarning() << "进程" << args<< "执行失败!" << process.error();
//    }

//    process.close();
    process.waitForFinished(-1);

    output = process.readAllStandardOutput();
    error = process.readAllStandardError();
    WriteLogFile(runLogPath, output);
    exitCode = process.exitCode();
    qDebug() << output<< error<< process.exitCode() ;
     return (process.exitStatus() == QProcess::NormalExit &&
             process.exitCode() == 0);
}
}
