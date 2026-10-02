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

#ifndef __UKUILOG4QT_ROLLING_H__
#define __UKUILOG4QT_ROLLING_H__

#include "ukui-logmacros.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include <QFileInfoList>

/**
 * @brief UKUI日志文件滚动清理线程类
 */
class UkuiLog4qtRolling : public QThread
{
public:
    /**
     * @brief 构造函数
     * @param strBaseFilePath 基础日志文件路径
     * @param parent 父对象指针
     */
    UkuiLog4qtRolling(QString strBaseFilePath, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~UkuiLog4qtRolling();

    /**
     * @brief 停止线程
     */
    void stop();

    /**
     * @brief 设置文件检查限制条件
     * @param fileInfoList 要检查的文件信息列表
     * @param uMaxFileCount 最大允许的文件数量（0表示不限制）
     * @param uMaxFileSize 最大允许的文件总大小（0表示不限制）
     */
    void setFileCheckLimit(const QFileInfoList &fileInfoList, unsigned uMaxFileCount = 0, quint64 uMaxFileSize = 0);

    /**
     * @brief 检查文件数量
     * @param fileInfoList 要检查的文件信息列表
     */
    void checkLogFilesCount(const QFileInfoList &fileInfoList);

    /**
     * @brief 检查文件大小
     * @param fileInfoList 要检查的文件信息列表
     */
    void checkLogFilesSize(const QFileInfoList &fileInfoList);

public Q_SLOTS:
    /**
     * @brief 滚动处理槽函数
     */
    void onRollingProc();

protected:
    /**
     * @brief 线程运行函数
     */
    void run();

private:
    /**
     * @brief 线程退出标志
     */
    QAtomicInt m_isExit = 1;

    /**
     * @brief 当前基础文件路径
     */
    QString m_strBaseFilePath;

    /**
     * @brief 最大文件数限制
     *
     * 允许保留的最大日志文件数量。
     * 0表示不限制文件数量。
     */
    unsigned m_uMaxFileCount = 0;

    /**
     * @brief 最大文件大小限制
     *
     * 允许保留的最大日志文件总大小（字节）。
     * 0表示不限制文件大小。
     */
    quint64 m_uMaxFileSize = 0;

    /**
     * @brief 文件信息列表
     */
    QFileInfoList m_listFileInfo;

    /**
     * @brief 互斥锁
     */
    QMutex m_mutex;

    /**
     * @brief 条件变量
     */
    QWaitCondition m_condition;
};

#endif // __UKUILOG4QT_ROLLING_H__
