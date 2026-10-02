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

#include "processlsof.h"
#include "../data/datacdrom.h"
#include "../frame/logger.h"

#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QStandardPaths>

#include <sys/types.h>
#include <unistd.h>

QString ProcessLsof::joinProcessNames( const QList<ProcessLsof::Process>& apps )
{
    QStringList l;
    for( QList<ProcessLsof::Process>::const_iterator it = apps.begin();
         it != apps.end(); ++it )
        l.append( (*it).name );
    return l.join( ", " );
}

static ProcessLsof::Process createProcess( const QString& name, int pid )
{
    ProcessLsof::Process p;
    p.name = name;
    p.pid = pid;
    return p;
}

class ProcessLsof::Private
{
public:
    QList<Process> apps;
    QString lsofBin;
};

ProcessLsof::ProcessLsof(QObject *parent)
{
    d = new Private;
}

ProcessLsof::~ProcessLsof()
{
    delete d;
}

bool ProcessLsof::checkDevice(DataCDROM *cdrom)
{
    d->apps.clear();

    if( !findLsofExecutable() )
        return false;

    // run lsof
    QProcess p;
    QStringList arg;

    p.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    arg << "-Fpc" << cdrom->device();
    p.start(d->lsofBin, arg);

    if( !p.waitForFinished( -1 ) )
        return false;

    // now process its output
    QByteArray output = p.readAllStandardOutput();
    const QStringList l = QString::fromLocal8Bit( output ).split( '\n', QString::SkipEmptyParts );
    for (int i = 0; i < l.size(); ++i) {
        LOGGER->info(QString("(ProcessLsof::checkDevice) lsof [%1] %2").arg(i).arg(l[i]));
    }

    QStringList::ConstIterator it = l.constBegin();
    while ( it != l.constEnd() ) {
        LOGGER->info(QString(""));
        QChar identifier = (it->mid(0,1).data())[0];
        int pid = 0;
        QString app;

        // p:指pid， c:指进程名， f:指文件描述符
        if(identifier =='p') {
            pid = it->mid(1).toInt();
            if ( ++it != l.constEnd() ) {
                identifier = (it->mid(0,1).data())[0];
                if(identifier == 'c') {
                    app = it->mid( 1 );
                    LOGGER->info(QString("(ProcessLsof %1) identifier: [%2] app:[%3] pid = [%4]").arg(__LINE__).arg(identifier).arg(app).arg(pid));

                    // we don't care about ourselves using the device ;)
                    if( pid != (int)::getpid() )
                        d->apps.append( createProcess( app, pid ) );
                } else if(identifier == 'f') {

                }
            }
        }
        ++it;
    }

    for(int i = 0; i < d->apps.size(); i++) {
        LOGGER->info(QString("((ProcessLsof::checkDevice)) d->app[%1] app:[%2] id:%3").arg(i).arg(d->apps[i].name).arg(d->apps[i].pid));
    }

    return true;
}

const QList<ProcessLsof::Process> &ProcessLsof::usingApplications() const
{
    return d->apps;
}

bool ProcessLsof::findLsofExecutable()
{
    if( d->lsofBin.isEmpty() ) {
        d->lsofBin = QStandardPaths::findExecutable( "lsof" );
    }

    return !d->lsofBin.isEmpty();
}
