/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONFMODULE_H
#define CONFMODULE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QDir>
#include "KylinDaqCommon.h"
#include <QSet>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include "BaseModule.h"

class ConfModule : public BaseModule
{
    Q_OBJECT
protected:
    /**
     * @brief 模块名
     *
     */
    static constexpr const char* s_moduleName = "ConfModule";
    static constexpr const char *s_pollingIntervalKey = "PollingInterval";
    static constexpr const char *s_transmitModuleUploadStateKey = "UploadState";
    static constexpr const char *s_transmitModuleKey = "TransmitModule";
    /**
     * @brief 定时器
     * 
     */
    QTimer *m_timer;
    /**
     * @brief 默认配置路径
     * 
     */
    QString m_defaultFilePath;
    /**
     * @brief 缓存配置路径
     * 
     */
    QString m_cacheFolderPath;
    /**
     * @brief 文件名
     * 
     */
    QString m_confFileName;
    /**
     * @brief 文件监控对象
     * 
     */
    QFileSystemWatcher* m_fileSystemWatcher;
    /**
     * @brief 单例对象
     * 
     */
    static QScopedPointer<ConfModule> ms_instance;
    //	function
    /**
     * @brief 检测并创建文件夹
     *
     * @param folderath 文件夹路径
     */
    void checkDir(const QString &folderath);
    /**
     * @brief 从指定文件中读取json对象
     *
     * @param filePath 文件路径
     * @return QJsonObject json对象
     */
    QJsonObject readJson(const QString &filePath);
    /**
     * @brief 向指定文件中写json对象
     *
     * @param filePath 文件路径
     * @param jsonObj 待写入json对象
     */
    void writeJson(const QString &filePath,QJsonObject &jsonObj);
    /**
     * @brief Construct a new Conf Module object
     *
     * @param parent
     */
    ConfModule(QObject *parent=nullptr);

    /**
     * @brief 配置文件防抖读取定时器（新增）
     * 
     */
    QTimer *m_confReadTimer;

private slots:
    /**
     * @brief 延迟真正读取配置的槽函数（新增）
     * 
     */
    void doReadConfDelayed();

public:
    /**
     * @brief Destroy the Conf Module object
     * 
     */
    virtual ~ConfModule();
    /**
     * @brief instance function
     * 
     * @return ConfModule* 
     */
    static ConfModule *instance(void);
    /**
     * @brief 从指定文件读取配置
     * 
     * @param filePath 文件路径
     */
    void readConf(const QString &filePath);
public slots:
    virtual void updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf);
    /**
     * @brief 模块写配置
     * 
     * @param moduleName 模块名
     * @param key 键
     * @param value 值
     */
    void writeConf(const QString &moduleName, const QString &key, const QVariant &value);
    void writeConf2(const QString &conf);
    void setTimePollingInterval(qlonglong sec);
signals:
    /**
     * @brief 将配置发送给其他模块
     * 
     * @param conf 配置
     * @param defaultConf 默认配置
     */
    void sendConf(const QJsonObject &conf,const QJsonObject &defaultConf);
    void getConf();
    //	property
private:
    Q_PROPERTY(qlonglong pollingInterval READ pollingInterval WRITE setPollingInterval NOTIFY pollingIntervalChanged)
    qlonglong m_pollingInterval;

public:
qlonglong pollingInterval() const
{
    return m_pollingInterval;
}

public slots:
void setPollingInterval(qlonglong pollingInterval)
{
    if (m_pollingInterval == pollingInterval)
        return;

    m_pollingInterval = pollingInterval;
    emit pollingIntervalChanged(m_pollingInterval);
}

signals:
void pollingIntervalChanged(qlonglong pollingInterval);
};

#endif