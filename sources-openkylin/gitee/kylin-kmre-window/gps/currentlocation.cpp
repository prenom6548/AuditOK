/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "currentlocation.h"
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QJsonArray>
#include "formatconversion.h"
namespace
{
QString IPURL = "http://whois.pconline.com.cn/";
QString IPLOCATIONURL = "https://restapi.amap.com/v5/ip?type=4&key=";
QString STATUSOK = "1";
} // namespace

using namespace kmre_dataformat;

namespace kmre_gps
{
CurrentLocation::CurrentLocation(QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timerIp.setSingleShot(true);
}

bool CurrentLocation::getLocation()
{
    QNetworkAccessManager manager;
    QString ip = ipLocation(IPURL);
    QString key = FormatConversion::decodeToOrigin();
    QString locationUrl = IPLOCATIONURL + key + "&ip=" + ip;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(locationUrl)));
    QEventLoop eventLoop;
    connect(&m_timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    m_timer.start(10000);
    eventLoop.exec();
    //超时处理
    if (!isTimeout(&m_timer, reply, &eventLoop)) {
        QString responeData = reply->readAll();
        //数据解析
        QJsonParseError parseJsonError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(responeData.toUtf8(), &parseJsonError);
        //将数据转换为QString 返回jsonDocument中的json对象。
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject["status"].toString() == STATUSOK) {
            m_currentlocation.province = jsonObject["province"].toString();
            m_currentlocation.cityName = jsonObject["city"].toString();
            m_currentlocation.adName = jsonObject["district"].toString();
            QString keywords = m_currentlocation.cityName + m_currentlocation.adName;
            if (LocationGet::getInstance()->getLocation(keywords)) {
                m_currentlocation.location = LocationGet::getInstance()->m_locationArray[0].location;
                reply->close();
                reply->deleteLater();
                return true;
            }
        }
    }
    reply->close();
    reply->deleteLater();
    return false;
}

//高德定位
QString CurrentLocation::ipLocation(const QString &url)
{
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop loop;
    connect(&m_timerIp, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    m_timerIp.start(10000); // 10s等待
    loop.exec();
    //超时处理
    if (!isTimeout(&m_timerIp, reply, &loop)) {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray buffer = reply->readAll();
            QString reply_content = QString::fromUtf8(buffer);
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
                            reply->close();
                            reply->deleteLater();
                            return ipList.at(0);
                        }
                        reply->close();
                        reply->deleteLater();
                        return ipList.at(1);
                    }
                }
            }
        }
    }
    reply->close();
    reply->deleteLater();
    return "";
}

//网络超时处理
bool CurrentLocation::isTimeout(QTimer *timer, QNetworkReply *reply, QEventLoop *eventLoop)
{
    //超时处理
    if (timer->isActive()) {
        timer->stop();
        return false;
    } else {
        disconnect(reply, &QNetworkReply::finished, eventLoop, &QEventLoop::quit);
        reply->abort();
        reply->deleteLater();
        return true;
    }
}

CurrentLocation::~CurrentLocation() {}

} // namespace kmre_gps
