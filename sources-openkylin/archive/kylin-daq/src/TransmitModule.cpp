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
 * @Author: pengcheng
 * @Date: 2021-06-24 10:27:50
 * @LastEditTime: 2021-08-24 14:10:39
 * @LastEditors: pengcheng
 * @Description: 
 * @FilePath: /kylin-daq/src/DataTransmit.cpp
 */
#include "TransmitModule.h"
#include <qglobal.h>

QScopedPointer<TransmitModule> TransmitModule::ms_instance;

TransmitModule::TransmitModule(QObject *parent)
    : BaseModule(s_moduleName,parent)
    , m_mannager(new QNetworkAccessManager(this))
    , uploadTimer(new QTimer(this))
    , reuploadTimer(new QTimer(this))
    , uploadStateInfoTimer(new QTimer(this))
{
    connect(this, &TransmitModule::uploadIntervalChanged, this, &TransmitModule::setUploadTimerPollingInterval);
    connect(this, &TransmitModule::reuploadIntervalChanged, this, &TransmitModule::setReuploadTimerPollingInterval);
    connect(this, &TransmitModule::uploadStateInfoIntervalChanged, this, &TransmitModule::setUploadStateInfoTimerPollingInterval);
    connect(uploadTimer, &QTimer::timeout, this , &TransmitModule::sendUploadInfoNum);
    connect(reuploadTimer, &QTimer::timeout, this , &TransmitModule::sendReuploadInfoNum);
    connect(uploadStateInfoTimer, &QTimer::timeout, this , &TransmitModule::sendStateInfoNum);

    uploadTimer->setSingleShot(false);
    reuploadTimer->setSingleShot(false);
    uploadStateInfoTimer->setSingleShot(false);

    m_uploadInfoBusy = false;
    m_reuploadInfoBusy = false;
    m_stateInfoBusy = false;
    m_uploadEventBusy = false;
    m_reuploadEventBusy = false;
}

TransmitModule::~TransmitModule()
{
    abortAllReplies();
}

TransmitModule *TransmitModule::instance()
{
    if (ms_instance.isNull()) {
        ms_instance.reset(new TransmitModule());
    }
    return ms_instance.data();
}

/* ------------------ 异步调度核心 ------------------ */
void TransmitModule::enqueue(const PendingRequest &pr)
{
    m_queue.enqueue(pr);
    tryStart();
}

void TransmitModule::tryStart()
{
    while(m_inFlight < m_maxConcurrency && !m_queue.isEmpty()){
        PendingRequest pr = m_queue.dequeue();
        startOne(pr);
    }
}

void TransmitModule::startOne(const PendingRequest &pr)
{
    QNetworkReply *reply = m_mannager->post(pr.request, pr.data);
    ReplyContext rc;
    rc.pr = pr;
    rc.timeoutTimer = new QTimer(this);
    rc.timeoutTimer->setSingleShot(true);
    connect(reply,&QNetworkReply::finished,this,static_cast<void (TransmitModule::*)()>(&TransmitModule::onReplyFinished));
    // 原来是：connect(rc.timeoutTimer,&QTimer::timeout,this,&TransmitModule::onReplyTimeout);
    connect(rc.timeoutTimer,&QTimer::timeout,this,static_cast<void (TransmitModule::*)()>(&TransmitModule::onReplyTimeout));
    rc.timeoutTimer->start(pr.timeoutMs);
    m_replyMap.insert(reply, rc);

    // 新增：建立 timer -> reply 映射，供 onReplyTimeout 精确定位
    if(rc.timeoutTimer){
        m_timerMap.insert(rc.timeoutTimer, reply);
    }

    ++m_inFlight;
}

void TransmitModule::onReplyFinished()
{
    onReplyFinished(qobject_cast<QNetworkReply*>(sender()));
}

void TransmitModule::onReplyFinished(QNetworkReply *reply)
{
    if(!reply) return;
    auto it = m_replyMap.find(reply);
    if(it == m_replyMap.end()) return;
    ReplyContext rc = it.value();

    SendInfoStatus st = OK;
    int32_t errCode = 0;
    QString errMsg;
    QString body;

    if(reply->error() != QNetworkReply::NoError){
        st = NetworkAnomaly;
        errCode = reply->error();
        errMsg = reply->errorString();
    }else{
        QVariant stateCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(stateCode.isValid()){
            if(stateCode.toInt() == s_httpReplyValidCode){
                QByteArray raw = reply->readAll();
                body = QString::fromUtf8(raw);
                QJsonDocument replyDoc = QJsonDocument::fromJson(raw);
                if(replyDoc.isObject()){
                    QJsonObject replyObj = replyDoc.object();
                    if (replyObj.contains(s_replyErrorCodeKey)){
                        if (replyObj[s_replyErrorCodeKey].toInt() != s_replyValidCode){
                            st = ServerReplyException;
                            errCode = replyObj[s_replyErrorCodeKey].toInt();
                            if (replyObj.contains(s_replyErrorMsgKey)){
                                errMsg = replyObj[s_replyErrorMsgKey].toString();
                            }
                        }
                        if(st == OK){
                            if (replyObj.contains(s_replyResponseBodyKey) && replyObj[s_replyResponseBodyKey].isObject()){
                                body = QString(QJsonDocument(replyObj[s_replyResponseBodyKey].toObject()).toJson());
                            }
                        }
                    }else{
                        st = ServerReplyException;
                    }
                }else{
                    st = ServerReplyException;
                }
            }else{
                st = ServerException;
                errCode = stateCode.toInt();
            }
        }else{
            st = ServerException;
        }
    }
    finishReply(reply, st, errCode, errMsg, body);
}

void TransmitModule::onReplyTimeout()
{
    onReplyTimeout(qobject_cast<QTimer*>(sender()));
}

void TransmitModule::onReplyTimeout(QTimer *timer)
{
    if (!timer) {
        return;
    }

    auto itTimer = m_timerMap.find(timer);
    if (itTimer == m_timerMap.end()) {
        return;
    }
    QNetworkReply *reply = itTimer.value();
    if (!reply) {
        m_timerMap.erase(itTimer);
        return;
    }

    auto it = m_replyMap.find(reply);
    if (it == m_replyMap.end()) {
        m_timerMap.erase(itTimer);
        return;
    }

    ReplyContext rc = it.value();

    // 打印超时请求的类型和基本信息
    const QString replyPtrStr = QString::asprintf("%p", static_cast<void*>(reply));

    switch (rc.pr.kind) {
    case RK_BatchUpload:
    case RK_BatchReupload:
    case RK_BatchState:
        qWarning().noquote()
            << QString("TransmitModule::onReplyTimeout: kind=batch, infoCount=%1, reply=%2")
                   .arg(rc.pr.batchInfoList.size())
                   .arg(replyPtrStr);
        break;
    case RK_EventSingleUpload:
    case RK_EventSingleReupload:
        qWarning().noquote()
            << QString("TransmitModule::onReplyTimeout: kind=event, id=%1, reupload=%2, reply=%3")
                   .arg(rc.pr.singleEventId)
                   .arg(rc.pr.kind == RK_EventSingleReupload)
                   .arg(replyPtrStr);
        break;
    case RK_Config:
        qWarning().noquote()
            << QString("TransmitModule::onReplyTimeout: kind=config, reply=%1")
                   .arg(replyPtrStr);
        break;
    }

    if (rc.timeoutTimer) {
        rc.timeoutTimer->stop();
    }

    if (reply->isRunning()) {
        reply->abort();
    }

    // 统一当作网络异常
    finishReply(reply, NetworkAnomaly, -1, QStringLiteral("timeout"), QString());
}

