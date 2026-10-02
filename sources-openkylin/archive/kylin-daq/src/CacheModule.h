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
#ifndef CACHEMODULE_H
#define CACHEMODULE_H

#include "DataBaseInterface.h"
#include "ConfModule.h"
#include "TransmitModule.h"
#include "KylinDaqCommon.h"
#include "CacheCheck.h"
#include "CacheCreate.h"
#include "CacheRecv.h"
#include "CacheUpload.h"
#include "BaseModule.h"

class CacheModule : public BaseModule
{
    Q_OBJECT
private:
    // conf key
    /**
     * @brief 传输模块名
     * 
     */
    static constexpr const char *s_transmitModuleName = "TransmitModule";
    /*
     * @brief 定量上传数
     *
     */
    static constexpr const char *s_transmitModuleNumOfQuantitativeUploadLinesKey = "NumOfQuantitativeUploadLines";
    /**
     * @brief 模块名
     * 
     */
    static constexpr const char* s_moduleName = "CacheModule";
    /**
     * @brief 定时器配置字段
     * 
     */
    static constexpr const char* s_pollingIntervalKey = "PollingInterval";
    /**
     * @brief 数据缓存大小配置字段
     * 
     */
    static constexpr const char* s_cacheSizeKey = "CacheSize";
    /**
     * @brief 数据存储时长配置字段
     * 
     */
    static constexpr const char* s_storageDurationKey = "StorageDuration";
    /**
     * @brief 数据存储最大行数配置字段
     * 
     */
    static constexpr const char* s_cacheRowsKey = "CacheRows";
    /**
     * @brief 数据单行最大存储长度配置字段
     * 
     */
    static constexpr const char* s_singleLineBytesKey = "SingleLineBytes";
    /**
     * @brief 数据最大上传次数配置字段
     * 
     */
    static constexpr const char* s_uploadTimesKey = "UploadTimes";
    /**
     * @brief 定时器
     * 
     */
    QTimer *m_timer;
    /**
     * @brief 数据库检测对象
     * 
     */
    CacheCheck *m_cacheCheck;
    /**
     * @brief 数据库创建对象
     * 
     */
    CacheCreate *m_cacheCreate;
    /**
     * @brief 数据库接收数据对象
     * 
     */
    CacheRecv *m_cacheRecv;
    /**
     * @brief 数据库发送数据对象
     * 
     */
    CacheUpload *m_cacheUpload;
    /**
     * @brief 单例对象
     * 
     */
    static QScopedPointer<CacheModule> ms_instance;
protected:
    /**
     * @brief 打印配置
     *
     */
    void printConf();
    /**
     * @brief Construct a new Cache Module object
     *
     * @param parent
     */
    explicit CacheModule(QObject *parent = nullptr);
public:
    /**
     * @brief Destroy the Cache Module object
     * 
     */
    virtual ~CacheModule();
    /**
     * @brief singleton function
     * 
     * @return CacheModule* 
     */
    static CacheModule *instance(void);
public slots:
    //	更新配置
    virtual void updateProperty(const QJsonObject &conf,const QJsonObject &defaultConf);
    //   存储接收到的数据
    /**
     * @brief 接收埋点数据
     * 
     * @param info 埋点数据
     */
    void recvUploadInfo(const QString &info);
    /**
     * @brief 接收事件埋点数据
     *
     * @param info 事件埋点数据
     */
    void recvUploadEventInfo(const QString &info);
    /**
     * @brief 接收公共信息
     * 
     * @param info 公共信息
     */
    void recvCommonInfo(const QString &info);
    /**
     * @brief 接收Tid信息
     * 
     * @param info tid信息
     */
    void recvTidInfo(const QString &info);

    void recvSaveUploadStateInfo(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);

    void recvUpdateUploadStateInfo(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);

    /**
     * @brief 接收上传的返回结果
     * 
     * @param id 序列号
     * @param info 埋点信息
     * @param status 埋点信息状态
     * @param sendInfoStatus 上传结果
     */
    void recvUploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void recvEventUploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    /**
     * @brief 接收重传的返回结果
     * 
     * @param id 序列号
     * @param info 埋点信息
     * @param status 埋点信息状态
     * @param sendInfoStatus 重传传结果
     */
    void recvReuploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void recvEventReuploadInfoAndState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void recvStateInfoAndState(const QList<QString> &packageInfoList, const TransmitModule::SendInfoStatus &sendInfoStatus);
    /**
     * @brief 接收需要上传数据的数目
     * 
     * @param num 最大上传数据的数目
     */
    void recvUploadSignal(qlonglong num);
    /**
     * @brief 接收需要重传数据的数目
     *
     * @param num 最大重传数据的数目
     */
    void recvEventReuploadSignal(qlonglong num);

