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

#include "ukui-logconfigurator.h"
#include "log4qt/logger.h"
#include "log4qt/basicconfigurator.h"
#include "log4qt/level.h"
#include "log4qt/log4qt.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"
#include "log4qt/helpers/initialisationhelper.h"
#include "log4qt/logmanager.h"

#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QTime>
#include <QFile>
#include <QFileInfo>
#include <unistd.h>

// 全局互斥锁，用于保护单例实例的创建
LOG4QT_GLOBAL_STATIC(QMutex, single_config)
// 主进程ID，用于标识当前进程
pid_t g_MainProcPid = -1;

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
UkuiLog4qtConfig::UkuiLog4qtConfig(QObject *parent)
    : QObject(parent)
    , m_threadCheckFile(nullptr)
    , m_isInited(false)
    , m_gsLog4qtGlobal(nullptr)
    , m_gsLog4qtSpecial(nullptr)
{
    // 清空Log4Qt属性配置
    m_log4qtProperties.clear();
    // 设置默认的文件数量限制（0表示不限制）
    m_uMaxFileCount = 0;
    // 设置默认的文件大小限制（0表示不限制）
    m_uMaxFileSize = 0;
    // 设置默认的文件检查间隔为60秒
    m_timeCheckFile = 60;
    // 记录当前进程ID
    g_MainProcPid = getpid();
}

/**
 * @brief 析构函数
 */
UkuiLog4qtConfig::~UkuiLog4qtConfig()
{
    // 停止清理检查定时器和相关线程
    stopCleanUpCheckTimer();

    // 清理全局GSettings配置对象
    if (m_gsLog4qtGlobal) {
        delete m_gsLog4qtGlobal;
        m_gsLog4qtGlobal = nullptr;
    }

    // 清理应用程序特定GSettings配置对象
    if (m_gsLog4qtSpecial) {
        delete m_gsLog4qtSpecial;
        m_gsLog4qtSpecial = nullptr;
    }
}

/**
 * @brief 获取单例实例
 * @return 返回全局唯一的配置器实例引用
 */
UkuiLog4qtConfig &UkuiLog4qtConfig::instance()
{
    QMutexLocker locker(single_config());
    static UkuiLog4qtConfig mInstance;
    return mInstance;
}

/**
 * @brief 初始化日志SDK
 * @param strAppName 应用程序名称，用于生成特定的日志文件路径和配置
 * @return 0表示成功，-1表示路径错误，-2表示配置失败
 */