void TransmitModule::onOneBatchFinished(const QSharedPointer<BatchRealmContext> &ctx,
                                        SendInfoStatus st)
{
    if (!ctx || !ctx->roundCtx) {
        return;
    }
    QSharedPointer<RoundContext> round = ctx->roundCtx;
    round->anyFinished = true;

    int roundId = round->roundId;
    int partSize = ctx->infoList.size();

    // 每个批次完成时打印一条结果日志
    qInfo().noquote()
        << QString("TransmitModule::onOneBatchFinished: round=%1, partResult=%2, batchSize=%3, largeSingle=%4")
               .arg(roundId)
               .arg((int)st)
               .arg(partSize)
               .arg(ctx->largeSingle);

    // 按批汇总成功/失败子集
    if (st == OK || st == FormatException) {
        round->successIds.append(ctx->ids);
        round->successInfos.append(ctx->infoList);
        round->successStatus.append(ctx->statusList);
    } else {
        round->failIds.append(ctx->ids);
        round->failInfos.append(ctx->infoList);
        round->failStatus.append(ctx->statusList);
    }

    // 记录最后一个批次状态（用于失败状态枚举）
    round->finalStatus = st;
    round->finishedParts++;
    if (round->finishedParts < round->totalParts) {
        return;
    }

    // 整轮结束，打印汇总日志
    qInfo().noquote()
        << QString("TransmitModule::onOneBatchFinished: round=%1 finished, kind=%2, totalParts=%3, successCount=%4, failCount=%5")
               .arg(roundId)
               .arg((int)round->kind)
               .arg(round->totalParts)
               .arg(round->successIds.size())
               .arg(round->failIds.size());

    if (round->kind == RK_BatchUpload) {
        if (!round->successIds.isEmpty()) {
            qInfo().noquote()
                << QString("TransmitModule::onOneBatchFinished: round=%1, uploadInfo successIds=%2")
                       .arg(roundId)
                       .arg(QString::number(round->successIds.size()));
            emit retUploadState(round->successIds,
                                round->successInfos,
                                round->successStatus,
                                OK);
        }
        if (!round->failIds.isEmpty()) {
            SendInfoStatus failSt = round->finalStatus;
            if (failSt == OK || failSt == FormatException) {
                failSt = ServerException;
            }
            qWarning().noquote()
                << QString("TransmitModule::onOneBatchFinished: round=%1, uploadInfo failIds=%2, failStatus=%3")
                       .arg(roundId)
                       .arg(QString::number(round->failIds.size()))
                       .arg((int)failSt);
            emit retUploadState(round->failIds,
                                round->failInfos,
                                round->failStatus,
                                failSt);
        }
        m_uploadInfoBusy = false;
    } else if (round->kind == RK_BatchReupload) {
        if (!round->successIds.isEmpty()) {
            qInfo().noquote()
                << QString("TransmitModule::onOneBatchFinished: round=%1, reuploadInfo successIds=%2")
                       .arg(roundId)
                       .arg(QString::number(round->successIds.size()));
            emit retReuploadState(round->successIds,
                                  round->successInfos,
                                  round->successStatus,
                                  OK);
        }
        if (!round->failIds.isEmpty()) {
            SendInfoStatus failSt = round->finalStatus;
            if (failSt == OK || failSt == FormatException) {
                failSt = ServerException;
            }
            qWarning().noquote()
                << QString("TransmitModule::onOneBatchFinished: round=%1, reuploadInfo failIds=%2, failStatus=%3")
                       .arg(roundId)
                       .arg(QString::number(round->failIds.size()))
                       .arg((int)failSt);
            emit retReuploadState(round->failIds,
                                  round->failInfos,
                                  round->failStatus,
                                  failSt);
        }
        m_reuploadInfoBusy = false;
    } else if (round->kind == RK_BatchState) {
        qInfo().noquote()
            << QString("TransmitModule::onOneBatchFinished: round=%1, stateInfo finalStatus=%2")
                   .arg(roundId)
                   .arg((int)round->finalStatus);
        // 为兼容老版接口，第一个参数使用原始 packageInfoList
        const QList<QString> &retList =
            !round->origPackageInfoList.isEmpty() ? round->origPackageInfoList
                                                  : round->allInfos;

        emit retUploadStateInfoState(retList, round->finalStatus);
        m_stateInfoBusy = false;
    }
}

