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
#ifndef COMMONINFOMODULE_H
#define COMMONINFOMODULE_H

#include <QObject>
#include <algorithm>
#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>
#include <QtDBus/QtDBus>
#include <stdio.h>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include "ConfModule.h"
#include "CommonInfoCollection.h"
#include "BaseModule.h"

class CommonInfoModule : public BaseModule
{
    Q_OBJECT
private:
    /**
     * @brief 主板信息请求头键
     * 
     */
    static constexpr const char* s_mainBoardSerialNumberKeyHeader = "X-Machine-Code";
    /**
     * @brief 网卡mac请求头键
     * 
     */
    static constexpr const char* s_networkMacAddressKeyHeader = "X-Machine-Mac";
    /**
     * @brief 处理器类型请求头键
     * 
     */
    static constexpr const char* s_processorModelKeyHeader = "X-Machine-Cpu";
    /**
     * @brief 显卡类型请求头键
     * 
     */
    static constexpr const char* s_graphicsCardModelKeyHeader = "X-Graphics-Card-Type";
    /**
     * @brief 系统版本请求头键
     * 
     */
    static constexpr const char* s_systemVersionKeyHeader = "X-System-Version";
    /**
     * @brief 系统服务序列号请求头键
     * 
     */
    static constexpr const char* s_systemSerialNumberKeyHeader = "X-Service-Serial-No";
    /**
     * @brief 网卡类型请求头键
     * 
     */
    static constexpr const char* s_networkTypeKeyHeader = "X-Network-Type";
    /**
     * @brief 硬盘序列号请求头键
     * 
     */
    static constexpr const char* s_hardDiskSerialNumberKeyHeader = "X-Hard-Disk-Serial-No";
    /**
     * @brief 内核版本请求头键
     * 
     */
    static constexpr const char* s_kernelVersionKeyHeader = "X-Kernel-Version";
    /**
     * @brief 系统uuid
     * 
     */
    static constexpr const char* s_systemUuidKeyHeader = "X-System-UUID";
    /**
     * @brief 厂商名
     *
     */
    static constexpr const char* s_systemManufacturerKeyHeader = "X-System-Manufacturer";
    /**
     * @brief 产品名
     *
     */
    static constexpr const char* s_systemProductNameKeyHeader = "X-System-Product-Name";
    /**
     * @brief machineId
     * 
     */
    static constexpr const char* s_machineIdKeyHeader = "X-Uid";
    /**
      * @brief register number
      */
    static constexpr const char* s_registerNumberHeader = "X-Register-Number";
    static constexpr const char *s_dpkgVersionKeyHeader = "X-Dpkg-Version";
    static constexpr const char *s_kysecVersionKeyHeader = "X-Kysec-Version";
    static constexpr const char *s_kysecAuthVersionKeyHeader = "X-Kysec-Auth-Version";
    static constexpr const char *s_kysecDaemonVersionKeyHeader = "X-Kysec-Daemon-Version";
    static constexpr const char *s_kysecUtilsVersionKeyHeader = "X-Kysec-Utils-Version";
    static constexpr const char *s_libkysecVersionKeyHeader = "X-Lib-Kysec-Version";
    static constexpr const char *s_osVersionKeyHeader = "X-Os-Version";
    static constexpr const char *s_osReleaseIdKeyHeader = "X-Os-Release-Id";
    static constexpr const char *s_osBuildInfoKeyHeader = "X-Os-Build-Info";
    static constexpr const char *s_commonInfoSha256KeyHeader = "X-Version-Id";
    static constexpr const char *s_osSerialNumberKeyHeader = "X-Os-Serial-Number";
    static constexpr const char *s_systemArchitectureKeyHeader = "X-System-Architecture";
    static constexpr const char *s_systemLanguageKeyHeader = "X-System-Language";
    static constexpr const char *s_commonInfoPackageNameValue = "kylin-daq";
    static constexpr const char *s_commonInfoMessageTypeValue = "commonInformation";

    /**
     * @brief 模块名
     * 
     */
    static constexpr const char *s_moduleName = "CommonInfoModule";
    /**
     * @brief 定时器间隔配置键
     * 
     */
    static constexpr const char *s_pollingIntervalKey = "PollingInterval";
    static constexpr const char *s_uploadTypeKey = "UploadType";


