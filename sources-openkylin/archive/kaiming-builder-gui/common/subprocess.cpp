#include "subprocess.h"

#include <QDebug>

SubProcess::SubProcess(QObject *parent) :
    QObject(parent),
    m_p(new QProcess(this))
{
    connect(m_p, &QProcess::readyRead, this, [&]() {
        QString str = QString(m_p->readAll());
        if (m_block) {
            m_output += str;
        }
        emit output(str);
    });

    connect(m_p, &QProcess::readyReadStandardError, this, [&]() {
        QString err = QString(m_p->readAllStandardError());
        emit output(err);
    });

    connect(m_p, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_p->kill();
    });

    connect(m_p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
}

SubProcess::~SubProcess()
{
    if (!m_p && m_p->state() != QProcess::NotRunning) {
        m_p->kill();
    }
}

bool SubProcess::start(const QString& exe, const QStringList& args, bool block)
{
    m_block = block;
    QString cmd(exe);
    for (const QString& item : args) {
       cmd += " ";
       cmd += item;
    }
    qDebug() << cmd;

    m_p->start(exe, args);
    if (!m_p->waitForStarted()) {
        return false;
    }

    if (block) {
        if (!m_p->waitForFinished()) {
            return false;
        }
    }

    return true;
}

void SubProcess::processFinished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == QProcess::NormalExit || exitCode == 24 || exitCode == 23) {
        emit finished(true);
    } else {
        emit finished(false);
    }
}