void TransmitModule::finishReply(QNetworkReply *reply,
                                 SendInfoStatus st,
                                 int32_t errCode,
                                 const QString &errMsg,
                                 const QString &body)
{
    auto it = m_replyMap.find(reply);
    if (it != m_replyMap.end()) {
        ReplyContext rc = it.value();

        // 清理 timeout timer
        if (rc.timeoutTimer) {
            auto itTimer = m_timerMap.find(rc.timeoutTimer);
            if (itTimer != m_timerMap.end()) {
                m_timerMap.erase(itTimer);
            }
            rc.timeoutTimer->stop();
            rc.timeoutTimer->deleteLater();
        }

        PendingRequest pr = rc.pr;

        switch (pr.kind) {
        case RK_BatchUpload:
        case RK_BatchReupload:
        case RK_BatchState: {
            // 批量上传 / 重传 / 状态上报
            printSendInfoLog(pr.batchInfoList, st, errCode, errMsg);
            if (pr.batchCtx && !pr.batchCtx->finished) {
                if (st == OK || st == FormatException) {
                    // 当前 realm 成功，整批完成
                    pr.batchCtx->finished = true;
                    onOneBatchFinished(pr.batchCtx, st);
                } else {
                    // 当前 realm 失败，尝试下一个 realm
                    pr.batchCtx->realmIndex++;
                    if (pr.batchCtx->realmIndex < pr.batchCtx->realms.size()) {
                        QMetaObject::invokeMethod(
                            this,
                            [this, ctx = pr.batchCtx]() { postOneBatchRealm(ctx); },
                            Qt::QueuedConnection);
                    } else {
                        // 所有 realm 都失败，整批失败
                        pr.batchCtx->finished = true;
                        onOneBatchFinished(pr.batchCtx, st);
                    }
                }
            }
            break;
        }

        case RK_EventSingleUpload:
        case RK_EventSingleReupload: {
            // 原有：按 id 打日志
            printSendInfoLog((int)pr.singleEventId, st, errCode, errMsg);

            // 新增：结构化事件日志
            QString realm = pr.eventCtx ? pr.eventCtx->realm : QString();
            qInfo().noquote()
                << QString("TransmitModule::finishReply(Event): realm=%1, id=%2, reupload=%3, status=%4, errCode=%5, errMsg=%6")
                       .arg(realm)
                       .arg(pr.singleEventId)
                       .arg(pr.kind == RK_EventSingleReupload)
                       .arg((int)st)
                       .arg(errCode)
                       .arg(errMsg);

            if (pr.eventCtx) {
                if (st == OK) {
                    pr.eventCtx->successId << pr.singleEventId;
                } else {
                    pr.eventCtx->failId << pr.singleEventId;
                    pr.eventCtx->failInfo << pr.singleEventInfo;

                    // 记录该失败条目的 status 子集
                    if (!pr.eventCtx->status.isEmpty()
                        && pr.eventCtx->index >= 0
                        && pr.eventCtx->index < pr.eventCtx->status.size()) {
                        pr.eventCtx->failStatus << pr.eventCtx->status.at(pr.eventCtx->index);
                    } else {
                        // 对齐长度，填空串
                        pr.eventCtx->failStatus << QString();
                    }
                }

                pr.eventCtx->index++;
                bool isLast = (pr.eventCtx->index >= pr.eventCtx->infos.size());

                // 继续发送下一条事件
                QMetaObject::invokeMethod(
                    this,
                    [this, ctx = pr.eventCtx]() { enqueueNextEventItem(ctx); },
                    Qt::QueuedConnection);

                // 当前 realm 的最后一条事件处理完，解除 busy
                if (isLast) {
                    if (pr.kind == RK_EventSingleUpload) {
                        m_uploadEventBusy = false;
                    } else {
                        m_reuploadEventBusy = false;
                    }
                }
            }
            break;
        }

        case RK_Config: {
            if (st == OK) {
                emit sendConfig(body);
            } else if (pr.configCtx) {
                // 尝试下一个 realm
                pr.configCtx->realmIndex++;
                if (pr.configCtx->realmIndex < pr.configCtx->realms.size()) {
                    QMetaObject::invokeMethod(
                        this,
                        [this, ctx = pr.configCtx]() { postOneConfigRealm(ctx); },
                        Qt::QueuedConnection);
                }
            }
            break;
        }
        }

        m_replyMap.erase(it);
    }

    reply->deleteLater();
    --m_inFlight;
    if (m_inFlight < 0)
        m_inFlight = 0;

    QMetaObject::invokeMethod(this, [this]() { tryStart(); }, Qt::QueuedConnection);
}

void TransmitModule::abortAllReplies()
{
    for(auto it = m_replyMap.begin(); it!= m_replyMap.end(); ){
        QNetworkReply *r = it.key();
        ReplyContext rc = it.value();
        if(r->isRunning()) r->abort();
        if(rc.timeoutTimer){
            // 新增：清理 timer 映射
            auto itTimer = m_timerMap.find(rc.timeoutTimer);
            if(itTimer != m_timerMap.end()){
                m_timerMap.erase(itTimer);
            }
            rc.timeoutTimer->stop();
            rc.timeoutTimer->deleteLater();
        }
        r->deleteLater();
        it = m_replyMap.erase(it);
    }
    m_queue.clear();
    m_inFlight = 0;
}

/* ------------------ 批量上传流程 ------------------ */
void TransmitModule::startBatchUpload(const QList<qlonglong> &ids,
                                      const QList<QString> &info,
                                      const QList<QString> &status,
                                      const QString &requestHead,
                                      RequestKind kind,
                                      const QList<QString> &origPackageInfoList)
{
    if(info.isEmpty()){
        if(kind == RK_BatchUpload)
            emit retUploadState(ids,info,status,FormatException);
        else if(kind == RK_BatchReupload)
            emit retReuploadState(ids,info,status,FormatException);
        else
            emit retUploadStateInfoState(info, FormatException);
        return;
    }
    if(m_uploadRealm.isEmpty()){
        if(kind == RK_BatchUpload)
            emit retUploadState(ids,info,status,UrlException);
        else if(kind == RK_BatchReupload)
            emit retReuploadState(ids,info,status,UrlException);
        else
            emit retUploadStateInfoState(info, UrlException);
        return;
    }

    // 一轮上下文
    QSharedPointer<RoundContext> round(new RoundContext);
    round->kind = kind;
    round->allIds = ids;
    round->allInfos = info;
    round->allStatus = status;
    if (kind == RK_BatchState) {
        round->origPackageInfoList = origPackageInfoList;
    }
    round->roundId = ++m_roundSeq;

    qInfo().noquote() << QString("TransmitModule::startBatchUpload: round=%1, kind=%2, totalInfo=%3")
                         .arg(round->roundId)
                         .arg((int)kind)
                         .arg(info.size());

    // 先拆分：large 单条 + 小批
    QList<QList<qlonglong>> partIds;
    QList<QList<QString>> partInfos;
    QList<QList<QString>> partStatus;
    QList<bool> partIsLarge;

    int largeCount = 0; // 统计大包数量

    // large 单条
    for(int i=0;i<info.size();++i){
        const QString &s = info.at(i);
        int sz = s.toUtf8().size();
        if(sz >= s_largeSingleThreshold){
            QList<qlonglong> idsPart;
            QList<QString> infosPart;
            QList<QString> statusPart;
            if(!ids.isEmpty() && i < ids.size()) idsPart.append(ids.at(i));
            infosPart.append(s);
            if(!status.isEmpty() && i < status.size()) statusPart.append(status.at(i));
            partIds.append(idsPart);
            partInfos.append(infosPart);
            partStatus.append(statusPart);
            partIsLarge.append(true);
            ++largeCount;
        }
    }

    // 普通小包：< 100KB 的
    QList<qlonglong> normalIds;
    QList<QString> normalInfos;
    QList<QString> normalStatus;
    for(int i=0;i<info.size();++i){
        const QString &s = info.at(i);
        int sz = s.toUtf8().size();
        if(sz >= s_largeSingleThreshold){
            continue;
        }
        normalInfos.append(s);
        if(!ids.isEmpty() && i < ids.size()) normalIds.append(ids.at(i));
        if(!status.isEmpty() && i < status.size()) normalStatus.append(status.at(i));
    }
    QList<qlonglong> curIds;
    QList<QString> curInfos;
    QList<QString> curStatus;
    int curBytes = 0;
    for(int i=0;i<normalInfos.size();++i){
        const QString &s = normalInfos.at(i);
        int sz = s.toUtf8().size();
        if(curBytes > 0 && curBytes + sz > s_maxBatchBytes){
            partIds.append(curIds);
            partInfos.append(curInfos);
            partStatus.append(curStatus);
            partIsLarge.append(false);
            curIds.clear();
            curInfos.clear();
            curStatus.clear();
            curBytes = 0;
        }
        curInfos.append(s);
        if(!normalIds.isEmpty() && i < normalIds.size())
            curIds.append(normalIds.at(i));
        if(!normalStatus.isEmpty() && i < normalStatus.size())
            curStatus.append(normalStatus.at(i));
        curBytes += sz;
    }
    if(!curInfos.isEmpty()){
        partIds.append(curIds);
        partInfos.append(curInfos);
        partStatus.append(curStatus);
        partIsLarge.append(false);
    }

    if(partInfos.isEmpty()){
        if(kind == RK_BatchUpload)
            emit retUploadState(ids,info,status,FormatException);
        else if(kind == RK_BatchReupload)
            emit retReuploadState(ids,info,status,FormatException);
        else
            emit retUploadStateInfoState(info, FormatException);
        return;
    }

    round->totalParts = partInfos.size();

    qInfo().noquote() << QString("TransmitModule::startBatchUpload: round=%1, parts=%2, largeSingles=%3, maxBatchBytes=%4")
                         .arg(round->roundId)
                         .arg(round->totalParts)
                         .arg(largeCount)
                         .arg(s_maxBatchBytes);

    for(int pi=0; pi<partInfos.size(); ++pi){
        QSharedPointer<BatchRealmContext> ctx(new BatchRealmContext);
        ctx->realms = m_uploadRealm;
        ctx->realmIndex = 0;
        ctx->ids = partIds.at(pi);
        ctx->infoList = partInfos.at(pi);
        ctx->statusList = partStatus.at(pi);
        ctx->requestHead = requestHead;
        ctx->kind = kind;
        ctx->finished = false;
        ctx->largeSingle = partIsLarge.at(pi);
        ctx->roundCtx = round;

        qDebug().noquote() << QString("TransmitModule::startBatchUpload: round=%1, part=%2, size=%3, largeSingle=%4")
                              .arg(round->roundId)
                              .arg(pi)
                              .arg(ctx->infoList.size())
                              .arg(ctx->largeSingle);

        postOneBatchRealm(ctx);
    }
}

