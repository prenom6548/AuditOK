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
#ifndef ONLINEINFOMODULE_H
#define ONLINEINFOMODULE_H

#include <QObject>
#include <QtCore/QtCore>
#include <stdio.h>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include "OnlineInfoCollection.h"
#include "KylinDaqCommon.h"
#include "BaseModule.h"

class OnlineInfoModule : public BaseModule
{
    Q_OBJECT
private:
    static constexpr const char *s_bootDateTimeKey = "bootDateTime";
    static constexpr const char *s_shutdownDateTimeKey = "shutdownDateTime";
    static constexpr const char *s_lastRunDateTimeIntervalKey = "lastRunDateTimeInterval";
    static constexpr const char *s_lastShutdownDateTimeIntervalKey = "lastShutdownDateTimeInterval";
    static constexpr const char *s_lastRunDateTimeQuantumKey = "lastRunDateTimeQuantum";
    static constexpr const char *s_lastShutdownDateTimeQuantumKey = "lastShutdownDateTimeQuantum";


    /**
     * @brief json对象中source对象的packageName键对应值
     * 
     */
    static constexpr const char *s_packageNemeValue = "kylin-daq";
    /**
     * @brief json对象中source对象的messageType键对应值
     * 
     */
    static constexpr const char *s_messageTypeValue = "onlineInformation";
    /**
     * @brief 模块名
     * 
     */
    static constexpr const char *s_moduleName = "OnlineInfoModule";
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
    OnlineInfo onlineInfo;
    /**
     * @brief 定时器
     * 
     */
    QTimer *m_timer;
    /**
     * @brief 单例对象
     * 
     */
    static QScopedPointer<OnlineInfoModule> ms_instance;
    /**
     * @brief 将BasicInfo对象转化为QJsonObject,可参考dbus接口协议
     * 
     * @param basicInfo 基础信息结构体
     * @return QJsonObject 基础信息Json数据
     */
    QJsonObject packageOnlineInfo(const OnlineInfo &onlineInfo);
    /**
     * @brief 获得source的QJsonObject,可参考dbus接口协议
     * 
     * @return QJsonObject source的QJsonObject
     */
    QJsonObject sourceInfo();
    OnlineInfoModule(QObject *parent = nullptr);

    void createStateMessageInfo(QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);
    QString createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);

    void saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType);
public:
    /**
     * @brief Destroy the Basic Info Module object
     * 
     */
    virtual ~OnlineInfoModule();
    /**
     * @brief singleton function
     * 
     * @return BasicInfoModule* 
     */
    static OnlineInfoModule *instance(void);
    /**
     * @brief 收集基础信息并发送包含基础信息的信号
     * 
     */
    void dealInfo();
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

#endif // ONLINEINFOMODULE_H
