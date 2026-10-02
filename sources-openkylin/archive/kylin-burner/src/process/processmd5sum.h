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


#ifndef PROCESSMD5SUM_H
#define PROCESSMD5SUM_H

#include "src/process/processbasic.h"

#include <QGSettings/QGSettings>

class ProcessMd5sum : public ProcessBasic
{
    Q_OBJECT
public:
    ProcessMd5sum(void *cdrom, QString checkFile, QObject *parent = nullptr);
    ~ProcessMd5sum();
protected:
    bool prepare() Q_DECL_OVERRIDE;
    bool worked();
    int countLinesInTxtFile(const QString &filePath);
protected slots:
    void slotsStart(){}
    void slotsOutput();
    void slotsError();
    void slotsFinished(int, QProcess::ExitStatus);
private:
    int     m_iProgress;
    int     m_numChecked;
    int     m_numCheckFiles;
    int     m_numFailedFile;
    void   *m_oCDROM;
    QString m_oCheckFile;
    QGSettings *auditburner;
    QString auditlogpath;
};

#endif // PROCESSMD5SUM_H