int UkuiLog4qtConfig::init(QString strAppName)
{
    QMutexLocker locker(single_config());

    // 防止重复初始化
    if (m_isInited) {
        return true;
    }

    // 清空现有属性配置
    m_log4qtProperties.clear();

    // 设置是否重置所有配置，恢复全局设置默认值
    m_log4qtProperties.setProperty(UKUILOG4QT_RESET, "true");

    // 设置日志SDK内部记录器级别为INFO
    m_log4qtProperties.setProperty(UKUILOG4QT_DEBUG, "INFO");

    // 设置日志记录器仓库的阈值为NULL（不限制）
    const QLatin1String key_threshold("log4j.threshold");
    m_log4qtProperties.setProperty(key_threshold, "NULL");

    // 设置是否监听QDebug输出的字符串
    m_log4qtProperties.setProperty(UKUILOG4QT_HANDLEQT, "true");

    // 设置根Logger的输出日志等级和输出目的地（DEBUG级别，输出到daily文件）
    m_log4qtProperties.setProperty(UKUILOG4QT_ROOTLOGGER, "DEBUG,daily");

    // 设置控制台输出器类型
    const QLatin1String key_appender_console("log4j.appender.console");
    m_log4qtProperties.setProperty(key_appender_console, "org.apache.log4j.ConsoleAppender");

    // 设置控制台输出目标为标准输出
    const QLatin1String key_appender_console_target("log4j.appender.console.target");
    m_log4qtProperties.setProperty(key_appender_console_target, "STDOUT_TARGET");

    // 设置控制台输出布局为TTCCLayout（时间、线程、类别、消息）
    const QLatin1String key_appender_console_layout("log4j.appender.console.layout");
    m_log4qtProperties.setProperty(key_appender_console_layout, "org.apache.log4j.TTCCLayout");

    // 设置控制台布局的日期格式
    const QLatin1String key_appender_console_layout_dateformat("log4j.appender.console.layout.dateFormat");
    m_log4qtProperties.setProperty(key_appender_console_layout_dateformat, "yyy-MM-dd hh:mm:ss.zzz");

    // 设置控制台布局不包含上下文信息
    const QLatin1String key_appender_console_layout_contextprinting("log4j.appender.console.layout.contextPrinting");
    m_log4qtProperties.setProperty(key_appender_console_layout_contextprinting, "false");

    // 设置控制台布局不包含当前类目信息
    const QLatin1String key_appender_console_layout_categoryprefixing(
        "log4j.appender.console.layout.categoryPrefixing");
    m_log4qtProperties.setProperty(key_appender_console_layout_categoryprefixing, "false");

    // 设置文件输出器类型为DailyRollingFileAppender（按天滚动）
    const QLatin1String key_appender_daily("log4j.appender.daily");
    m_log4qtProperties.setProperty(key_appender_daily, "org.apache.log4j.DailyRollingFileAppender");

    // 设置文件输出器的基础文件路径
    const QLatin1String key_appender_daily_file("log4j.appender.daily.file");
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homePath.size() <= 0) {
        m_log4qtProperties.setProperty(key_appender_daily_file, "");
    } else {
        // 检查目录权限
        QString logDirPath = homePath[0] + UKUILOG4QT_ROOTPATH;
        if (!checkLogDirectoryValid(homePath[0], UKUILOG4QT_ROOTPATH)) {
            m_log4qtProperties.setProperty(key_appender_daily_file, "");
        } else {
            // 构建日志文件路径：~/.log/应用程序名.log
            m_strBaseFilePath = logDirPath + strAppName + ".log";
            m_log4qtProperties.setProperty(key_appender_daily_file, m_strBaseFilePath);
        }
    }

    // 设置文件输出器以追加模式写入（不覆盖现有内容）
    const QLatin1String key_appender_daily_appendfile("log4j.appender.daily.appendFile");
    m_log4qtProperties.setProperty(key_appender_daily_appendfile, "true");

    // 设置文件输出器立即刷新到磁盘（确保日志及时写入）
    const QLatin1String key_appender_daily_immediateflush("log4j.appender.daily.immediateFlush");
    m_log4qtProperties.setProperty(key_appender_daily_immediateflush, "true");

    // 设置文件滚动日期模式（按天滚动，文件名后缀格式：.yyyy-MM-dd）
    m_log4qtProperties.setProperty(UKUILOG4QT_DAILY_DATEPATTERN, ".yyyy-MM-dd");

    // 设置文件输出器布局为PatternLayout（模式布局）
    const QLatin1String key_appender_daily_layout("log4j.appender.daily.layout");
    m_log4qtProperties.setProperty(key_appender_daily_layout, "org.apache.log4j.PatternLayout");

    // 设置文件输出器的日志格式模式
    // 格式：日期时间(相对时间)[线程]|级别| - 消息内容
    m_log4qtProperties.setProperty(
        UKUILOG4QT_DAILY_CONVERSIONPATTERN, "%d{yyyy-MM-dd HH:mm:ss,zzz}(%-4r)[%t]|%-5p| - %m%n");

    initSettings(strAppName, m_log4qtProperties);
    // 根据配置属性初始化Log4Qt的根Logger
    if (!Log4Qt::PropertyConfigurator::configure(m_log4qtProperties)) {
        return -2; // Log4Qt配置失败，返回错误
    }
    // 标记初始化完成
    m_isInited = true;

    // 启动日志文件清理检查定时器
    startCleanUpCheckTimer();

    return 0;
}