void TransmitModule::postOneBatchRealm(QSharedPointer<BatchRealmContext> ctx)
{
    if(!ctx || ctx->realmIndex >= ctx->realms.size() || ctx->finished) return;

    QString realm = ctx->realms.at(ctx->realmIndex);
    QString relativeUrl;
    if(ctx->kind == RK_BatchUpload) relativeUrl = s_uploadUrl;
    else if(ctx->kind == RK_BatchReupload) relativeUrl = s_reuploadUrl;
    else relativeUrl = s_stateInfoUploadUrl;

    QString fullUrl = realm + relativeUrl;

    int roundId = ctx->roundCtx ? ctx->roundCtx->roundId : 0;

    qDebug().noquote() << QString("TransmitModule::sendInfo: round=%1, realmIndex=%2/%3, urlValue=%4, batchSize=%5, largeSingle=%6")
                          .arg(roundId)
                          .arg(ctx->realmIndex+1).arg(ctx->realms.size())
                          .arg(fullUrl)
                          .arg(ctx->infoList.size())
                          .arg(ctx->largeSingle);

    QUrl url(fullUrl);
    SendInfoStatus urlState = checkUrl(url);
    QByteArray requestData;
    SendInfoStatus mergeState = OK;
    if(urlState == OK){
        mergeState = mergeInfo(ctx->infoList, requestData);
    }
    if(urlState != OK || mergeState != OK){
        printSendInfoLog(ctx->infoList, urlState!=OK?urlState:mergeState, 0, QString());
        ctx->realmIndex++;
        if(ctx->realmIndex < ctx->realms.size()){
            QMetaObject::invokeMethod(this,[this,ctx](){ postOneBatchRealm(ctx); }, Qt::QueuedConnection);
        }else{
            ctx->finished = true;
            onOneBatchFinished(ctx, (urlState!=OK?urlState:mergeState));
        }
        return;
    }

    QJsonObject headObj;
    QJsonDocument headDoc = QJsonDocument::fromJson(ctx->requestHead.toUtf8());
    if(headDoc.isObject()) headObj = headDoc.object();
    QNetworkRequest request;
    setRequest(request, url, headObj);

    PendingRequest pr;
    pr.kind = ctx->kind;
    pr.request = request;
    pr.data = requestData;
    pr.timeoutMs = KylinDaqCommon::secToMsec(uploadRequestMaxTime());
    if(ctx->largeSingle){
        pr.timeoutMs *= 3;
    }
    pr.batchCtx = ctx;
    pr.batchInfoList = ctx->infoList;
    enqueue(pr);
}

/* ------------------ 事件上传流程 ------------------ */
void TransmitModule::startEventBatch(const QList<qlonglong> &ids,
                                     const QList<QString> &infos,
                                     const QList<QString> &status,
                                     const QString &requestHead,
                                     bool reupload)
{
    if(infos.isEmpty() || ids.size()!=infos.size()){
        if(reupload)
            emit retEventReuploadState(ids, infos, status, FormatException);
        else
            emit retEventUploadState(ids, infos, status, FormatException);
        return;
    }
    if(m_uploadRealm.isEmpty()){
        if(reupload)
            emit retEventReuploadState(ids, infos, status, UrlException);
        else
            emit retEventUploadState(ids, infos, status, UrlException);
        return;
    }

    qInfo().noquote() << QString("TransmitModule::startEventBatch: realmCount=%1, totalEvents=%2, reupload=%3")
                         .arg(m_uploadRealm.size())
                         .arg(infos.size())
                         .arg(reupload);

    for(const QString &realm : m_uploadRealm){
        QSharedPointer<EventBatchContext> ctx(new EventBatchContext);
        ctx->realm = realm;
        ctx->ids = ids;
        ctx->infos = infos;
        ctx->status = status;
        ctx->requestHead = requestHead;
        ctx->reupload = reupload;
        ctx->index = 0;
        enqueueNextEventItem(ctx);
    }
}

void TransmitModule::enqueueNextEventItem(QSharedPointer<EventBatchContext> ctx)
{
    if (!ctx)
        return;

    // 所有条目都处理完：回调结果
    if (ctx->index >= ctx->infos.size()) {
        qInfo().noquote()
            << QString("TransmitModule::enqueueNextEventItem: realm=%1, reupload=%2, successCount=%3, failCount=%4")
                   .arg(ctx->realm)
                   .arg(ctx->reupload)
                   .arg(ctx->successId.size())
                   .arg(ctx->failId.size());

        // 成功子集：这里仍然传原始 infos/status，Cache 在成功分支主要按 id 删除
        if (!ctx->successId.isEmpty()) {
            if (ctx->reupload)
                emit retEventReuploadState(ctx->successId, ctx->infos, ctx->status, OK);
            else
                emit retEventUploadState(ctx->successId, ctx->infos, ctx->status, OK);
        }

        // 失败子集：使用 failId/failInfo/failStatus 子集，三者一一对应
        if (!ctx->failId.isEmpty()) {
            QList<QString> failStatus = ctx->failStatus;
            // 兜底，避免长度不够
            while (failStatus.size() < ctx->failId.size()) {
                failStatus << QString();
            }

            if (ctx->reupload)
                emit retEventReuploadState(ctx->failId, ctx->failInfo, failStatus, UrlException);
            else
                emit retEventUploadState(ctx->failId, ctx->failInfo, failStatus, UrlException);
        }
        return;
    }

    // 继续发下一条
    qlonglong curId = ctx->ids.at(ctx->index);
    QString curInfo = ctx->infos.at(ctx->index);

    qDebug().noquote()
        << QString("TransmitModule::eventUpload: realm=%1, index=%2/%3, id=%4")
               .arg(ctx->realm)
               .arg(ctx->index + 1)
               .arg(ctx->infos.size())
               .arg(curId);

    QString fullUrl = ctx->realm + s_uploadEventUrl;
    QUrl url(fullUrl);
    SendInfoStatus urlState = checkUrl(url);
    if (urlState != OK) {
        // URL 本地配置错误，直接视为失败
        printSendInfoLog((int)curId, urlState, 0, QString());
        ctx->failId << curId;
        ctx->failInfo << curInfo;
        ctx->failStatus << QString(); // URL 错误时，用空 status 占位
        ctx->index++;
        QMetaObject::invokeMethod(
            this,
            [this, ctx]() { enqueueNextEventItem(ctx); },
            Qt::QueuedConnection);
        return;
    }

    QNetworkRequest request;
    QJsonObject headObj;
    QJsonDocument headDoc = QJsonDocument::fromJson(ctx->requestHead.toUtf8());
    if (headDoc.isObject())
        headObj = headDoc.object();
    setRequest(request, url, headObj);

    PendingRequest pr;
    pr.kind = ctx->reupload ? RK_EventSingleReupload : RK_EventSingleUpload;
    pr.request = request;
    pr.data = curInfo.toUtf8();
    pr.timeoutMs = KylinDaqCommon::secToMsec(uploadRequestMaxTime());
    pr.eventCtx = ctx;
    pr.singleEventId = curId;
    pr.singleEventInfo = curInfo;

    enqueue(pr);
}

