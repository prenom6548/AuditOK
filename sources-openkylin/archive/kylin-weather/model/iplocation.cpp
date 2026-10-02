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
#include "iplocation.h"
const QString codeSecond = QString("YzljMDcwODc5YmViZTYxNQ=="); //第二段加密内容
Iplocation::Iplocation(QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true); //设置定时器单例--必须设置
    m_timerGeo.setSingleShot(true);
    m_timerIPIP.setSingleShot(true);
    //超时逻辑
    //使用 QTimer 启动一个单次定时器，并设置超时时间。
    //在事件循环退出之后，判断定时器的状态，如果是激活状态，证明请求已经完成；否则，说明超时
}
const QString Iplocation::getCodeThird()
{
    QString codeThird = "";
    codeThird = CoreVar::getSettings("ipkeypart").toString();
    return codeThird;
}
QString Iplocation::gotoLocation()
{
    //{"status":"0","info":"USER_DAILY_QUERY_OVER_LIMIT","infocode":"10044"}
    //高德api定位逻辑
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    //获得ip
    QString ip_str = pconlineLocation(CoreVar::PCONLINEURL);

    QString str = CoreVar::URLHEAD + decodeToOrigin(codeFirst, codeSecond, getCodeThird());
    //获得完整url
    QString url_str = str.append(ip_str);
    //获得返回数据：地址--精确到区
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(url_str)));
    //返回数据解析
    QString responeData;
    //事件循环-做超时处理
    QEventLoop eventLoop;
    //定时器过期之后或者网络响应完成后事件循环得到退出，不至于一直处于阻塞状态
    connect(&m_timer, SIGNAL(timeout()), &eventLoop, SLOT(quit())); //定时器结束，循环断开
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    m_timer.start(CoreVar::DEFAULT_TIMEOUT); // 30s等待
    eventLoop.exec();
    //超时处理
    if (!timeOut(&m_timer, reply, &eventLoop)) {
        return "";
    }
    // ipip定位
    if (reply == nullptr) {
        return ipipLocation();
    }
    responeData = reply->readAll();
    //数据解析
    // 创建 QJsonParseError 对象，用来获取解析结果
    QJsonParseError parseJsonError;
    //使用静态函数获取 QJsonDocument 对象  读写json文档
    QJsonDocument jsonDocument = QJsonDocument::fromJson(responeData.toUtf8(), &parseJsonError);
    if (parseJsonError.error != QJsonParseError::NoError) {
        return ipipLocation();
    }
    //将数据转换为QString 返回jsonDocument中的json对象。
    QJsonObject jsonObject = jsonDocument.object();
    if (jsonObject["status"].toString() == "1" && jsonObject["status"].toString() != "") {
        return jsonObject["district"].toString();
    } else {
        return ipipLocation();
    }
}
QString Iplocation::decodeToOrigin(const QString &codeStr1, const QString &codeStr2, const QString &codeStr3)
{
    QString finalStr = "";
    QByteArray text1 = codeStr1.toLocal8Bit();
    QByteArray by1 = text1.fromBase64(text1);
    QString str1 = QString::fromLocal8Bit(by1);

    QByteArray text2 = codeStr2.toLocal8Bit();
    QByteArray by2 = text2.fromBase64(text2);
    QString str2 = QString::fromLocal8Bit(by2);

    QByteArray text3 = codeStr3.toLocal8Bit();
    QByteArray by3 = text3.fromBase64(text3);
    QString str3 = QString::fromLocal8Bit(by3);

    finalStr.append(str1);
    finalStr.append(str2);
    finalStr.append(str3);

    return finalStr;
}
//高德定位
QString Iplocation::pconlineLocation(const QString &url)
{

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    connect(&m_timerGeo, SIGNAL(timeout()), &loop, SLOT(quit())); //定时器结束，循环断开
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    m_timerGeo.start(CoreVar::DEFAULT_TIMEOUT); // 30s等待
    loop.exec();
    //超时处理
    if (!timeOut(&m_timerGeo, reply, &loop)) {
        return "";
    }
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray ba = reply->readAll();
        QString reply_content = QString::fromUtf8(ba);
        reply->close();
        reply->deleteLater();
        manager->deleteLater();
        if (!reply_content.isEmpty()) {
            QString htmlStr = reply_content.replace(" ", "");
            htmlStr = htmlStr.replace("\r", "");
            htmlStr = htmlStr.replace("\n", "");
            QStringList htmlList = htmlStr.split("<br/>");
            if (htmlList.size() >= 4) {
                QStringList ipList = htmlList.at(4).split("=");
                if (ipList.count() > 1) {
                    if (ipList.at(1).contains(",")) {
                        ipList = ipList.at(1).split(",");
                        return ipList.at(0);
                    }
                    return ipList.at(1);
                }
            }
        }
    }
    //无法定位，返回""
    return "";
}
// ipip定位
QString Iplocation::ipipLocation()
{
    //使用ipip.net的服务进行定位  "当前 IP：123.150.8.43  来自于：中国 天津 天津  电信\n"
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(CoreVar::IPIP)));
    QByteArray responseData;
    QEventLoop eventLoop;
    connect(&m_timerIPIP, SIGNAL(timeout()), &eventLoop, SLOT(quit())); //定时器结束，循环断开
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    m_timerIPIP.start(CoreVar::DEFAULT_TIMEOUT); // 30s等待
    eventLoop.exec();
    //超时处理
    if (!timeOut(&m_timerIPIP, reply, &eventLoop)) {
        return "";
    }
    responseData = reply->readAll();
    QString str = responseData;

    QStringList respList = str.split(' ');
    if (!(respList.size() >= 5))
        return "";
    return respList[5];
}
//网络超时处理
bool Iplocation::timeOut(QTimer *timer, QNetworkReply *reply, QEventLoop *eventloop)
{
    //超时处理
    if (timer->isActive()) {
        //在设定的时间内没有超时，定时器关闭
        timer->stop();
        return true;
    } else {
        //超时-断开访问连接
        disconnect(reply, &QNetworkReply::finished, eventloop, &QEventLoop::quit);
        reply->abort();
        reply->deleteLater();
        return false;
    }
}
