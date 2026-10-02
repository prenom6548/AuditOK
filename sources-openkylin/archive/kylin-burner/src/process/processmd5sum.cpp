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

#include "processmd5sum.h"
#include "src/frame/logger.h"
#include "src/data/datacdrom.h"
#include "src/po/translation.h"
#include "src/view/modaldialog.h"
#include "src/frame/auditlog.h"
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

ProcessMd5sum::ProcessMd5sum(void *cdrom, QString checkFile, QObject *parent) :
    ProcessBasic(parent),
    m_oCDROM(cdrom),
    m_oCheckFile(checkFile),
    m_iProgress(0)
{
    m_oProcessName = _("Disk Check");
    LOGGER->registerName(this, _("PROCESS_DC"));
    title(_("Checking disk"));
    m_numCheckFiles = countLinesInTxtFile(m_oCheckFile);
    m_numChecked = 0;
    m_numFailedFile = 0;
    this->setRange(0,100);

    connect(this, &ProcessMd5sum::cancel, ModalDialog::instance(), &ModalDialog::cancelEnable);
}

int ProcessMd5sum::countLinesInTxtFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         LOGGER->error(QString("open md5 file %1 failed").arg(filePath));
        return -1; // 返回一个错误码，表示文件打开失败
    }

    QTextStream in(&file);
    int lineCount = 0;
    while (!in.atEnd()) {
        QString line = in.readLine(); // 读取一行并存储到字符串中
        if (line.contains("./")) { // 检查是否包含特定的字符串
            ++lineCount; // 如果包含，则增加计数器
        }

    }

    file.close();
    return lineCount;
}

bool ProcessMd5sum::prepare()
{
    DataCDROM *cdrom;

    PROGRESS;
    MODAL->show();
    emit cancel(false);

    cdrom = static_cast<DataCDROM *>(m_oCDROM);
    if (cdrom->mediumMountPoint().isEmpty()) return false;
    QDir::setCurrent(cdrom->mediumMountPoint());
    m_oProcess->setProgram("md5sum");
    m_oArguments << "-c";
    m_oArguments << m_oCheckFile;
    m_oProcess->setArguments(m_oArguments);
    setValue(++m_iProgress, m_oIcon, _("Checking..."));
    return true;
}

bool ProcessMd5sum::worked()
{
    if (m_bCancel) return false;
    if (QProcess::NormalExit != m_oProcess->exitStatus() && 0 != m_numFailedFile) return false;
    return m_oKeep;
}


void ProcessMd5sum::slotsOutput()
{
#ifndef UNIT_TEST
    QString output = m_oProcess->readAllStandardOutput();

    QStringList retProcessList = output.split("\n");
    for(int i = 0; i < retProcessList.size();i++)
    {
        QString tmpOutput = retProcessList.at(i);
        if (tmpOutput == "" || !tmpOutput.contains("./"))
            continue;

        LOGGER->info(tmpOutput);
        ++m_numChecked;

        m_iProgress = 1 + (int)((m_numChecked)/((double)m_numCheckFiles) * 99);

        setValue(m_iProgress, m_oIcon, _("Checking..."));
    }
#endif
}

void ProcessMd5sum::slotsError()
{
    QString error = m_oProcess->readAllStandardError();
    LOGGER->error(_("Error happened when checking disk,content: %1").arg(error));
}

void ProcessMd5sum::slotsFinished(int errCode, QProcess::ExitStatus status)
{
#ifndef UNIT_TEST
    setValue(100, m_oIcon, _("Checking..."));
    MODAL->hide();

    if (QProcess::NormalExit == status && 0 == errCode) {
        m_oKeep = true;
        LOGGER->info( QString("file:[%1] line:[%2] : slotsFinished() Medium md5sum data success.").arg(__FILE__).arg(__LINE__) );
        QDir::setCurrent(QDir::homePath());
    }else{
        m_oKeep = false;
        LOGGER->error( QString("file:[%1] line:[%2] : slotsFinished() Medium md5sum data failure. [%3]").arg(__FILE__).arg(__LINE__).arg(m_oProcess->errorString()) );
        QDir::setCurrent(QDir::homePath());
    }

    bool m_isSpecialMachine;
    AuditLog md5checkLog;

    auditburner=new QGSettings("org.ukui.kylinburner");
    auditlogpath="/var/log/kylin-burner/os_audit.log";
    m_isSpecialMachine=auditburner->get("burneraudit-isrunning").toBool();

    if(m_isSpecialMachine==true){
        auditburner->set("burner-os-auditlog-name",auditlogpath);
        DataCDROM* cdrom = (DataCDROM*)m_oCDROM;
        md5checkLog.outputResult(cdrom->display(), cdrom->mediumType(), m_oProcessName, true==m_oKeep);
    }else{
        auditburner->set("burner-os-auditlog-name","");
    }
#endif
}

ProcessMd5sum::~ProcessMd5sum()
{}
