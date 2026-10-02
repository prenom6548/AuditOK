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
/*** 
 * @Author: pengcheng (async refactor)
 * @Description: 方案三：全异步实现（基于最新同步源码改写，保持原日志格式与对外接口不变）
 */
#ifndef DATATRANSMIT_H
#define DATATRANSMIT_H

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <QTimer>
#include "BaseModule.h"
#include "KylinDaqCommon.h"
#include <QDBusInterface>
#include <QDateTime>

class TransmitModule: public BaseModule
{
    Q_OBJECT
public:
    enum SendInfoStatus
    {
        OK = 0,
        FormatException = -1,
        NetworkAnomaly = -2,
        ServerException = -3,
        EncryptMd5Exception = -4,
        ServerReplyException = -5,
        UrlException = -6
    };
    enum RequestKind {
        RK_BatchUpload,
        RK_BatchReupload,
        RK_BatchState,
        RK_EventSingleUpload,
        RK_EventSingleReupload,
        RK_Config
    };
private:
    static constexpr const char *s_messageKey = "messages";
    //	reply keys
    static constexpr const char *s_replyErrorCodeKey = "code";
    static constexpr const char *s_replyErrorMsgKey = "msg";
    static constexpr const char *s_replyResponseBodyKey = "bo";
    //	HTTP param
    static constexpr const char *s_requestTyepValue = "application/json";
    static constexpr const int32_t s_replyValidCode = 200;
    static constexpr const int32_t s_httpReplyValidCode = 200;
    //	API url
    static constexpr const char *s_uploadUrl = "collect-info-server/collect/info/submit/event/v1";
    static constexpr const char *s_reuploadUrl = "collect-info-server/collect/info/submit/event/v1";
    static constexpr const char *s_stateInfoUploadUrl = "collect-info-server/collect/info/submit/status/v1";
    static constexpr const char *s_configUrl = "collect-info-server/collect/info/config/v1";
    static constexpr const char *s_uploadEventUrl = "kylin-data-collection-service/collect-api/submit/v1";
    // conf
    static constexpr const char *s_moduleName = "TransmitModule";
    static constexpr const char *s_uploadIntervalKey = "UploadInterval";
    static constexpr const char *s_reuploadIntervalKey = "ReuploadInterval";
    static constexpr const char *s_stateInfoUploadIntervalKey = "UploadStateInfoInterval";
    static constexpr const char *s_numOfUploadedLinesKey = "NumOfUploadedLines";
    static constexpr const char *s_numOfReuploadedLinesKey = "NumOfReuploadedLines";
    static constexpr const char *s_numOfStateInfoUploadedLinesKey = "NumOfUploadedStateInfoLines";
    static constexpr const char *s_uploadStateKey = "UploadState";
    static constexpr const char *s_uploadRealmKey = "UploadRealm";
    static constexpr const char *s_uploadRequestMaxTimeKey = "UploadRequestMaxTime";
    static constexpr const char *s_numOfQuantitativeUploadLinesKey = "NumOfQuantitativeUploadLines";
    // HTTP HEAD
    static constexpr const char *s_httpUploadTimeStamp= "X-Upload-Timestamp";
    // field
    static constexpr const char *s_uploadTimeKey="uploadTimestamp";
    static constexpr const int32_t s_uploadIntervalThreshold = 120;

    // 大小策略（字节数）
    static constexpr const int32_t s_largeSingleThreshold = 100 * 1024; // 100KB
    static constexpr const int32_t s_maxBatchBytes = 10 * 1024;         // 10KB 近似



    QScopedPointer<QNetworkAccessManager> m_mannager;
    QTimer *uploadTimer;
    QTimer *reuploadTimer;
    QTimer *uploadStateInfoTimer;

    // 原 busy 拆分为 5 条独立通道
    bool m_uploadInfoBusy = false;      // 普通批量上传
    bool m_reuploadInfoBusy = false;    // 批量重传
    bool m_stateInfoBusy = false;       // 状态信息上传
    bool m_uploadEventBusy = false;     // 事件上传
    bool m_reuploadEventBusy = false;   // 事件重传

