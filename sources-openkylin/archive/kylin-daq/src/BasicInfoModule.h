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
#ifndef BASICINFOMODULE_H
#define BASICINFOMODULE_H

#include <QObject>
#include <QtCore/QtCore>
#include <stdio.h>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include "BasicInfoCollection.h"
#include "KylinDaqCommon.h"
#include "BaseModule.h"

class BasicInfoModule: public BaseModule
{
    Q_OBJECT
private:
    //  protocol key
    /**
     * @brief json对象中information对象的内核版本键
     * 
     */
    static constexpr const char *s_kernelVersionKey = "kernelVersion";
    /**
     * @brief json对象中information对象的gcc版本键
     * 
     */
    static constexpr const char *s_gccVersionKey = "gccVersion";
    /**
     * @brief json对象中information对象的glibc版本键
     * 
     */
    static constexpr const char *s_glibcVersiKey = "glibcVersion";
    /**
     * @brief json对象中information对象的gdb版本键
     * 
     */
    static constexpr const char *s_gdbVersionKey = "gdbVersion";
    /**
     * @brief json对象中information对象的qt5版本键
     * 
     */
    static constexpr const char *s_qt5VersionKey = "qt5Version";
    /**
     * @brief json对象中information对象的qt4版本键
     * 
     */
    static constexpr const char *s_qt4VersionKey = "qt4Version";
    /**
     * @brief json对象中information对象的python3版本键
     * 
     */
    static constexpr const char *s_python3VersionKey = "python3Version";
    /**
     * @brief json对象中information对象的python2版本键
     * 
     */
    static constexpr const char *s_python2VersionKey = "python2Version";
    /**
     * @brief json对象中information对象的java版本键
     * 
     */
    static constexpr const char *s_javaVersionKey = "javaVersion";
    /**
     * @brief json对象中information对象的grub版本键
     * 
     */
    static constexpr const char *s_grubVersionKey = "grubVersion";
    /**
     * @brief json对象中information对象的cups版本键
     * 
     */
    static constexpr const char *s_cupsVersionKey = "cupsVersion";
    /**
     * @brief json对象中information对象的libjbig版本键
     * 
     */
    static constexpr const char *s_libjbigVersionKey = "libjbigVersion";
    /**
     * @brief json对象中information对象的libpng版本键
     * 
     */
    static constexpr const char *s_libpngVersionKey = "libpngVersion";
    /**
     * @brief json对象中information对象的libjpeg版本键
     * 
     */
    static constexpr const char *s_libjpegKey = "libjpegVersion";
    /**
     * @brief 
     * 
     */
    static constexpr const char *s_kylinDaqVersionKey = "kylinDaqVersion";
    /**
     * @brief json对象中source对象的packageName键对应值
     * 
     */
    static constexpr const char *s_packageNemeValue = "kylin-daq";
    /**
     * @brief json对象中source对象的messageType键对应值
     * 
     */
    static constexpr const char *s_messageTypeValue = "basicInformation";
    /**
     * @brief 模块名
     * 
     */
    static constexpr const char *s_moduleName = "BasicInfoModule";
    /**
     * @brief 定时器配置
     * 
     */
    static constexpr const char *s_pollingIntervalKey = "PollingInterval";    
    static constexpr const char *s_uploadTypeKey = "UploadType";
    /**
     * @brief 基础信息对象
     * 
     */
    BasicInfo basicInfo;
    /**
     * @brief 定时器
     * 
     */
    QTimer *m_timer;
    /**
     * @brief 单例对象
     * 
     */
    static QScopedPointer<BasicInfoModule> ms_instance;
    /**
     * @brief 将BasicInfo对象转化为QJsonObject,可参考dbus接口协议
     * 
     * @param basicInfo 基础信息结构体
     * @return QJsonObject 基础信息Json数据
     */
    QJsonObject packageBasicInfo(const BasicInfo &basicInfo);
    /**
     * @brief 获得source的QJsonObject,可参考dbus接口协议
     * 
     * @return QJsonObject source的QJsonObject
     */
    QJsonObject sourceInfo();
    /**
     * @brief Construct a new Basic Info Module object
     * 
     * @param parent 
     */
    BasicInfoModule(QObject *parent=nullptr);
private:

    void createStateMessageInfo(QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);
    QString createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);

    void saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType);
public:
    /**
     * @brief Destroy the Basic Info Module object
     * 
     */
    virtual ~BasicInfoModule();
    /**
     * @brief singleton function
     * 
     * @return BasicInfoModule* 
     */
    static BasicInfoModule *instance(void);
    /**
     * @brief 收集基础信息并发送包含基础信息的信号
     * 
     */
    void dealInfo();
    /**
     * @brief 打印基础信息
     * 
     * @param basicInfo 基础信息结构体
     */
    void printInfo(const BasicInfo &basicInfo);
public slots:
    /**
     * @brief 设置定时器激发间隔
     * 
     * @param sec 定时间隔
     */
    void setTimerPollingInterval(qlonglong sec);
    //	update property
    virtual void updateProperty(const QJsonObject &conf,const QJsonObject &defaultConf);
signals:
    /**
     * @brief 发送基础信息的信号
     * 
     * @param info 封装好的基础信息
     */
    void saveInfo(const QString &info);
    void getHisotryStateMessage(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState);
    void saveStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);
    void updateStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,KylinDaqCommon::UploadStateEnum &uploadState);
    // 设置属性
private:
    Q_PROPERTY(qlonglong pollingInterval READ pollingInterval WRITE setPollingInterval NOTIFY pollingIntervalChanged)
    Q_PROPERTY(qlonglong uploadType READ uploadType WRITE setUploadType NOTIFY uploadTypeChanged)
    qlonglong m_pollingInterval;
    qlonglong m_uploadType;

public:
    qlonglong pollingInterval() const
    {
        return m_pollingInterval;
    }
    qlonglong uploadType() const
    {
        return m_uploadType;
    }

public slots:
    void setPollingInterval(qlonglong PollingInterval)
    {
        if (m_pollingInterval == PollingInterval)
            return;

        m_pollingInterval = PollingInterval;
        emit pollingIntervalChanged(m_pollingInterval);
    }
    void setUploadType(qlonglong uploadType)
    {
        if (m_uploadType == uploadType)
            return;

        m_uploadType = uploadType;
        emit uploadTypeChanged(m_uploadType);
    }

signals:
    void pollingIntervalChanged(qlonglong pollingInterval);
    void uploadTypeChanged(qlonglong uploadType);
};
#endif
