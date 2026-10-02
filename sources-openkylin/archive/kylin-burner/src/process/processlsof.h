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


#ifndef PROCESSLSOF_H
#define PROCESSLSOF_H

#include <QObject>

class DataCDROM;

class ProcessLsof : public QObject
{
    Q_OBJECT
public:
    ProcessLsof(QObject *parent=nullptr);
    ~ProcessLsof();

    bool checkDevice(DataCDROM *cdrom);

    struct Process {
        QString name;
        int pid;
    };
    const QList<Process>& usingApplications() const;
    QString joinProcessNames( const QList<ProcessLsof::Process>& );

private:
    bool findLsofExecutable();

    class Private;
    Private* d;
};

#endif // PROCESSLSOF_H
