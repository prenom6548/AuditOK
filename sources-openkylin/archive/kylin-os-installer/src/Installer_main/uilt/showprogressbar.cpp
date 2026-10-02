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

#include "showprogressbar.h"

//#include "../PluginService/kcommand.h"
#include <QSettings>
#include <QDir>
#include <QThread>
#include <QDebug>
#include <sys/statvfs.h>
#include <QProcess>

namespace KInstaller {
ShowProgressBar::ShowProgressBar(QObject *parent) : QObject(parent),
    m_value(0)
{
    m_nTotal = 0;
    m_nCopyed = 0;
    if(!TotalInodesInDir("/rofs/sysroot", m_nTotal))
    {
        qDebug()<<"get total inodes number in /rofs error!";
    }

    timer = new QTimer(this);
    timer->setInterval(500);

    connect(timer, &QTimer::timeout, this, &ShowProgressBar::flushProgressBar);
    start();
}

void ShowProgressBar::ReadProgress()
{

    QString path = QString("");
    path = "/var/log/installer/installer-progress.log";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    m_nReadStart=setting.value("installer-progress").toInt();
    m_nReadEnd =setting.value("installer-progress-next").toInt();

}

void ShowProgressBar::flushProgressBar()
{
    qDebug()<<"m_nStart="<<m_nStart;
    qDebug()<<"m_nEnd="<<m_nEnd;
    
    ReadProgress();
    if(m_nStart <= m_nReadStart)
    {
	m_nStart=m_nReadStart;
    	qDebug()<<"m_nReadStart="<<m_nReadStart;
    }
    if(m_nEnd <= m_nReadEnd)
    {
	m_nEnd=m_nReadEnd;
    	qDebug()<<"m_nReadEnd="<<m_nReadEnd;
    }

    m_Cnt++;
    if(m_Cnt > m_SkipCnt)
    {
	m_Cnt=1;
	m_nStart++;
    }

    if(m_nStart >= m_nEnd)
    {
	m_nStart=m_nEnd;
    }

    if(m_nStart >= 100)
    {
    	timer->stop();
    }
    emit signelProgressValue(m_nStart);
}

void ShowProgressBar::start()
{
    timer->start();
}

bool ShowProgressBar::TotalInodesInDir(const QString strPathOfDir, int & nTotal)
{
    nTotal = 0;
    struct statvfs buf;
    unsigned long int  nTotalInodes = 0;
    if (statvfs(strPathOfDir.toLocal8Bit().data() , &buf) == 0)
    {
        nTotalInodes = buf.f_files - buf.f_ffree;
        nTotal = nTotalInodes;
    }
    else
    {
        qDebug()<<"Get total inodes in dir["<<strPathOfDir<<"] errno:target["<<errno<<"]"<<strerror(errno);
    }

    if(0 == nTotalInodes)//换一种方法，再次尝试
    {
        QProcess serverProc(this);
        QString strCommand = QString("du -s --inodes %1 2>/dev/null").arg(strPathOfDir);
	qDebug()<<strCommand;
        serverProc.start(strCommand);//非阻塞调用

        bool bMakeQRImg = false;
        if(serverProc.waitForStarted())//阻塞（默认最多30秒）等待被调用程序开始运行
        {//启动成功
            if(serverProc.waitForFinished(10000))//本线程最多等待10秒
            {
                bMakeQRImg = true;
            }
        }

        if(!bMakeQRImg)
        {//异常结束（运行超时或运行失败）
            qDebug()<<"QProcess state[du -s --inodes]:"<<serverProc.state();//判断新进程状态
            qDebug()<<"QProcess qError[du -s --inodes]:"<<serverProc.error();//查看最后一个错误
            qDebug()<<"QProcess exitStatus[du -s --inodes]:"<<serverProc.exitStatus();//程序退出状态
            qDebug()<<"QProcess readAllStandardError[du -s --inodes]:"<<QString::fromLocal8Bit(serverProc.readAllStandardError());//被调用程序向标准错误打印出的信息。多次打印出的信息不会重复
            qDebug()<<"QProcess readAllStandardOutput[du -s --inodes]:"<<QString::fromLocal8Bit(serverProc.readAllStandardOutput());//被调用程序向标准输出打印出的信息。多次打印出的信息不会重复

            nTotal = 0;
            return false;
        }
        else
        {
            QString strOut = QString::fromLocal8Bit(serverProc.readAllStandardOutput());
            qDebug()<<strOut;
            if(!strOut.isEmpty())
            {
                QString strT;
                int i=0;
                while(strOut.at(i).isDigit())
                {
                    strT += strOut.at(i);
                    i++;
                }
                nTotal = strT.toInt();
               // qDebug()<<"nTotal:"<<nTotal;
            }
        }
    }
    return true;
}

}