/* ------------------ Config 获取 ------------------ */
void TransmitModule::startConfigFetch(const QString &requestHead)
{
    if(m_uploadRealm.isEmpty()){
        return;
    }
    QSharedPointer<ConfigRealmContext> ctx(new ConfigRealmContext);
    ctx->realms = m_uploadRealm;
    ctx->realmIndex = 0;
    ctx->requestHead = requestHead;
    postOneConfigRealm(ctx);
}

void TransmitModule::postOneConfigRealm(QSharedPointer<ConfigRealmContext> ctx)
{
    if(!ctx || ctx->realmIndex >= ctx->realms.size() || ctx->finished) return;
    QString realm = ctx->realms.at(ctx->realmIndex);
    QString fullUrl = realm + s_configUrl;
    qDebug()<< "TransmitModule::getInfo: urlValue="<<fullUrl;
    QUrl url(fullUrl);
    SendInfoStatus urlState = checkUrl(url);
    if(urlState != OK){
        ctx->realmIndex++;
        if(ctx->realmIndex < ctx->realms.size()){
            QMetaObject::invokeMethod(this,[this,ctx](){ postOneConfigRealm(ctx); }, Qt::QueuedConnection);
        }
        return;
    }
    QJsonObject headObj;
    QJsonDocument headDoc = QJsonDocument::fromJson(ctx->requestHead.toUtf8());
    if(headDoc.isObject()) headObj = headDoc.object();

    QNetworkRequest request;
    setRequest(request,url,headObj);

    PendingRequest pr;
    pr.kind = RK_Config;
    pr.request = request;
    pr.data = QByteArray();
    pr.timeoutMs = KylinDaqCommon::secToMsec(uploadRequestMaxTime());
    pr.configCtx = ctx;
    enqueue(pr);
}

/* ------------------ 公共逻辑(保留原实现样式) ------------------ */
TransmitModule::SendInfoStatus TransmitModule::checkUrl(const QUrl &urlValue)
{
    if(!urlValue.isValid())
    {
        return SendInfoStatus::UrlException;
    }
    if(urlValue.scheme() != "http"&&urlValue.scheme() != "https")
    {
        return SendInfoStatus::UrlException;
    }
    return SendInfoStatus::OK;
}

TransmitModule::SendInfoStatus TransmitModule::mergeInfo(const QList<QString> &info, QByteArray &requestData)
{
    QJsonArray mergeJsonObject;
    QJsonObject sendedJsonObject;
    foreach(const QString &tempInfo, info)
    {
        QJsonDocument tempJsonDoc = QJsonDocument::fromJson(tempInfo.toUtf8());
        if(!tempJsonDoc.isObject())
        {
            qWarning()<<QString("TransmitModule::mergeInfo: tempInfo = %1. it is invalid data.").arg(tempInfo);
            continue;
        }
        QJsonObject tempJsonObj =  tempJsonDoc.object();
        if(!tempJsonObj.contains(KylinDaqCommon::gs_sourceKey)
            || !tempJsonObj.contains(KylinDaqCommon::gs_informationKey)
            || !tempJsonObj[KylinDaqCommon::gs_sourceKey].isObject()
            || !tempJsonObj[KylinDaqCommon::gs_informationKey].isObject())
        {
            qWarning()<<QString("TransmitModule::mergeInfo: tempInfo = %1. it is invalid data.").arg(tempInfo);
            continue;
        }
        QJsonObject sourceJsonObj(tempJsonObj[KylinDaqCommon::gs_sourceKey].toObject());
        QJsonObject informationJsonObj(tempJsonObj[KylinDaqCommon::gs_informationKey].toObject());
        if(informationJsonObj.contains(KylinDaqCommon::gs_packageNameKey)
                || informationJsonObj.contains(KylinDaqCommon::gs_messageTypeKey)
                || informationJsonObj.contains(KylinDaqCommon::gs_tidKey)
                || informationJsonObj.contains(KylinDaqCommon::gs_parentTidKey))
        {
            qWarning()<<QString("TransmitModule::mergeInfo: tempInfo = %1. it is invalid data.").arg(tempInfo);
            continue;
        }
        QJsonObject  waitAddJsonObj(informationJsonObj);
        if(!sourceJsonObj.contains(KylinDaqCommon::gs_packageNameKey)
                || !sourceJsonObj.contains(KylinDaqCommon::gs_messageTypeKey)
                || !sourceJsonObj.contains(KylinDaqCommon::gs_tidKey))
        {
            qWarning()<<QString("TransmitModule::mergeInfo: tempInfo = %1. it is invalid data.").arg(tempInfo);
            continue;
        }
        waitAddJsonObj.insert(KylinDaqCommon::gs_packageNameKey,sourceJsonObj[KylinDaqCommon::gs_packageNameKey].toString());
        waitAddJsonObj.insert(KylinDaqCommon::gs_messageTypeKey,sourceJsonObj[KylinDaqCommon::gs_messageTypeKey].toString());
        waitAddJsonObj.insert(KylinDaqCommon::gs_tidKey,sourceJsonObj[KylinDaqCommon::gs_tidKey].toString());
        waitAddJsonObj.insert(KylinDaqCommon::gs_sourceVersionKey,KylinDaqCommon::gs_sourceVersionValue);
        if(sourceJsonObj.contains(KylinDaqCommon::gs_parentTidKey))
        {
            waitAddJsonObj.insert(KylinDaqCommon::gs_parentTidKey,sourceJsonObj[KylinDaqCommon::gs_parentTidKey].toString());
        }

        // 修改处：统一使用 s_uploadTimeKey
        QString nowStr = QDateTime::currentDateTime()
                             .toTimeZone(QTimeZone(KylinDaqCommon::gs_timezone))
                             .toString(KylinDaqCommon::gs_timeFormat);
        if(waitAddJsonObj.contains(s_uploadTimeKey)) {
            waitAddJsonObj[s_uploadTimeKey] = nowStr;
        }
        else
        {
            waitAddJsonObj.insert(s_uploadTimeKey, nowStr);
        }

        if(sourceJsonObj.contains(KylinDaqCommon::gs_stateOperatorKey)) 
        {
            waitAddJsonObj.insert(KylinDaqCommon::gs_stateOperatorKey,sourceJsonObj[KylinDaqCommon::gs_stateOperatorKey]);
        }
        mergeJsonObject.append(waitAddJsonObj);
    }
    if(mergeJsonObject.isEmpty())
    {
        return SendInfoStatus::FormatException;
    }
    sendedJsonObject.insert(s_messageKey,mergeJsonObject);
    requestData = QJsonDocument(sendedJsonObject).toJson(QJsonDocument::Compact);
    return SendInfoStatus::OK;
}

