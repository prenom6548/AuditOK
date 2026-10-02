/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "calcbackupsize.h"
#include <QDebug>
#include <QRegularExpression>

CalcBackupSize::CalcBackupSize(QObject* parent) :
    QObject(parent),
    m_process(new QProcess(this))
{
    connect(m_process, &QProcess::readyRead, this, [&]() {
        parseResult();
    });

    connect(m_process, &QProcess::readyReadStandardError, this, [&]() {
        QByteArray err = m_process->readAllStandardError();
        qCritical("backup process error: %s", err.data());
    });

    connect(m_process, &QProcess::errorOccurred, this, [&](QProcess::ProcessError error) {
        Q_UNUSED(error)
        m_process->kill();
    });

    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinish(int, QProcess::ExitStatus)));
}

CalcBackupSize::~CalcBackupSize()
{
    if (!m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
    }
}

/**
 * @brief 计算备份大小，单位字节
 * @param args rsync参数列表
 * @param block 是否阻塞计算模式。默认true——阻塞计算模式
 * @return block为true时返回值为待备份数据大小，单位字节；block为false时，默认返回0;
 */
qint64 CalcBackupSize::start(QStringList args, bool block)
{
    qDebug() << "CalcBackupSize::calcBackupSize invoke begin";

    QString cmd("rsync ");
    for (const QString& item : args) {
       cmd += " ";
       cmd += item;
    }
    qDebug() << cmd;

    m_size = 0;

    m_process->start("rsync", args);
    if (!m_process->waitForStarted()) {
        qCritical("rsync start failed");
        return m_size;
    }

    if (block) {
        m_process->waitForFinished();
    }

    qDebug() << "CalcBackupSize::calcBackupSize invoke end";
    return m_size;
}

/**
 * @brief 解析process的输出结果
 */
void CalcBackupSize::parseResult()
{
    // 解析结果，结果内容样例如下：
    /*
    Number of files: 256,274 (reg: 207,101, dir: 23,764, link: 25,407, special: 2)
    Number of created files: 256,274 (reg: 207,101, dir: 23,764, link: 25,407, special: 2)
    Number of deleted files: 0
    Number of regular files transferred: 207,101
    Total file size: 12,160,363,663 bytes
    Total transferred file size: 12,159,780,794 bytes
    Literal data: 0 bytes
    Matched data: 0 bytes
    File list size: 786,254
    File list generation time: 0.001 seconds
    File list transfer time: 0.000 seconds
    Total bytes sent: 8,273,515
    Total bytes received: 794,093

    sent 8,273,515 bytes  received 794,093 bytes  1,209,014.40 bytes/sec
    total size is 12,160,363,663  speedup is 1,341.08 (DRY RUN)

    */
    QString out(m_process->readAll());
    QStringList lines = out.split("\n");
    qDebug() << out;
    for (QString& line : lines) {
        // 获取文件夹数目
        if (line.startsWith("Number of files:")) {
            int indexOfDir = line.indexOf("dir: ");
            indexOfDir += 5;
            int indexOfDirEnd = line.indexOf(QRegularExpression("[ )]"), indexOfDir);
            int numOfDirs = line.mid(indexOfDir, indexOfDirEnd-indexOfDir).replace(",","").trimmed().toInt();
            // 每个目录下还都有.和..，故总数还要*3
            numOfDirs *= 3;
            // ext4格式的目录本身要占用4K空间（4096）
            m_size += numOfDirs * 4096;
        }
        if (line.startsWith("Total transferred file size: ")) {
            m_size += line.replace("Total transferred file size: ", "").replace("bytes", "").replace(",","").trimmed().toLongLong();
        }
    }
}

/**
 * @brief process结束
 * @param exitCode
 */
void CalcBackupSize::processFinish(int exitCode, QProcess::ExitStatus)
{
    qDebug() << "CalcBackupSize::getCalcResult invoke begin";
    Q_UNUSED(exitCode)

    if (exitCode == QProcess::NormalExit)
        parseResult();

    emit finished(m_size);

    qDebug() << "CalcBackupSize::getCalcResult invoke end";
}