/**
 * @brief 初始化GSettings配置并设置监听
 * @param strAppName 应用程序名称
 * @param properties Log4Qt属性对象引用
 */
void UkuiLog4qtConfig::initSettings(QString strAppName, Log4Qt::Properties &properties)
{
    // 读取全局GSettings配置
    const QByteArray gid(UKUILOG4QT_SETTINGS);
    if (QGSettings::isSchemaInstalled(gid)) {
        m_gsLog4qtGlobal = new QGSettings(gid);

        // 设置全局配置变化监听器
        connect(m_gsLog4qtGlobal, &QGSettings::changed, [=](QString key) {
            QMutexLocker locker(single_config());
            QVariant gValue;
            bool bLogConfigChange = false;  // 标记是否需要重新配置Log4Qt
            bool bLogRollingChange = false; // 标记是否需要重新启动清理定时器

            // 处理重置配置变化
            if (UKUILOG4QT_SETTINGS_RESET == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_RESET);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_RESET, gValue.toString());
                    bLogConfigChange = true;
                }
            }
            // 处理调试级别配置变化
            else if (UKUILOG4QT_SETTINGS_DEBUG == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_DEBUG);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_DEBUG, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_HANDLEQT == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_HANDLEQT);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_HANDLEQT, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_ROOTLOGGER == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROOTLOGGER);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_ROOTLOGGER, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_DAILY_DATEPATTERN == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_DAILY_DATEPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_DAILY_DATEPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            }
            if (bLogConfigChange) {
                Log4Qt::PropertyConfigurator::configure(m_log4qtProperties);
            }
            if (UKUILOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
                if (gValue.isValid()) {
                    m_timeCheckFile = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
                if (gValue.isValid()) {
                    m_uMaxFileCount = gValue.toUInt();
                    bLogRollingChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE == key) {
                gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
                if (gValue.isValid()) {
                    m_uMaxFileSize = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            }
            if (bLogRollingChange) {
                startCleanUpCheckTimer();
            }
        });
    }
    if (m_gsLog4qtGlobal) {
        QVariant gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_RESET);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_RESET, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_DEBUG);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_DEBUG, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_HANDLEQT);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_HANDLEQT, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROOTLOGGER);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_ROOTLOGGER, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_DAILY_DATEPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_DAILY_DATEPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
        if (gValue.isValid()) {
            m_timeCheckFile = gValue.toULongLong();
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
        if (gValue.isValid()) {
            m_uMaxFileCount = gValue.toUInt();
        }
        gValue = m_gsLog4qtGlobal->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
        if (gValue.isValid()) {
            m_uMaxFileSize = gValue.toULongLong();
        }
    }
    //读取特例gsettings配置
    QString strSpecialSetting = QString("%1-%2").arg(UKUILOG4QT_SETTINGS).arg(strAppName);
    const QByteArray sid = strSpecialSetting.toUtf8();
    if (QGSettings::isSchemaInstalled(sid)) {
        m_gsLog4qtSpecial = new QGSettings(sid);
        connect(m_gsLog4qtSpecial, &QGSettings::changed, [=](QString key) {
            QMutexLocker locker(single_config());
            QVariant gValue;
            bool bLogConfigChange = false;
            bool bLogRollingChange = false;
            if (UKUILOG4QT_SETTINGS_HANDLEQT == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_HANDLEQT);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_HANDLEQT, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_ROOTLOGGER == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROOTLOGGER);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_ROOTLOGGER, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_DAILY_DATEPATTERN == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_DAILY_DATEPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_DAILY_DATEPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
                if (gValue.isValid()) {
                    m_log4qtProperties.setProperty(UKUILOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
                    bLogConfigChange = true;
                }
            }
            if (bLogConfigChange) {
                Log4Qt::PropertyConfigurator::configure(m_log4qtProperties);
            }
            if (UKUILOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
                if (gValue.isValid()) {
                    m_timeCheckFile = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
                if (gValue.isValid()) {
                    m_uMaxFileCount = gValue.toUInt();
                    bLogRollingChange = true;
                }
            } else if (UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE == key) {
                gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
                if (gValue.isValid()) {
                    m_uMaxFileSize = gValue.toULongLong();
                    bLogRollingChange = true;
                }
            }
            if (bLogRollingChange) {
                startCleanUpCheckTimer();
            }
        });
    }
    if (m_gsLog4qtSpecial) {
        QVariant gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROOTLOGGER);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_ROOTLOGGER, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_HANDLEQT);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_HANDLEQT, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_DAILY_DATEPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_DAILY_DATEPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_DAILY_CONVERSIONPATTERN);
        if (gValue.isValid()) {
            properties.setProperty(UKUILOG4QT_DAILY_CONVERSIONPATTERN, gValue.toString());
        }
        gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_DELAYTIME);
        if (gValue.isValid()) {
            m_timeCheckFile = gValue.toULongLong();
        }
        gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILECOUNT);
        if (gValue.isValid()) {
            m_uMaxFileCount = gValue.toUInt();
        }
        gValue = m_gsLog4qtSpecial->get(UKUILOG4QT_SETTINGS_ROLLINGCHECK_MAXFILESIZE);
        if (gValue.isValid()) {
            m_uMaxFileSize = gValue.toULongLong();
        }
    }
}

