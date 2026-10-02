/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "rsyncpathtodirprocess.h"
#include <QDebug>

RsyncPathToDirProcess::RsyncPathToDirProcess(QObject *parent) :
    QObject(parent),
    m_p(new QProcess(this)),
    m_syncProcess(new QProcess(this))
{
    m_currentRate = 0;
    m_bSuccess = false;

    connect(m_p, &QProcess::readyRead, this, [&]() {
        QString str = QString(m_p->readAll());
        qDebug() << str;
        // if (str.contains("B\/s") && str.contains("%")) {
        if (str.contains("B/s") && str.contains("%")) {
            if (str.split("%").at(0).length() < 3)
                return;
            int tmpRate = str.split("%").at(0).right(3).toInt();
            if (m_currentRate == tmpRate)
                return;
            m_currentRate = tmpRate;
            emit progress(m_currentRate);
        }
    });

    connect(m_p, &QProcess::readyReadStandardError, this, [&]() {
        QByteArray err = m_p->readAllStandardError();
        qCritical("backup process error: %s", err.data());
    });

    connect(m_p, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_p->kill();
    });

    connect(m_p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(rsync_finished(int, QProcess::ExitStatus)));

    connect(m_syncProcess, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_syncProcess->kill();
    });
    connect(m_syncProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(sync_finished(int, QProcess::ExitStatus)));
}

RsyncPathToDirProcess::~RsyncPathToDirProcess()
{
    if (!m_p && m_p->state() != QProcess::NotRunning) {
        m_p->kill();
    }

    if (!m_syncProcess && m_syncProcess->state() != QProcess::NotRunning) {
        m_syncProcess->kill();
    }
}

bool RsyncPathToDirProcess::start(QStringList args, bool block)
{
    qDebug() << "RsyncPathToDirProcess::start invoke begin";
    m_currentRate = 0;
    m_bSuccess = false;
    m_block = block;

    QString cmd("rsync");
    for (const QString& item : args) {
       cmd += " ";
       cmd += item;
    }
    qDebug() << cmd;

    m_p->start("rsync", args);
    if (!m_p->waitForStarted()) {
        qCritical("rsync started failed");
        return false;
    }

    if (m_block) {
        if (!m_p->waitForFinished()) {
            qCritical("rsync finished failed");
            return false;
        }
    }

    qDebug() << "RsyncPathToDirProcess::start invoke end";
    return m_bSuccess;
}

void RsyncPathToDirProcess::rsync_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug() << "RsyncPathToDirProcess::rsync_finished invoke begin";
    if (exitCode == QProcess::NormalExit || exitCode == 24 || exitCode == 23) {
        if (m_block) {
            qDebug() << "RsyncPathToDirProcess::rsync_finished block mode";
            m_bSuccess = true;
            emit progress(100);
            emit finished(true);
        } else {
            qDebug() << "RsyncPathToDirProcess::rsync_finished unblock mode";
            emit progress(100);
            m_syncProcess->start("sync");
            if (!m_syncProcess->waitForStarted()) {
                qDebug() << "sync start failed";
                m_bSuccess = false;
                emit finished(false);
            }
        }
    } else {
        qDebug() << "RsyncPathToDirProcess::rsync_finished failed";
        m_bSuccess = false;
        emit finished(false);
    }
    qDebug() << "RsyncPathToDirProcess::rsync_finished invoke end";
}

void RsyncPathToDirProcess::sync_finished(int exitCode, QProcess::ExitStatus)
{
    qDebug() << "RsyncPathToDirProcess::sync_finished invoke begin";

    if (exitCode == QProcess::NormalExit || exitCode == 24 || exitCode == 23) {
        qDebug() << "RsyncPathToDirProcess::sync_finished success";
        m_bSuccess = true;
        emit finished(true);
    } else {
        qDebug() << "RsyncPathToDirProcess::sync_finished failed";
        m_bSuccess = false;
        emit finished(false);
    }

    qDebug() << "RsyncPathToDirProcess::sync_finished invoke end";
}
