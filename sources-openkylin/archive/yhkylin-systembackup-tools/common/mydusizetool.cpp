/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include "mydusizetool.h"
#include <QDebug>
#include "utils.h"

MyDuSizeTool::MyDuSizeTool(QObject* parent) :
    QObject(parent),
    m_p(new QProcess(this))
{
    connect(m_p, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(m_p, &QProcess::readyReadStandardError, this, [&]() {
        QByteArray err = m_p->readAllStandardError();
        qCritical("du process error: %s", err.data());
    });
}

MyDuSizeTool::~MyDuSizeTool()
{
    delete m_p;
}

/**
 * @brief MyDuSizeTool::Do
 * @param paths, du计算路径列表
 * @param excludePaths，du排查路径列表
 * @param block，bool, 是否阻塞方式计算
 * @return
 */
qint64 MyDuSizeTool::Do(QStringList paths, QStringList excludePaths, bool block)
{
    return _Do(paths, excludePaths, true, block);
}

/**
 * @brief MyDuSizeTool::_Do
 * @param paths, du计算路径列表
 * @param excludePaths，du排查路径列表
 * @param needExclude，bool，是否需要排除
 * @param block，bool, 是否阻塞方式计算
 * @return
 */
qint64 MyDuSizeTool::_Do(QStringList paths, QStringList excludePaths, bool needExclude, bool block)
{
    Init(paths, excludePaths);

    if (paths.isEmpty()) {
        qCritical("du paths is empty!");
        return -1;
    }
    if (needExclude && excludePaths.isEmpty()) {
        qCritical("du excludepaths is empty!");
        return -1;
    }

    QString cmd = "du";
    for (const QString& x : paths) {
        cmd.append(QString(" \"%1\"").arg(x));
    }
    if (needExclude) {
        for (const QString& item : excludePaths) {
            QString arg = QString(" --exclude=\"%1\"").arg(item);
            cmd.append(arg);
        }
    }

    cmd.append(" -sb | tail -1 | awk -F \" \" '{print $1}'");

    qDebug("cmd is %s", cmd.toStdString().c_str());

    QStringList Args;
    Args << "-c" << cmd;
    m_p->start("bash", Args);

    if (!m_p->waitForStarted())
        return -1;

    if (block) {
        if (!m_p->waitForFinished(-1)) {
            return -1;
        }

        return m_size;
    }

    return 0;
}

/**
 * @brief MyDuSizeTool::Do
 * @param paths, du计算路径列表
 * @param excludePaths，du排查路径列表
 * @param block，bool, 是否阻塞方式计算
 * @return
 */
qint64 MyDuSizeTool::Do(QStringList paths, bool block)
{
    return _Do(paths, {}, false, block);
}

void MyDuSizeTool::Init(QStringList paths, QStringList excludePaths)
{
    m_paths = paths;
    m_excludePaths = excludePaths;
}

void MyDuSizeTool::finished(int, QProcess::ExitStatus)
{
    QString result = m_p->readAll();
    if (result.isEmpty())
        m_size = 0;
    else
        m_size = result.toLongLong();
    emit duFinished(m_size);
}

