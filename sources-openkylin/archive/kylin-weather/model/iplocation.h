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
#ifndef IPLOCATION_H
#define IPLOCATION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonParseError>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>
#include "global/corevar.h"
class Iplocation : public QObject
{
    Q_OBJECT
    friend class TestIplocation;
public:
    explicit Iplocation(QObject *parent = nullptr);
    QString gotoLocation(); //去定位
private:
    QString pconlineLocation(const QString &url); //高德定位
    QString ipipLocation();                       // ipip定位
    QTimer m_timer;                               //超时处理--定位
    QTimer m_timerGeo;                            //超时处理--高德
    QTimer m_timerIPIP;                           //超时处理--ipip
    bool timeOut(QTimer *timer, QNetworkReply *reply, QEventLoop *eventloop);
    const QString codeFirst = QString("aXA/a2V5PWZhZjhmYWFkNA=="); //第一段加密内容
    QString decodeToOrigin(const QString &codeStr1, const QString &codeStr2, const QString &codeStr3); //解密
    const QString getCodeThird();
signals:
};

#endif // IPLOCATION_H
