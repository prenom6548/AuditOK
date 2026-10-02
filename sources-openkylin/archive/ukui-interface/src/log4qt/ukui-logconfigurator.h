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

#ifndef __UKUI_LOGCONFIGURATOR_H__
#define __UKUI_LOGCONFIGURATOR_H__

#include "ukui-logmacros.h"

#include <QObject>
#include <QMutex>
#include <qgsettings.h>
#include "log4qt/helpers/properties.h"
#include "ukui-logrolling.h"
#include <QTimer>

/**
 * @brief UKUI日志配置管理器类
 *
 * 该类是日志SDK的对外接口类，负责管理日志系统的配置和初始化。
 * 采用单例模式，提供全局统一的日志配置管理功能。
 *
 * 主要功能：
 * - 初始化Log4Qt日志系统
 * - 管理全局和应用程序特定的GSettings配置
 * - 监听配置变化并动态更新日志设置
 * - 管理日志文件清理和滚动策略
 * - 提供定时检查机制防止日志文件过多或过大
 */
class LIBUKUILOG4QT_EXPORT UkuiLog4qtConfig : public QObject
{
    Q_OBJECT

private:
    /**
     * @brief 析构函数
     * 清理资源，停止定时器和线程
     */
    virtual ~UkuiLog4qtConfig();

    /**
     * @brief 私有构造函数
     * @param parent 父对象指针
     * 实现单例模式，防止外部直接实例化
     */
    UkuiLog4qtConfig(QObject *parent = nullptr);

public:
    /**
     * @brief 获取日志配置器单例实例
     * @return 返回全局唯一的配置器实例引用
     */
    static UkuiLog4qtConfig &instance();

    /**
     * @brief 初始化日志SDK
     * @param strAppName 应用程序名称，用于生成特定的日志文件路径和配置
     * @return 0表示成功，-1表示路径错误，-2表示配置失败
     */
    int init(QString strAppName);

    /**
     * @brief 启动日志文件清理检查定时器
     */
    void startCleanUpCheckTimer();

    /**
     * @brief 停止日志文件清理检查定时器
     *
     * 停止定时器并清理相关资源，包括停止清理线程。
     */
    void stopCleanUpCheckTimer();

private Q_SLOTS:
    /**
     * @brief 定时器超时槽函数
     *
     * 当清理检查定时器超时时调用，更新文件检查限制条件。
     */
    void onCleanUpCheckTimer();

    void onLogOutput();

private:
    /**
     * @brief 初始化GSettings配置并设置监听
     * @param strAppName 应用程序名称
     * @param properties Log4Qt属性对象引用
     */
    void initSettings(QString strAppName, Log4Qt::Properties &properties);

    /**
     * @brief 获取日志文件列表
     * @return 返回当前日志目录下的所有日志文件信息列表
     */
    QFileInfoList getLogFiles() const;

    /**
     * @brief 检查日志目录是否有效
     * @param mainDirPath 目录主路径
     * @param dirName 目录名称
     * @return 如果主目录存在且可读写，且子目录不存在或者存在且可读写返回true，否则返回false
     */
    bool checkLogDirectoryValid(const QString &mainDirPath, const QString &dirName) const;

public:
    /**
     * @brief 日志文件清理检查线程对象
     */
    UkuiLog4qtRolling *m_threadCheckFile;

private:
    /**
     * @brief 初始化状态标志
     */
    bool m_isInited;

    /**
     * @brief 全局GSettings配置对象
     */
    QGSettings *m_gsLog4qtGlobal;

    /**
     * @brief 应用程序特定GSettings配置对象
     */
    QGSettings *m_gsLog4qtSpecial;

    /**
     * @brief Log4Qt属性配置对象
     */
    Log4Qt::Properties m_log4qtProperties;

    /**
     * @brief 最大允许的日志文件数量
     */
    unsigned m_uMaxFileCount;

    /**
     * @brief 最大允许的日志文件总大小（字节）
     */
    quint64 m_uMaxFileSize;

    /**
     * @brief 文件检查时间间隔（秒）
     */
    quint64 m_timeCheckFile;

    /**
     * @brief 清理检查定时器对象
     */
    QTimer *m_cleanUpCheckTimer = nullptr;

    /**
     * @brief 当前基础日志文件路径
     */
    QString m_strBaseFilePath;

    bool m_isConnectToLogger = false;
};

#endif // __UKUI_LOGCONFIGURATOR_H__
