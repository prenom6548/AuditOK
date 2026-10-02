/*
 * Copyright (C) 2021 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "ukui-logrolling.h"
#include "ukui-log4qt.h"

#include <QStringList>
#include <QProcess>
#include <QMutexLocker>
#include <QTime>

/**
 * @brief 构造函数
 * @param strBaseFilePath 基础日志文件路径
 * @param parent 父对象指针
 */
UkuiLog4qtRolling::UkuiLog4qtRolling(QString strBaseFilePath, QObject *parent)
    : QThread(parent), m_isExit(0), m_strBaseFilePath(strBaseFilePath)
{
}

/**
 * @brief 析构函数
 */
UkuiLog4qtRolling::~UkuiLog4qtRolling() {}

/**
 * @brief 线程运行函数
 */
void UkuiLog4qtRolling::run()
{
    // 检查退出标志，如果已设置则直接返回
    if (m_isExit.loadAcquire()) {
        return;
    }

    // 主循环：等待任务并执行清理操作
    while (!m_isExit.loadAcquire()) {
        QMutexLocker locker(&m_mutex);

        // 等待条件变量唤醒（有新的清理任务）
        m_condition.wait(&m_mutex);

        // 再次检查退出标志，如果已设置则退出循环
        if (m_isExit.loadAcquire()) {
            break;
        }

        // 执行文件数量检查（如果设置了限制）
        if (m_uMaxFileCount > 0) {
            checkLogFilesCount(m_listFileInfo);
        }

        // 执行文件大小检查（如果设置了限制）
        if (m_uMaxFileSize > 0) {
            checkLogFilesSize(m_listFileInfo);
        }
    }
}

/**
 * @brief 设置文件检查限制条件
 * @param fileInfoList 要检查的文件信息列表
 * @param uMaxFileCount 最大允许的文件数量（0表示不限制）
 * @param uMaxFileSize 最大允许的文件总大小（0表示不限制）
 */
void UkuiLog4qtRolling::setFileCheckLimit(
    const QFileInfoList &fileInfoList, unsigned uMaxFileCount, quint64 uMaxFileSize)
{
    QMutexLocker locker(&m_mutex);

    // 更新文件信息列表和限制条件
    m_listFileInfo = fileInfoList;
    m_uMaxFileCount = uMaxFileCount;
    m_uMaxFileSize = uMaxFileSize;

    // 唤醒等待的线程开始执行清理任务
    m_condition.wakeOne();
}

/**
 * @brief 停止线程
 */
void UkuiLog4qtRolling::stop()
{
    // 设置退出标志
    m_isExit = 1;

    // 获取互斥锁并唤醒等待的线程
    QMutexLocker locker(&m_mutex);
    m_condition.wakeOne();
}

/**
 * @brief 检查文件数量
 * @param fileInfoList 要检查的文件信息列表
 */
void UkuiLog4qtRolling::checkLogFilesCount(const QFileInfoList &fileInfoList)
{
    // 检查退出标志，如果已设置则直接返回
    if (m_isExit.loadAcquire()) {
        return;
    }

    // 检查文件数量是否超过限制
    if (fileInfoList.size() <= (int)(m_uMaxFileCount)) {
        return;
    }

    // 计算需要删除的文件数量
    int filesToRemove = fileInfoList.size() - m_uMaxFileCount;
    QTime startTime = QTime::currentTime();

    // 从文件列表末尾开始删除（最旧的文件在前）
    for (int i = fileInfoList.size() - 1; i >= 0 && filesToRemove > 0; i--) {
        // 检查超时或退出标志，避免长时间阻塞
        if (startTime.msecsTo(QTime::currentTime()) > 500 || m_isExit.loadAcquire()) {
            break;
        }

        QString filePath = fileInfoList[i].absoluteFilePath();

        // 跳过当前正在使用的基础文件，避免删除正在写入的文件
        if (filePath == m_strBaseFilePath) {
            continue;
        }

        // 尝试删除文件
        QFile file(filePath);
        if (file.remove()) {
            KyDebug() << QString("Removed old log file: %1").arg(filePath);
            filesToRemove--;
        } else {
            KyWarning() << "Failed to remove old log file:" << filePath;
        }
    }
}

/**
 * @brief 检查文件大小
 * @param fileInfoList 要检查的文件信息列表
 */
void UkuiLog4qtRolling::checkLogFilesSize(const QFileInfoList &fileInfoList)
{
    // 检查退出标志，如果已设置则直接返回
    if (m_isExit.loadAcquire()) {
        return;
    }

    // 计算所有文件的总大小
    qint64 totalSize = 0;
    QTime startTime = QTime::currentTime();

    // 遍历所有文件，累加文件大小
    for (const QFileInfo &fileInfo : fileInfoList) {
        // 设置200ms超时，避免大小计算耗时过长
        if (startTime.msecsTo(QTime::currentTime()) > 200) {
            qWarning("Size calculation timeout, using estimated size");
            break;
        }
        totalSize += fileInfo.size();
    }

    // 检查总大小是否超过限制
    if (totalSize <= (qint64)(m_uMaxFileSize)) {
        return;
    }

    // 计算需要清理的大小
    qint64 sizeToRemove = totalSize - m_uMaxFileSize;
    QString currentFilePath = m_strBaseFilePath;
    startTime = QTime::currentTime();

    // 从文件列表末尾开始清理（最旧的文件在前）
    for (int i = fileInfoList.size() - 1; i >= 0 && sizeToRemove > 0; i--) {
        // 设置1秒超时，避免清理过程耗时过长
        if (startTime.msecsTo(QTime::currentTime()) > 1000 || m_isExit.loadAcquire()) {
            qWarning() << "Size cleanup timeout, stopping to avoid blocking";
            break;
        }

        QString filePath = fileInfoList[i].absoluteFilePath();
        qint64 fileSize = fileInfoList[i].size();

        // 处理当前正在使用的文件
        if (filePath == currentFilePath) {
            // 对于当前文件，清空内容而不是删除，避免影响正在进行的日志记录
            QProcess process;
            process.start("sh", QStringList() << "-c" << QString("> %1").arg(filePath));
            process.waitForFinished(100);
            sizeToRemove -= fileSize;
        } else {
            // 对于历史文件，直接删除
            QFile file(filePath);
            if (file.remove()) {
                KyDebug() << QString("Removed old log file: %1 (size: %2 bytes)").arg(filePath).arg(fileSize);
                sizeToRemove -= fileSize;
            } else {
                qWarning() << QString("Failed to remove old log file: %1").arg(filePath);
            }
        }
    }
}