TransmitModule::SendInfoStatus TransmitModule::setRequest(QNetworkRequest &request,
                                                          const QUrl &url,
                                                          const QJsonObject &requestHeadObj)
{
    if (url.scheme() == "https") {
        QSslConfiguration config;
        config.setProtocol(QSsl::TlsV1SslV3);
        config.setPeerVerifyMode(QSslSocket::PeerVerifyMode::VerifyNone);
        request.setSslConfiguration(config);
    }

    for (auto iter = requestHeadObj.begin(); iter != requestHeadObj.end(); ++iter) {
        if (iter.value().isString()) {
            // 如需调试 HTTP 头，可设置环境变量 KYLIN_DAQ_DEBUG_HTTP_HEADER=1
            if (qEnvironmentVariableIsSet("KYLIN_DAQ_DEBUG_HTTP_HEADER")) {
                qDebug().noquote() << QString("TransmitModule::setRequest: header %1=%2")
                                      .arg(iter.key())
                                      .arg(iter.value().toString());
            }
            request.setRawHeader(iter.key().toUtf8(), iter.value().toString().toUtf8());
        }
    }

    // 上传时间戳
    request.setRawHeader(s_httpUploadTimeStamp,
                         QDateTime::currentDateTime()
                             .toTimeZone(QTimeZone(KylinDaqCommon::gs_timezone))
                             .toString(KylinDaqCommon::gs_timeFormat)
                             .toUtf8());

    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(s_requestTyepValue));
    request.setUrl(url);
    return SendInfoStatus::OK;
}

void TransmitModule::printSendInfoLog(const QList<QString> &info, const TransmitModule::SendInfoStatus &requestState, int32_t requestErrorCode, const QString &requestErrorRelyStr)
{
    switch (requestState)
    {
    case SendInfoStatus::OK:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestInfoNum = %1,requestState=%2").arg(info.size()).arg(QString::number(requestState));
        break;
    case SendInfoStatus::FormatException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestInfoNum = %1,requestState=%2").arg(info.size()).arg(QString::number(requestState));
        break;
    case SendInfoStatus::NetworkAnomaly:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestInfoNum = %1,requestState=%2, ErrorCode=%3").arg(info.size()).arg(QString::number(requestState),QString::number(requestErrorCode));
        break;
    case SendInfoStatus::ServerException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestInfoNum = %1,requestState=%2, ErrorCode=%3").arg(info.size()).arg(QString::number(requestState),QString::number(requestErrorCode) );
        break;
    case SendInfoStatus::ServerReplyException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestInfoNum = %1,requestState=%2, ErrorCode=%3, RelyStr=%4").arg(info.size()).arg(QString::number(requestState),QString::number(requestErrorCode) ,requestErrorRelyStr);
        break;
    case SendInfoStatus::UrlException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestInfoNum = %1,requestState=%2").arg(info.size()).arg(QString::number(requestState));
        break;
    default:
        break;
    }
}

void TransmitModule::printSendInfoLog(const int &requestId,const TransmitModule::SendInfoStatus &requestState, int32_t requestErrorCode, const QString &requestErrorRelyStr)
{
    switch (requestState)
    {
    case SendInfoStatus::OK:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestId = %1,requestState=%2").arg(requestId).arg(QString::number(requestState));
        break;
    case SendInfoStatus::FormatException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestId = %1,requestState=%2").arg(requestId).arg(QString::number(requestState));
        break;
    case SendInfoStatus::NetworkAnomaly:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestId = %1,requestState=%2, ErrorCode=%3").arg(requestId).arg(QString::number(requestState),QString::number(requestErrorCode));
        break;
    case SendInfoStatus::ServerException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestId = %1,requestState=%2, ErrorCode=%3").arg(requestId).arg(QString::number(requestState),QString::number(requestErrorCode) );
        break;
    case SendInfoStatus::ServerReplyException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestId = %1,requestState=%2, ErrorCode=%3, RelyStr=%4").arg(requestId).arg(QString::number(requestState),QString::number(requestErrorCode) ,requestErrorRelyStr);
        break;
    case SendInfoStatus::UrlException:
        qInfo().noquote() << QString("TransmitModule::sendInfo: requestId = %1,requestState=%2").arg(requestId).arg(QString::number(requestState));
        break;
    default:
        break;
    }
}

/* 定时信号触发数量 */
void TransmitModule::sendUploadInfoNum()
{
    qInfo() << "upload state = " << uploadState();
    if(uploadState() == KylinDaqCommon::UploadEnable)
    {
        if(m_uploadInfoBusy)
        {
              qInfo().noquote() << QString("sendUploadInfoNum: uploadInfo busy, lastRound=%1").arg(m_roundSeq);
        }
        else
        {
            emit uploadInfoSignal(m_numOfUploadedLines);
        }
        if(m_uploadEventBusy)
        {
              qInfo().noquote() << QString("sendUploadInfoNum: uploadEvent busy, lastRound=%1").arg(m_roundSeq);
        }
        else
        {
            emit uploadEventInfoSignal(m_numOfUploadedLines);
        }

    }
}

void TransmitModule::sendReuploadInfoNum()
{
    if(uploadState() == KylinDaqCommon::UploadEnable)
    {
        if(m_reuploadInfoBusy)
        {
              qInfo().noquote() << QString("sendReuploadInfoNum: reuploadInfo busy, lastRound=%1").arg(m_roundSeq);
        }
        else
        {
            emit reuploadInfoSignal(m_numOfReuploadedLines);
        }
        if(m_reuploadEventBusy)
        {
              qInfo().noquote() << QString("sendReuploadInfoNum: reuploadEvent busy, lastRound=%1").arg(m_roundSeq);
        }
        else
        {
            emit reuploadEventInfoSignal(m_numOfReuploadedLines);
        }


    }
}