/**
 * @brief 获取日志文件列表
 * @return 返回当前日志目录下的所有日志文件信息列表
 */
QFileInfoList UkuiLog4qtConfig::getLogFiles() const
{
    QFileInfoList files;

    // 如果没有设置基础文件路径，返回空列表
    if (m_strBaseFilePath.isEmpty()) {
        return files;
    }

    try {
        // 检查基础日志文件是否存在
        QFileInfo currentFileInfo(m_strBaseFilePath);
        if (!currentFileInfo.exists()) {
            return files;
        }

        // 检查日志文件所在目录是否存在
        QDir dir(currentFileInfo.path());
        if (!dir.exists()) {
            return files;
        }

        // 检查目录是否可读
        QFileInfo dirInfo(currentFileInfo.path());
        if (!dirInfo.isReadable()) {
            qWarning() << "dir couldn't access:" << currentFileInfo.path();
            return files;
        }

        // 设置目录过滤器：只获取文件，排除符号链接
        dir.setFilter(QDir::Files | QDir::NoSymLinks);

        // 构建文件名过滤器：匹配基础文件名开头的所有文件
        QString baseName = currentFileInfo.baseName();
        QStringList filters;
        filters << baseName + "*";
        dir.setNameFilters(filters);

        // 按修改时间排序（最新的在前）
        dir.setSorting(QDir::Time);

        // 记录开始时间，用于性能监控
        QTime startTime = QTime::currentTime();
        files = dir.entryInfoList();

        // 检查文件扫描耗时，如果超过100ms则发出警告
        int elapsed = startTime.msecsTo(QTime::currentTime());
        if (elapsed > 100) {
            qWarning() << QString("File listing took %1ms, consider increasing cleanup interval").arg(elapsed);
        }
    } catch (const std::exception &e) {
        qWarning() << "get log files cause exception:" << e.what();
        files.clear();
    } catch (...) {
        qWarning() << "get log files cause unknown exception";
        files.clear();
    }

    return files;
}

/**
 * @brief 检查日志目录是否有效
 * @param mainDirPath 目录主路径
 * @param dirName 目录名称
 * @return 如果主目录存在且可读写，且子目录不存在或者存在且可读写返回true，否则返回false
 */
