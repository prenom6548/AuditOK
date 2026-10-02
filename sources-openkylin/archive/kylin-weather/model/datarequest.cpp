/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "datarequest.h"

DataRequest::DataRequest(QObject *parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_timer.setSingleShot(true);
    m_timerLAndR.setSingleShot(true);
    // m_timerR.setSingleShot(true);
    m_timerColl.setSingleShot(true);
}
DataRequest::~DataRequest()
{
    m_networkManager->deleteLater();
    m_reply->deleteLater();
    m_replyCollect->deleteLater();
    m_replyLAndR->deleteLater();
}

//利用连接请求网络数据
void DataRequest::dataRequest(const QString &cityId)
{
    qDebug() << " ##### Function Positioning ##### 利用连接请求网络数据 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    if (cityId.isEmpty()) {
        return;
    }
    QString forecastUrl = CoreVar::SERVICEREQUEST.arg(cityId);
    QNetworkRequest request;
    request.setUrl(forecastUrl);
    m_reply = m_networkManager->get(request);
    QEventLoop eventLoop;
    connect(&m_timer, SIGNAL(timeout()), &eventLoop, SLOT(quit())); //定时器结束，循环断开
    connect(m_reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    m_timer.start(CoreVar::DEFAULT_TIMEOUT); // 30s等待
    eventLoop.exec();
    //超时处理
    if (!timeOut(&m_timer, m_reply, &eventLoop)) {
        qDebug() << " ##### Function Positioning ##### 超时处理 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        ErrorReact::getInstance()->networkFeedBack(false,"time out"); //超时
        return;
    } else {
        //数据处理
        dataReply();
    }
}

//处理返回的网络数据
void DataRequest::dataReply()
{
    qDebug() << " ##### Function Positioning ##### 处理返回的网络数据 ##### " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //状态码不为200时，异常
    if (statusCode != 200) {
        ErrorReact::getInstance()->networkFeedBack(false, QString::number(statusCode));
        return;
    }
    //网络层错误[与目标服务器有关]
    if (m_reply->error() != QNetworkReply::NoError) {
        ErrorReact::getInstance()->networkFeedBack(false, m_reply->errorString());
        return;
    }
    //一切正常，进行数据解析
    QByteArray dataBa = m_reply->readAll();
    m_reply->abort();
    emit sendData(dataBa);
}

//请求收藏城市数据
void DataRequest::dataRequestCollect(const QString &cityIdList)
{
    if (cityIdList.isEmpty()) {
        return;
    }
    //如果是搜藏城市列表加载城市
    QString forecastUrl = CoreVar::SERVICEREQUEST_COLLECT;
    QStringList cityList = cityIdList.split(","); // cityList最后一项为空字符
    //访问要求为 : "11111"+ "22222"形式
    for (int i = 0; i < cityList.size(); i++) {
        if (i == cityList.size() - 1) {
            forecastUrl.append(cityList.at(i));
        } else {
            forecastUrl.append(cityList.at(i));
            forecastUrl.append("+");
        }
    }

    QNetworkRequest request;
    request.setUrl(forecastUrl);
    m_replyCollect = m_networkManager->get(request);
    QEventLoop eventLoop;
    connect(&m_timerColl, SIGNAL(timeout()), &eventLoop, SLOT(quit())); //定时器结束，循环断开
    connect(m_replyCollect, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    m_timerColl.start(CoreVar::DEFAULT_TIMEOUT); // 30s等待
    eventLoop.exec();
    //超时处理
    if (!timeOut(&m_timerColl, m_replyCollect, &eventLoop)) {
        ErrorReact::getInstance()->networkFeedBack(false,"time out"); //超时
        return;
    } else {
        //数据处理
        dataReplyCollect();
    }
}
//处理收藏城市数据返回
void DataRequest::dataReplyCollect()
{
    int statusCode = m_replyCollect->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //状态码不为200时，异常
    if (statusCode != 200) {
        ErrorReact::getInstance()->networkFeedBack(false, QString::number(statusCode));
        return;
    }
    //网络层错误[与目标服务器有关]
    if (m_replyCollect->error() != QNetworkReply::NoError) {
        ErrorReact::getInstance()->networkFeedBack(false, m_replyCollect->errorString());
        return;
    }
    QByteArray dataBaC = m_replyCollect->readAll();
    m_replyCollect->abort();
    emit sendCollData(dataBaC);
}

bool DataRequest::timeOut(QTimer *timer, QNetworkReply *reply, QEventLoop *eventloop)
{
    //超时处理
    if (timer->isActive()) {
        //在设定的时间内没有超时，定时器关闭
        timer->stop();
        return true;
    } else {
        if (reply == nullptr) {
            return false;
        }
        //超时-断开访问连接
        disconnect(reply, &QNetworkReply::finished, eventloop, &QEventLoop::quit);
        reply->abort();
        return false;
    }
}