void TransmitModule::sendStateInfoNum()
{
    if(uploadState() == KylinDaqCommon::UploadEnable && !m_stateInfoBusy)
    {
       emit uploadStateInfoSignal(m_numOfUploadedStateInfoLines);
    }else if(m_stateInfoBusy){
       qInfo().noquote() << QString("sendStateInfoNum skipped, stateInfo busy, lastRound=%1").arg(m_roundSeq);
    }
}

/* ------------------ 对外接口（异步） ------------------ */
void TransmitModule::uploadInfo(const QList<qlonglong> &id,
                                const QList<QString> &info,
                                const QList<QString> & status,
                                const QString &requestHead)
{
    qInfo() << "TransmitModule::uploadInfo start";
    if (uploadState() != KylinDaqCommon::UploadEnable)
        return;

    // 提前校验：空 info 直接回调错误，不进入 busy 状态
    if (info.isEmpty()) {
        emit retUploadState(id, info, status, FormatException);
        return;
    }
    // 如果 realm 未配置，也直接回调错误
    if (m_uploadRealm.isEmpty()) {
        emit retUploadState(id, info, status, UrlException);
        return;
    }

    if (m_uploadInfoBusy) {
        qInfo().noquote()
            << QString("TransmitModule::uploadInfo skipped, previous uploadInfo batch still running, lastRound=%1")
                   .arg(m_roundSeq);
        return;
    }
    m_uploadInfoBusy = true;
    startBatchUpload(id, info, status, requestHead, RK_BatchUpload);
}

void TransmitModule::reuploadInfo(const QList<qlonglong> &id,
                                  const QList<QString> &info,
                                  const QList<QString> & status,
                                  const QString &requestHead)
{
    qInfo() << "TransmitModule::reuploadInfo start";
    if (uploadState() != KylinDaqCommon::UploadEnable)
        return;

    // 提前校验
    if (info.isEmpty()) {
        emit retReuploadState(id, info, status, FormatException);
        return;
    }
    if (m_uploadRealm.isEmpty()) {
        emit retReuploadState(id, info, status, UrlException);
        return;
    }

    if (m_reuploadInfoBusy) {
        qInfo().noquote()
            << QString("TransmitModule::reuploadInfo skipped, previous reuploadInfo batch still running, lastRound=%1")
                   .arg(m_roundSeq);
        return;
    }
    m_reuploadInfoBusy = true;
    startBatchUpload(id, info, status, requestHead, RK_BatchReupload);
}


void TransmitModule::uploadEventInfo(const QList<qlonglong> &id,
                                     const QList<QString> &info,
                                     const QList<QString> & status,
                                     const QString &requestHead)
{
    qInfo() << "TransmitModule::uploadEventInfo start";
    if (uploadState() != KylinDaqCommon::UploadEnable)
        return;

    // 提前校验：空 / size 不一致 直接回调
    if (info.isEmpty() || id.size() != info.size()) {
        emit retEventUploadState(id, info, status, FormatException);
        return;
    }
    // realm 未配置直接回调
    if (m_uploadRealm.isEmpty()) {
        emit retEventUploadState(id, info, status, UrlException);
        return;
    }

    if (m_uploadEventBusy) {
        qInfo().noquote()
            << QString("TransmitModule::uploadEventInfo skipped, previous uploadEvent batch still running, lastRound=%1")
                   .arg(m_roundSeq);
        return;
    }
    m_uploadEventBusy = true;
    startEventBatch(id, info, status, requestHead, false);
}

void TransmitModule::reuploadEventInfo(const QList<qlonglong> &id,
                                       const QList<QString> &info,
                                       const QList<QString> & status,
                                       const QString &requestHead)
{
    qInfo() << "TransmitModule::reuploadEventInfo start";
    if (uploadState() != KylinDaqCommon::UploadEnable)
        return;

    // 提前校验
    if (info.isEmpty() || id.size() != info.size()) {
        emit retEventReuploadState(id, info, status, FormatException);
        return;
    }
    if (m_uploadRealm.isEmpty()) {
        emit retEventReuploadState(id, info, status, UrlException);
        return;
    }

    if (m_reuploadEventBusy) {
        qInfo().noquote()
            << QString("TransmitModule::reuploadEventInfo skipped, previous reuploadEvent batch still running, lastRound=%1")
                   .arg(m_roundSeq);
        return;
    }
    m_reuploadEventBusy = true;
    startEventBatch(id, info, status, requestHead, true);
}


void TransmitModule::uploadStateInfo(const QList<QString> &packageInfoList,
                                     const QList<QString> &uploadedMessageList,
                                     const QList<QString> &tidsList,
                                     const QList<QString> &createTimeStampList,
                                     const QList<KylinDaqCommon::UploadStateEnum> & uploadStateList,
                                     const QString &requestHead)
{
    qInfo() << "TransmitModule::uploadStateInfo start";
    if (uploadState() != KylinDaqCommon::UploadEnable)
        return;

    // realm 为空直接回调，避免 busy 卡死
    if (m_uploadRealm.isEmpty()) {
        emit retUploadStateInfoState(packageInfoList, UrlException);
        return;
    }

    if (m_stateInfoBusy) {
        qInfo().noquote()
            << QString("TransmitModule::uploadStateInfo skipped, previous stateInfo batch still running, lastRound=%1")
                   .arg(m_roundSeq);
        return;
    }
    m_stateInfoBusy = true;

    if (packageInfoList.isEmpty()
        || packageInfoList.size() != uploadedMessageList.size()
        || packageInfoList.size() != tidsList.size()
        || packageInfoList.size() != createTimeStampList.size()
        || packageInfoList.size() != uploadStateList.size())
    {
        emit retUploadStateInfoState(packageInfoList, FormatException);
        m_stateInfoBusy = false;
        return;
    }

    QList<QString> infoList;
    for (int i = 0; i < packageInfoList.size(); ++i) {
        QString info = createStateInfo(packageInfoList[i],
                                       uploadedMessageList[i],
                                       tidsList[i],
                                       createTimeStampList[i],
                                       uploadStateList[i]);
        if (!info.isEmpty())
            infoList.append(info);
    }
    if (infoList.isEmpty()) {
        emit retUploadStateInfoState(packageInfoList, FormatException);
        m_stateInfoBusy = false;
        return;
    }
    startBatchUpload(QList<qlonglong>(),
                     infoList,
                     QList<QString>(),
                     requestHead,
                     RK_BatchState,
                     packageInfoList);
}

void TransmitModule::getConfig()
{
    qInfo()<<"TransmitModule::getConfig start";
    startConfigFetch(QString());
}

/* ------------------ 定时器间隔 ------------------ */
void TransmitModule::setUploadTimerPollingInterval(qlonglong sec)
{
    uploadTimer->stop();
    uploadTimer->setInterval(KylinDaqCommon::secToMsec(sec));
    uploadTimer->start();
}

void TransmitModule::setReuploadTimerPollingInterval(qlonglong sec)
{
    reuploadTimer->stop();
    reuploadTimer->setInterval(KylinDaqCommon::secToMsec(sec));
    reuploadTimer->start();
}

void TransmitModule::setUploadStateInfoTimerPollingInterval(qlonglong sec)
{
    uploadStateInfoTimer->stop();
    uploadStateInfoTimer->setInterval(KylinDaqCommon::secToMsec(sec));
    uploadStateInfoTimer->start();
}

