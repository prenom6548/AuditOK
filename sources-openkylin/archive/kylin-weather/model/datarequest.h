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
#ifndef DATAREQUEST_H
#define DATAREQUEST_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QByteArray>
#include <QTimer>
#include <QEventLoop>
#include <QJsonParseError>
#include "controller/core/errorreact.h"
#include "global/corevar.h"
class DataRequest : public QObject
{
    Q_OBJECT
public:
    explicit DataRequest(QObject *parent = nullptr);
    ~DataRequest();
    void dataRequest(const QString &cityId);            //主界面城市数据请求
    void dataRequestCollect(const QString &cityIdList); //收藏城市数据请求
private:
    QNetworkAccessManager *m_networkManager = nullptr;
    void dataReply();        //主界面城市网络返回
    void dataReplyCollect(); //收藏界面网络返回
    bool m_requsetWay = false; //默认为false，请求数据的方式，true代表是从搜索列表请求，false为其他
    QTimer m_timer;            //当前城市的定时器
    QTimer m_timerLAndR;       //左侧城市的定时器
    QTimer m_timerColl;        //收藏的定时器
    QNetworkReply *m_reply = nullptr;                                         //主界面城市网络reply
    QNetworkReply *m_replyLAndR = nullptr;                                        //主界面左边城市网络reply
    QNetworkReply *m_replyCollect = nullptr;                                  //收藏城市界面网络reply
    bool timeOut(QTimer *timer, QNetworkReply *reply, QEventLoop *eventloop); //主界面超时函数
signals:
    void sendData(QByteArray dataBa);     //主界面城市数据
    void sendCollData(QByteArray dataBa); //收藏城市数据
};

#endif // DATAREQUEST_H