bool UkuiLog4qtConfig::checkLogDirectoryValid(const QString &mainDirPath, const QString &dirName) const
{
    QDir dir(mainDirPath);

    // 如果目录已存在，检查是否可访问
    if (dir.exists()) {
        QFileInfo dirInfo(mainDirPath);
        if (dirInfo.isReadable() && dirInfo.isWritable()) {
            QString logDirPath = mainDirPath + dirName;
            QDir logDir(logDirPath);
            if (logDir.exists()) {
                QFileInfo logDirInfo(logDirPath);
                if (logDirInfo.isReadable() && logDirInfo.isWritable()) {
                    return true;
                }
            } else {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 启动日志文件清理检查定时器
 */
void UkuiLog4qtConfig::startCleanUpCheckTimer()
{
    // 如果没有设置基础文件路径，不启动清理定时器
    if (m_strBaseFilePath.isEmpty()) {
        return;
    }

    // 创建并启动日志文件清理检查线程
    if (!m_threadCheckFile) {
        m_threadCheckFile = new UkuiLog4qtRolling(m_strBaseFilePath);
        m_threadCheckFile->start();
    }

    // 获取当前日志文件列表并设置检查限制条件
    QFileInfoList fileInfoList = getLogFiles();
    if (m_threadCheckFile) {
        m_threadCheckFile->setFileCheckLimit(fileInfoList, m_uMaxFileCount, m_uMaxFileSize * 1024 * 1024);
    }

    if (QCoreApplication::instance()) {
        // 创建清理检查定时器
        if (!m_cleanUpCheckTimer) {
            m_cleanUpCheckTimer = new QTimer(this);
            connect(m_cleanUpCheckTimer, &QTimer::timeout, this, &UkuiLog4qtConfig::onCleanUpCheckTimer);
        }

        // 设置定时器间隔并启动
        m_cleanUpCheckTimer->setInterval(m_timeCheckFile * 1000);
        m_cleanUpCheckTimer->start();
    } else {
        if (!m_isConnectToLogger) {
            connect(
                Log4Qt::LogManager::qtLogger(), &Log4Qt::Logger::logOutput, this, &UkuiLog4qtConfig::onLogOutput);
            m_isConnectToLogger = true;
        }
    }
}

void UkuiLog4qtConfig::onLogOutput()
{
    if (QCoreApplication::instance()) {
        // 创建清理检查定时器
        if (!m_cleanUpCheckTimer) {
            m_cleanUpCheckTimer = new QTimer(this);
            connect(m_cleanUpCheckTimer, &QTimer::timeout, this, &UkuiLog4qtConfig::onCleanUpCheckTimer);
        }

        // 设置定时器间隔并启动
        m_cleanUpCheckTimer->setInterval(m_timeCheckFile * 1000);
        m_cleanUpCheckTimer->start();
        if (m_isConnectToLogger) {
            disconnect(
                Log4Qt::LogManager::qtLogger(), &Log4Qt::Logger::logOutput, this, &UkuiLog4qtConfig::onLogOutput);
        }
    }
}

/**
 * @brief 停止日志文件清理检查定时器
 */
void UkuiLog4qtConfig::stopCleanUpCheckTimer()
{
    // 停止清理检查定时器
    if (m_cleanUpCheckTimer && m_cleanUpCheckTimer->isActive()) {
        m_cleanUpCheckTimer->stop();
    }

    // 停止并删除清理线程
    if (m_threadCheckFile) {
        m_threadCheckFile->stop();
        m_threadCheckFile->deleteLater();
    }
}

/**
 * @brief 定时器超时槽函数
 */
void UkuiLog4qtConfig::onCleanUpCheckTimer()
{
    // 如果没有设置基础文件路径或清理线程不存在，直接返回
    if (m_strBaseFilePath.isEmpty() || !m_threadCheckFile) {
        return;
    }

    // 重新获取日志文件列表
    QFileInfoList fileInfoList = getLogFiles();

    // 更新清理线程的文件检查限制条件
    m_threadCheckFile->setFileCheckLimit(fileInfoList, m_uMaxFileCount, m_uMaxFileSize * 1024 * 1024);
}