/* ------------------ 打印配置 ------------------ */
void TransmitModule::printConf()
{
    qDebug()<<"uploadInterval:"<< m_uploadInterval;
    qDebug()<<"reuploadInterval:"<< m_reuploadInterval;
    qDebug()<<"stateInfoUploadInterval:"<< m_uploadStateInfoInterval;
    qDebug()<<"numOfUploadedLines:"<< m_numOfUploadedLines;
    qDebug()<<"numOfReuploadedLines:"<<m_numOfReuploadedLines;
    qDebug()<<"numOfStateInfoUploadedLines:"<<m_numOfUploadedStateInfoLines;
    qDebug()<<"uploadState:"<<m_uploadState;
    qDebug()<<"uploadRealm:"<<m_uploadRealm;
    qDebug()<<"uploadRequestMaxTime:"<<m_uploadRequestMaxTime;
}

/* ------------------ 更新配置 ------------------ */
void TransmitModule::updateProperty(const QJsonObject &conf, const QJsonObject &defaultConf)
{
    qlonglong value;
    qlonglong uploadIntervalSum;
    QList<QString> tempStrList;
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_numOfUploadedLinesKey, value);
    setNumOfUploadedLines(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_numOfReuploadedLinesKey, value);
    setNumOfReuploadedLines(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_numOfStateInfoUploadedLinesKey, value);
    setNumOfUploadedStateInfoLines(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_uploadStateKey, value);
    setUploadState(KylinDaqCommon::UploadState(value));
    KylinDaqCommon::setValidConfStrArray(conf, defaultConf, moduleName(), s_uploadRealmKey, tempStrList);
    setUploadRealm(tempStrList);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_uploadRequestMaxTimeKey, value);
    setUploadRequestMaxTime(value);
    uploadIntervalSum = uploadRealm().size()*uploadRequestMaxTime()*2 + s_uploadIntervalThreshold; 
    KylinDaqCommon::setValidConfIntCmpMax(conf, defaultConf, moduleName(), s_uploadIntervalKey, uploadIntervalSum, value);
    setUploadInterval(value);
    KylinDaqCommon::setValidConfIntCmpMax(conf, defaultConf, moduleName(), s_reuploadIntervalKey, uploadIntervalSum, value);
    setReuploadInterval(value);
    KylinDaqCommon::setValidConfIntCmpMax(conf, defaultConf, moduleName(), s_stateInfoUploadIntervalKey, uploadIntervalSum, value);
    setUploadStateInfoInterval(value);
    KylinDaqCommon::setValidConfInt(conf, defaultConf, moduleName(), s_numOfQuantitativeUploadLinesKey, value);
    setNumOfQuantitativeUploadLines(value);
    printConf();
}

/* 判断网络是否连接 (保持原逻辑) */
bool TransmitModule::networkIsConnected()
{
    QDBusInterface *networkService =  new QDBusInterface("org.freedesktop.NetworkManager",
                                                         "/org/freedesktop/NetworkManager",
                                                         "org.freedesktop.NetworkManager",
                                                         QDBusConnection::systemBus());
    if(!networkService->isValid())
    {
        qWarning() << "网络Service Interface: " << qPrintable(QDBusConnection::systemBus().lastError().message());
        delete networkService;
        return false;
    }
    QDBusMessage message = networkService->call(QDBus::Block, "state");
    delete networkService;
    if (message.type() == QDBusMessage::ReplyMessage)
    {
        uint32_t value = message.arguments().takeFirst().toUInt();
        if (value == 70)
        {
            return true;
        }
        else
        {
            qWarning() << "网络状态接口返回值：" << value;
            return false;
        }
    }
    qWarning() << "获取网络状态接口返回值失败";
    return false;
}

/* 生成状态消息 (保持原实现) */
QString TransmitModule::createStateInfo(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState)
{
    QString info;
    QJsonObject infoJsonObj;
    QJsonObject sourceJsonObj;
    QJsonObject informationJsonObj;
    QJsonDocument packageInfoJsonDoc = QJsonDocument::fromJson(packageInfo.toUtf8());
    QJsonDocument uploadedMessageJsonDoc = QJsonDocument::fromJson(uploadedMessage.toUtf8());
    QJsonDocument tidsJsonDoc = QJsonDocument::fromJson(tids.toUtf8());
    QJsonDocument createTimeStampJsonDoc = QJsonDocument::fromJson(createTimeStamp.toUtf8());
    
    if (!packageInfoJsonDoc.isObject()
        ||!uploadedMessageJsonDoc.isObject()
        ||!tidsJsonDoc.isObject()
        ||!createTimeStampJsonDoc.isObject()
    )
    {
        return info;
    }
    QJsonObject packageInfoJsonObj = packageInfoJsonDoc.object();
    QJsonObject uploadedMessageJsonObj = uploadedMessageJsonDoc.object();
    QJsonObject tidsJsonObj = tidsJsonDoc.object();
    QJsonObject createTimeStampJsonObj = createTimeStampJsonDoc.object();

    sourceJsonObj = packageInfoJsonObj;
    informationJsonObj = uploadedMessageJsonObj;

    if (!packageInfoJsonObj.contains(KylinDaqCommon::gs_packageNameKey)
        ||!packageInfoJsonObj.contains(KylinDaqCommon::gs_messageTypeKey)
        ||!tidsJsonObj.contains(KylinDaqCommon::gs_tidKey)
        ||!createTimeStampJsonObj.contains(KylinDaqCommon::gs_createTimeStampKey))
    {
        return info;
    }
    if (uploadState != KylinDaqCommon::UploadStateEnum::UploadStateMessageCreated
        && uploadState != KylinDaqCommon::UploadStateEnum::UploadStateMessageUpdated)
    {
        return info;
    }

    sourceJsonObj[KylinDaqCommon::gs_tidKey]=tidsJsonObj[KylinDaqCommon::gs_tidKey];
    sourceJsonObj[KylinDaqCommon::gs_parentTidKey]=tidsJsonObj[KylinDaqCommon::gs_parentTidKey];
    informationJsonObj[KylinDaqCommon::gs_createTimeStampKey] = createTimeStampJsonObj[KylinDaqCommon::gs_createTimeStampKey];
    if (uploadState == KylinDaqCommon::UploadStateEnum::UploadStateMessageCreated)
    {
        sourceJsonObj[KylinDaqCommon::gs_stateOperatorKey] = KylinDaqCommon::gs_stateOperatorKeyAddValue;
    }
    else if (uploadState == KylinDaqCommon::UploadStateEnum::UploadStateMessageUpdated)
    {
        sourceJsonObj[KylinDaqCommon::gs_stateOperatorKey] = KylinDaqCommon::gs_stateOperatorKeyUpdateValue;
    }
    infoJsonObj[KylinDaqCommon::gs_sourceKey] = sourceJsonObj;
    infoJsonObj[KylinDaqCommon::gs_informationKey] = informationJsonObj;
    info = QString(QJsonDocument(infoJsonObj).toJson(QJsonDocument::Compact));
    return info; 
}
