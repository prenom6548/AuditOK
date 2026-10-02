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

#include "processopenfile.h"
#include "src/frame/logger.h"
#include "src/po/translation.h"

#include <QDebug>

ProcessOpenFile::ProcessOpenFile(QString path, QObject *parent) :
    ProcessBasic(parent)
{
    m_oProcessName = _("OpenInLocal");
    m_oArguments << path;
}

void ProcessOpenFile::setFile(QString path)
{
    m_oArguments.clear();
    m_oArguments << path;
}

void ProcessOpenFile::exec()
{
    m_isSucceed = true;
    prepare();
    m_oProcess->disconnect();
    connect(m_oProcess, SIGNAL(started()), this, SLOT(slotsStart()));
    connect(m_oProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotsOutput()));
    connect(m_oProcess, SIGNAL(readyReadStandardError()), this, SLOT(slotsError()));
    connect(m_oProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotsFinished(int,QProcess::ExitStatus)));

    m_oProcess->setArguments(m_oArguments);
    m_oProcess->start();
}

bool ProcessOpenFile::prepare()
{
    m_oProcess->setProgram("xdg-open");
    return true;
}

void ProcessOpenFile::slotsError()
{
    QString errorMsg = m_oProcess->readAllStandardError();
    QString logcontent = _("Error happened when open local,content: %1").arg(errorMsg);
    qDebug() << logcontent;
    LOGGER->error(logcontent);

    if (errorMsg.contains("No application") || errorMsg.contains("Launch failed") ||  errorMsg.contains("not found")) {
        m_isSucceed = false;
    }
}

void ProcessOpenFile::slotsFinished(int errCode, QProcess::ExitStatus status)
{
    if (status == QProcess::NormalExit && errCode == 0 && m_isSucceed)
    {
        qDebug() << "打开成功";
    } else {
        emit fail(m_oProcessName);
    }
}

ProcessOpenFile::~ProcessOpenFile()
{}