    static constexpr const char* s_mainBoardSerialNumberKey = "machineCode";
    static constexpr const char* s_networkMacAddressKey = "machineMac";
    static constexpr const char* s_processorModelKey = "cpuType";
    static constexpr const char* s_graphicsCardModelKey = "graphicsCardType";
    static constexpr const char* s_systemVersionKey = "systemVersion";
    static constexpr const char* s_systemSerialNumberKey = "serviceSerialNo";
    static constexpr const char* s_networkTypeKey = "networkType";
    static constexpr const char* s_hardDiskSerialNumberKey = "hardDiskSerialNo";
    static constexpr const char* s_kernelVersionKey = "kernelVersion";
    static constexpr const char* s_systemUuidKey = "systemUuid";
    static constexpr const char* s_systemManufacturerKey = "systemManufacturer";
    static constexpr const char* s_systemProductNameKey = "systemProductName";
    static constexpr const char* s_machineIdKey = "uid";
    static constexpr const char* s_registerNumber = "registerNumber";
    static constexpr const char *s_dpkgVersionKey = "dpkgVersion";
    static constexpr const char *s_kysecVersionKey = "kysecVersion";
    static constexpr const char *s_kysecAuthVersionKey = "kysecAuthVersion";
    static constexpr const char *s_kysecDaemonVersionKey = "kysecDaemonVersion";
    static constexpr const char *s_kysecUtilsVersionKey = "kysecUtilsVersion";
    static constexpr const char *s_libkysecVersionKey = "libKysecVersion";
    static constexpr const char *s_osVersionKey = "osVersion";
    static constexpr const char *s_osReleaseIdKey = "osReleaseId";
    static constexpr const char *s_osBuildInfoKey = "osBuildInfo";
    static constexpr const char *s_osTreeVersionKey = "osTreeVersion";
    static constexpr const char *s_osPatchVersionKey = "osPatchVersion";
    static constexpr const char *s_osSerialNumberKey = "osSerialNumber";
    static constexpr const char *s_systemArchitectureKey = "systemArchitecture";
    static constexpr const char *s_systemLanguageKey = "systemLanguage";
    /**
     * @brief 定时器
     * 
     */
    QTimer *m_timer;
    /**
     * @brief 单例对象
     * 
     */
    static QScopedPointer<CommonInfoModule> ms_instance;
    /**
     * @brief 打印基础信息
     * 
     * @param commonInfo 基础信息
     */
    void printInfo(const CommonInfo &commonInfo);
    /**
     * @brief 生成适合存储的基础信息字符串
     * 
     * @param commonInfo 基础信息
     * @return QString 基础信息字符串
     */
    QString packageHeaderInfo(const CommonInfo &commonInfo);
    QString packageInfo(const CommonInfo &commonInfo);
    QString createRequestHeadInfo(const CommonInfo &commonInfo);
    void createStateInfo(const CommonInfo &commonInfo,QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj);
    /**
     * @brief Construct a new Common Info Module object
     * 
     * @param parent 
     */
    explicit CommonInfoModule(QObject *parent = nullptr);

    void createStateMessageInfo(QJsonObject &packageInfoJsonObj, QJsonObject &uploadedMessageJsonObj, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);

    QString createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);

    void saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::UploadTypeEnum &uploadType);
public:
    /**
     * @brief Destroy the Common Info Module object
     * 
     */
    virtual ~CommonInfoModule();
    /**
     * @brief singleton function
     * 
     * @return CommonInfoModule* 单例结果
     */
    static CommonInfoModule *instance(void);
    /**
     * @brief 获取公共信息
     * 
     */
    void dealInfo();
public slots:
    virtual void updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf);
    void setTimePollingInterval(qlonglong sec);
signals:
    /**
     * @brief 发送公共信息
     * 
     * @param info 公共信息
     */
    void sendInfo(const QString &info);
    /**
     * @brief 存储时间信息
     * @param info
     */
    void saveInfo(const QString &info);
    void getHisotryStateMessage(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState);
    void saveStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);
    void updateStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,KylinDaqCommon::UploadStateEnum &uploadState);


    //	property
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
    void setPollingInterval(qlonglong pollingInterval)
    {
        if (m_pollingInterval == pollingInterval)
            return;

        m_pollingInterval = pollingInterval;
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

#endif // COMMONINFOCOLLECTION_H
