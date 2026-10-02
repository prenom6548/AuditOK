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


#ifndef PROCESSOPENFILE_H
#define PROCESSOPENFILE_H

#include "src/process/processbasic.h"

class ProcessOpenFile : public ProcessBasic
{
    Q_OBJECT
public:
    ProcessOpenFile(QString, QObject *parent = nullptr);
    ~ProcessOpenFile();
public:
    void setFile(QString);
    void exec();
protected:
    bool prepare();
protected slots:
    void slotsStart(){}
    void slotsError();
    void slotsOutput(){}
    void slotsFinished(int, QProcess::ExitStatus);
private:
    QProcess   *m_oPeony;
    QStringList m_oPeonyArgs;
    bool m_isSucceed;
};

#endif // PROCESSOPENFILE_H