    struct idList{
        QList<qlonglong> successId;
        QList<qlonglong> failId;
        QList<QString> failInfo;
    };

    // --------------------- 单例 ---------------------
    static QScopedPointer<TransmitModule> ms_instance;


protected:
    // --------------------- 异步结构 ---------------------
    int m_roundSeq = 0;

    struct RoundContext {
        RequestKind kind;
        QList<qlonglong> allIds;
        QList<QString> allInfos;
        QList<QString> allStatus;
        int totalParts = 0;
        int finishedParts = 0;
        SendInfoStatus finalStatus = OK;
        bool anyFinished = false;
        int roundId = 0;

        QList<qlonglong> successIds;
        QList<QString>   successInfos;
        QList<QString>   successStatus;
        QList<qlonglong> failIds;
        QList<QString>   failInfos;
        QList<QString>   failStatus;
        QList<QString>   origPackageInfoList;
    };

    struct BatchRealmContext {
        QList<QString> realms;
        int realmIndex = 0;
        QList<qlonglong> ids;
        QList<QString> infoList;
        QList<QString> statusList;
        QString requestHead;
        RequestKind kind;
        bool finished = false;
        bool largeSingle = false;
        QSharedPointer<RoundContext> roundCtx;
    };

    struct EventBatchContext {
        QString realm;
        QList<qlonglong> ids;
        QList<QString> infos;
        QList<QString> status;
        QString requestHead;
        int index = 0;
        QList<qlonglong> successId;
        QList<qlonglong> failId;
        QList<QString> failInfo;
        QList<QString> failStatus;
        bool reupload = false;
    };

    struct ConfigRealmContext {
        QList<QString> realms;
        int realmIndex = 0;
        QString requestHead;
        bool finished = false;
    };

    struct PendingRequest {
        RequestKind kind;
        QNetworkRequest request;
        QByteArray data;
        int timeoutMs = 0;
        QSharedPointer<BatchRealmContext> batchCtx;
        QSharedPointer<EventBatchContext> eventCtx;
        QSharedPointer<ConfigRealmContext> configCtx;
        QList<QString> batchInfoList;
        qlonglong singleEventId = -1;
        QString singleEventInfo;
    };

    struct ReplyContext {
        PendingRequest pr;
        QTimer *timeoutTimer = nullptr;
    };

    QQueue<PendingRequest> m_queue;
    QHash<QNetworkReply*, ReplyContext> m_replyMap;
    QHash<QTimer*, QNetworkReply*> m_timerMap;
    int m_inFlight = 0;
    int m_maxConcurrency = 2;

    void abortAllReplies();

    void startBatchUpload(const QList<qlonglong> &ids,
                          const QList<QString> &info,
                          const QList<QString> &status,
                          const QString &requestHead,
                          RequestKind kind,
                          const QList<QString> &origPackageInfoList = QList<QString>());

    void postOneBatchRealm(QSharedPointer<BatchRealmContext> ctx);

    void startEventBatch(const QList<qlonglong> &ids,
                         const QList<QString> &infos,
                         const QList<QString> &status,
                         const QString &requestHead,
                         bool reupload);

    void enqueueNextEventItem(QSharedPointer<EventBatchContext> ctx);

    void startConfigFetch(const QString &requestHead);
    void postOneConfigRealm(QSharedPointer<ConfigRealmContext> ctx);

    // 统计一轮 batch 的完成情况
    void onOneBatchFinished(const QSharedPointer<BatchRealmContext> &ctx, SendInfoStatus st);

    // --------------------- 逻辑函数 ---------------------
    SendInfoStatus checkUrl(const QUrl &urlValue);
    SendInfoStatus mergeInfo(const QList<QString> &info,QByteArray &requestData);
    SendInfoStatus setRequest(QNetworkRequest &request,const QUrl &url,const QJsonObject &requestHeadObj);
    void printSendInfoLog(const QList<QString> &info,const SendInfoStatus &requestState,int32_t requestErrorCode,const QString &requestErrorRelyStr);
    void printSendInfoLog(const int &requestId,const SendInfoStatus &requestState, int32_t requestErrorCode, const QString &requestErrorRelyStr);
    void sendUploadInfoNum();
    void sendReuploadInfoNum();
    void sendStateInfoNum();
    bool networkIsConnected();
    QString createStateInfo(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);

