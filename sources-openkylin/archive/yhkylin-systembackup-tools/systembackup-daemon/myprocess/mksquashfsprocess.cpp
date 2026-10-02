/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "mksquashfsprocess.h"
#include <QDebug>

MkSquashFSProcess::MkSquashFSProcess(QObject* parent)
    : QObject(parent)
    , m_p(new QProcess(this))
    , m_syncProcess(new QProcess(this))
{
    connect(m_p, &QProcess::readyRead, this, [&]() {
        QString str = QString(m_p->readAll());
        qDebug() << str;
        if (str.contains("]") && str.contains("%")) {
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
        qCritical() << err;
    });

    connect(m_p, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_p->kill();
    });

    connect(m_p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(mksquashfs_finished(int, QProcess::ExitStatus)));

    connect(m_syncProcess, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_syncProcess->kill();
    });
    connect(m_syncProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(sync_finished(int, QProcess::ExitStatus)));
}

MkSquashFSProcess::~MkSquashFSProcess()
{
    if (!m_p && m_p->state() != QProcess::NotRunning) {
        m_p->kill();
    }

    if (!m_syncProcess && m_syncProcess->state() != QProcess::NotRunning) {
        m_syncProcess->kill();
    }
}

bool MkSquashFSProcess::start(const QStringList &args)
{
    /*
    QString cmd("mksquashfs ");
    for (const QString& item : args) {
       cmd += " ";
       cmd += item;
    }*/
    qDebug() << "para list "<< args;

    m_p->start("mksquashfs", args);
    if (!m_p->waitForStarted(-1)) {
        qCritical("mksquashfs start failed!");
        return false;
    }

    return true;
}

void MkSquashFSProcess::mksquashfs_finished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == QProcess::NormalExit) {
        m_syncProcess->start("sync");
        if (!m_syncProcess->waitForStarted()) {
            emit finished(false);
        }
    } else {
        emit finished(false);
    }
}

void MkSquashFSProcess::sync_finished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == QProcess::NormalExit) {
        emit progress(100);
        emit finished(true);
    } else {
        emit finished(false);
    }
}

