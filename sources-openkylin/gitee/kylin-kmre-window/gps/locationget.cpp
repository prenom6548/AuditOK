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

#include "locationget.h"
#include <QString>
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QJsonArray>
#include "formatconversion.h"
using namespace kmre_dataformat;
namespace kmre_gps
{
QString STATUSOK = "1";
const QString LOCATIONURL = "http://restapi.amap.com/v5/place/text?key=";

LocationGet *LocationGet::getInstance()
{
    static LocationGet instance;
    return &instance;
}

LocationGet::LocationGet(QObject *parent) : QObject(parent)
{
    m_timer.setSingleShot(true); //设置定时器单例--必须设置
}

//定位
bool LocationGet::getLocation(QString keywords)
{
    QNetworkAccessManager manager;
    QString key = FormatConversion::decodeToOrigin();
    QString url = LOCATIONURL + key + "&keywords=" + keywords;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop eventLoop;
    connect(&m_timer, SIGNAL(timeout()), &eventLoop, SLOT(quit())); //定时器结束，循环断开
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    m_timer.start(10000); // 10s等待
    eventLoop.exec();
    //超时处理
    if (!isTimeout(&m_timer, reply, &eventLoop)) {
        //使用静态函数获取 QJsonDocument 对象  读写json文档
        QJsonParseError parseJsonError;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll(), &parseJsonError);
        //将数据转换为QString 返回jsonDocument中的json对象。
        QJsonObject jsonObject = jsonDocument.object();
        if (jsonObject["status"].toString() == STATUSOK) {
            QStringList list = jsonObject.keys();
            foreach (QString key, list) {
                if (jsonObject[key].isArray() && key == "pois") { //判断条件互换
                    QJsonArray arr = jsonObject[key].toArray();
                    for (int i = 0; i < 5; i++) {
                        QJsonObject arrObj = arr[i].toObject();
                        m_locationArray[i].province = arrObj["pname"].toString();
                        m_locationArray[i].cityName = arrObj["cityname"].toString();
                        m_locationArray[i].adName = arrObj["adname"].toString();
                        m_locationArray[i].address = arrObj["address"].toString();
                        m_locationArray[i].name = arrObj["name"].toString();
                        m_locationArray[i].location = arrObj["location"].toString();
                    }
                }
            }
            reply->close();
            reply->deleteLater();
            return true;
        }
    }
    reply->close();
    reply->deleteLater();
    return false;
}

//网络超时处理
bool LocationGet::isTimeout(QTimer *timer, QNetworkReply *reply, QEventLoop *eventloop)
{
    //超时处理
    if (timer->isActive()) {
        //在设定的时间内没有超时，定时器关闭
        timer->stop();
        return false;
    } else {
        //超时-断开访问连接
        disconnect(reply, &QNetworkReply::finished, eventloop, &QEventLoop::quit);
        reply->abort();
        reply->deleteLater();
        return true;
    }
}

LocationGet::~LocationGet() {}

} // namespace kmre_gps