    // --------------------- 异步内部实现 ---------------------
    void enqueue(const PendingRequest &pr);
    void tryStart();
    void startOne(const PendingRequest &pr);
    void finishReply(QNetworkReply *reply, SendInfoStatus st, int32_t errCode, const QString &errMsg, const QString &body);
    void onReplyFinished();
    void onReplyFinished(QNetworkReply *reply);
    void onReplyTimeout();
    void onReplyTimeout(QTimer *timer);

public:
    explicit TransmitModule(QObject *parent = nullptr);
    virtual ~TransmitModule();
    static TransmitModule *instance(void);

public slots:
    void updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf);

    void uploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    void reuploadInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    void uploadEventInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    void reuploadEventInfo(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const QString &requestHead);
    void uploadStateInfo(const QList<QString> &packageInfoList,const QList<QString> &uploadedMessageList, const QList<QString> &tidsList, const QList<QString> &createTimeStampList,const QList<KylinDaqCommon::UploadStateEnum> & uploadStateList, const QString &requestHead);
    void setUploadTimerPollingInterval(qlonglong sec);
    void setReuploadTimerPollingInterval(qlonglong sec);
    void setUploadStateInfoTimerPollingInterval(qlonglong sec);
    void printConf();
    void getConfig();

signals:
    void uploadInfoSignal(qlonglong num);
    void reuploadInfoSignal(qlonglong num);
    void uploadStateInfoSignal(qlonglong num);

    void retUploadState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void retReuploadState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void retUploadStateInfoState(const QList<QString> &packageInfoList, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void sendConfig(const QString &config);

    void uploadEventInfoSignal(qlonglong num);
    void reuploadEventInfoSignal(qlonglong num);
    void retEventUploadState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);
    void retEventReuploadState(const QList<qlonglong> &id,const QList<QString> &info,const QList<QString> & status, const TransmitModule::SendInfoStatus &sendInfoStatus);

private:
    // Q_PROPERTY
    Q_PROPERTY(qlonglong uploadInterval READ uploadInterval WRITE setUploadInterval NOTIFY uploadIntervalChanged)
    Q_PROPERTY(qlonglong reuploadInterval READ reuploadInterval WRITE setReuploadInterval NOTIFY reuploadIntervalChanged)
    Q_PROPERTY(qlonglong uploadStateInfoInterval READ uploadStateInfoInterval WRITE setUploadStateInfoInterval NOTIFY uploadStateInfoIntervalChanged)
    Q_PROPERTY(qlonglong numOfUploadedLines READ numOfUploadedLines WRITE setNumOfUploadedLines NOTIFY numOfUploadedLinesChanged)
    Q_PROPERTY(qlonglong numOfReuploadedLines READ numOfReuploadedLines WRITE setNumOfReuploadedLines NOTIFY numOfReuploadedLinesChanged)
    Q_PROPERTY(qlonglong numOfUploadedStateInfoLines READ numOfUploadedStateInfoLines WRITE setNumOfUploadedStateInfoLines NOTIFY numOfUploadedStateInfoLinesChanged)
    Q_PROPERTY(QList<QString> uploadRealm READ uploadRealm WRITE setUploadRealm NOTIFY uploadRealmChanged)
    Q_PROPERTY(KylinDaqCommon::UploadState uploadState READ uploadState WRITE setUploadState NOTIFY uploadStateChanged)
    Q_PROPERTY(qlonglong uploadRequestMaxTime READ uploadRequestMaxTime WRITE setUploadRequestMaxTime NOTIFY uploadRequestMaxTimeChanged)
    Q_PROPERTY(qlonglong numOfQuantitativeUploadLines READ numOfQuantitativeUploadLines WRITE setNumOfQuantitativeUploadLines NOTIFY numOfQuantitativeUploadLinesChanged)

    // property vars
    qlonglong m_numOfUploadedLines = 0;
    qlonglong m_numOfReuploadedLines = 0;
    qlonglong m_uploadInterval = 0;
    qlonglong m_reuploadInterval = 0;
    KylinDaqCommon::UploadState m_uploadState = KylinDaqCommon::UploadDisable;
    QList<QString> m_uploadRealm;
    qlonglong m_uploadRequestMaxTime = 0;
    qlonglong m_numOfUploadedStateInfoLines = 0;
    qlonglong m_uploadStateInfoInterval = 0;
    qlonglong m_numOfQuantitativeUploadLines = 0;