    /**
     * @brief 接收需要上传数据的数目
     *
     * @param num 最大上传数据的数目
     */
    void recvEventUploadSignal(qlonglong num);
    /**
     * @brief 接收需要重传数据的数目
     * 
     * @param num 最大重传数据的数目
     */
    void recvReuploadSignal(qlonglong num);
    void recvUploadStateInfoSingal(qlonglong sendedInfoNum);
    /**
     * @brief 检测数据库中是否已经存储了该tid信息
     * 
     * @param info 被检测的tid信息
     * @param tidState 检测结果
     */
    void checkTidInfo(const QString &info, bool &tidState);
    /**
     * @brief 
    */
    void getUploadStateMessageInfo(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState);
    /**
     * @brief 设置定时器触发时间间隔
     * 
     * @param sec 定时器触发时间间隔
     */
    void setTimerPollingInterval(qlonglong sec);
    /**
     * @brief 检查数据库存储数据状态
     * 
     */
    void recvCheckDateBaseSingal();

signals:
    /**
     * @brief 待上传数据
     * 
     * @param id 数据库中序列号
     * @param info 待上传信息
     * @param status 待上传信息状态
     * @param requestHead 请求头
     */
    void sendUploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    /**
     * @brief 待重传数据
     * 
     * @param id 数据库中序列号
     * @param info 待重传信息
     * @param status 待重传信息状态
     * @param requestHead 请求头
     */
    void sendReuploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    /**
     * @brief 待上传数据
     *
     * @param id 数据库中序列号
     * @param info 待上传信息
     * @param status 待上传信息状态
     * @param requestHead 请求头
     */
    void sendEventUploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    /**
     * @brief 待重传数据
     *
     * @param id 数据库中序列号
     * @param info 待重传信息
     * @param status 待重传信息状态
     * @param requestHead 请求头
     */
    void sendEventReuploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);


    void sendUpladStateInfo(const QList<QString> &packageInfoList,const QList<QString> &uploadedMessageList, const QList<QString> &tidsList, const QList<QString> &createTimeStampList, const QList<KylinDaqCommon::UploadStateEnum> uploadStateList,QString &requestHead);
    // property
private:
    Q_PROPERTY(qlonglong pollingInterval READ pollingInterval WRITE setPollingInterval NOTIFY pollingIntervalChanged)
    Q_PROPERTY(qlonglong cacheSize READ cacheSize WRITE setCacheSize NOTIFY cacheSizeChanged)
    Q_PROPERTY(qlonglong storageDuration READ storageDuration WRITE setStorageDuration NOTIFY storageDurationChanged)
    Q_PROPERTY(qlonglong cacheRows READ cacheRows WRITE setCacheRows NOTIFY cacheRowsChanged)
    Q_PROPERTY(qlonglong singleLineBytes READ singleLineBytes WRITE setSingleLineBytes NOTIFY singleLineBytesChanged)
    Q_PROPERTY(qlonglong uploadTimes READ uploadTimes WRITE setUploadTimes NOTIFY uploadTimesChanged)
    Q_PROPERTY(qlonglong numOfQuantitativeUploadLines READ numOfQuantitativeUploadLines WRITE setNumOfQuantitativeUploadLines NOTIFY numOfQuantitativeUploadLinesChanged)
    qlonglong m_pollingInterval;
    qlonglong m_cacheSize;
    qlonglong m_storageDuration;
    qlonglong m_cacheRows;
    qlonglong m_singleLineBytes;
    qlonglong m_uploadTimes;
    qlonglong m_numOfQuantitativeUploadLines;

public:
    qlonglong pollingInterval() const
    {
        return m_pollingInterval;
    }

    qlonglong cacheSize() const
    {
        return m_cacheSize;
    }

    qlonglong storageDuration() const
    {
        return m_storageDuration;
    }

    qlonglong cacheRows() const
    {
        return m_cacheRows;
    }

    qlonglong singleLineBytes() const
    {
        return m_singleLineBytes;
    }

    qlonglong uploadTimes() const
    {
        return m_uploadTimes;
    }
    qlonglong numOfQuantitativeUploadLines() const
    {
        return m_numOfQuantitativeUploadLines;
    }
public slots:
    void setPollingInterval(qlonglong pollingInterval)
    {
        if (m_pollingInterval == pollingInterval)
            return;

        m_pollingInterval = pollingInterval;
        emit pollingIntervalChanged(m_pollingInterval);
    }

    void setCacheSize(qlonglong cacheSize)
    {
        if (m_cacheSize == cacheSize)
            return;

        m_cacheSize = cacheSize;
        emit cacheSizeChanged(m_cacheSize);
    }

    void setStorageDuration(qlonglong storageDuration)
    {
        if (m_storageDuration == storageDuration)
            return;

        m_storageDuration = storageDuration;
        emit storageDurationChanged(m_storageDuration);
    }

    void setCacheRows(qlonglong cacheRows)
    {
        if (m_cacheRows == cacheRows)
            return;

        m_cacheRows = cacheRows;
        emit cacheRowsChanged(m_cacheRows);
    }

    void setSingleLineBytes(qlonglong singleLineBytes)
    {
        if (m_singleLineBytes == singleLineBytes)
            return;

        m_singleLineBytes = singleLineBytes;
        emit singleLineBytesChanged(m_singleLineBytes);
    }

    void setUploadTimes(qlonglong uploadTimes)
    {
        if (m_uploadTimes == uploadTimes)
            return;

        m_uploadTimes = uploadTimes;
        emit uploadTimesChanged(m_uploadTimes);
    }
    void setNumOfQuantitativeUploadLines(qlonglong numOfQuantitativeUploadLines)
    {
        if (m_numOfQuantitativeUploadLines == numOfQuantitativeUploadLines)
            return;

        m_numOfQuantitativeUploadLines = numOfQuantitativeUploadLines;
        emit numOfQuantitativeUploadLinesChanged(m_numOfQuantitativeUploadLines);
        m_cacheRecv->setNumOfQuantitativeUploadLines(m_numOfQuantitativeUploadLines);
    }
signals:
    void pollingIntervalChanged(qlonglong pollingInterval);

    void cacheSizeChanged(qlonglong cacheSize);

    void storageDurationChanged(qlonglong storageDuration);

    void cacheRowsChanged(qlonglong cacheRows);

    void singleLineBytesChanged(qlonglong singleLineBytes);

    void uploadTimesChanged(qlonglong uploadTimes);

    void numOfQuantitativeUploadLinesChanged(qlonglong numOfQuantitativeUploadLines);
};

#endif // DATAMANAGER_H