public:
    // getters
    qlonglong numOfUploadedLines() const { return m_numOfUploadedLines; }
    qlonglong numOfReuploadedLines() const { return m_numOfReuploadedLines; }
    qlonglong uploadInterval() const { return m_uploadInterval; }
    qlonglong reuploadInterval() const { return m_reuploadInterval; }
    KylinDaqCommon::UploadState uploadState() const { return m_uploadState; }
    QList<QString> uploadRealm() const { return m_uploadRealm; }
    qlonglong uploadRequestMaxTime() const { return m_uploadRequestMaxTime; }
    qlonglong numOfUploadedStateInfoLines() const { return m_numOfUploadedStateInfoLines; }
    qlonglong uploadStateInfoInterval() const { return m_uploadStateInfoInterval; }
    qlonglong numOfQuantitativeUploadLines() const { return m_numOfQuantitativeUploadLines; }

public slots:
    // setters
    void setNumOfUploadedLines(qlonglong v){ if(m_numOfUploadedLines==v)return; m_numOfUploadedLines=v; emit numOfUploadedLinesChanged(v); }
    void setNumOfReuploadedLines(qlonglong v){ if(m_numOfReuploadedLines==v)return; m_numOfReuploadedLines=v; emit numOfReuploadedLinesChanged(v); }
    void setUploadInterval(qlonglong v){ if(m_uploadInterval==v)return; m_uploadInterval=v; emit uploadIntervalChanged(v); }
    void setReuploadInterval(qlonglong v){ if(m_reuploadInterval==v)return; m_reuploadInterval=v; emit reuploadIntervalChanged(v); }
    void setUploadState(KylinDaqCommon::UploadState v){ if(m_uploadState==v)return; m_uploadState=v; emit uploadStateChanged(v); }
    void setUploadRealm(QList<QString> v){ if(m_uploadRealm==v)return; m_uploadRealm=v; emit uploadRealmChanged(v); }
    void setUploadRequestMaxTime(qlonglong v){ if(m_uploadRequestMaxTime==v)return; m_uploadRequestMaxTime=v; emit uploadRequestMaxTimeChanged(v); }
    void setNumOfUploadedStateInfoLines(qlonglong v){ if(m_numOfUploadedStateInfoLines==v)return; m_numOfUploadedStateInfoLines=v; emit numOfUploadedStateInfoLinesChanged(v); }
    void setUploadStateInfoInterval(qlonglong v){ if(m_uploadStateInfoInterval==v)return; m_uploadStateInfoInterval=v; emit uploadStateInfoIntervalChanged(v); }
    void setNumOfQuantitativeUploadLines(qlonglong v){ if(m_numOfQuantitativeUploadLines==v)return; m_numOfQuantitativeUploadLines=v; emit numOfQuantitativeUploadLinesChanged(v); }

signals:
    // property signals
    void numOfUploadedLinesChanged(qlonglong);
    void numOfReuploadedLinesChanged(qlonglong);
    void uploadIntervalChanged(qlonglong);
    void reuploadIntervalChanged(qlonglong);
    void uploadStateChanged(KylinDaqCommon::UploadState);
    void uploadRealmChanged(QList<QString>);
    void uploadRequestMaxTimeChanged(qlonglong);
    void numOfUploadedStateInfoLinesChanged(qlonglong);
    void uploadStateInfoIntervalChanged(qlonglong);
    void numOfQuantitativeUploadLinesChanged(qlonglong);
};

#endif
