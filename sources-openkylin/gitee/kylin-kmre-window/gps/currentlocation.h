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

#ifndef CURRENTLOCATION_H
#define CURRENTLOCATION_H
#include <QObject>
#include <QString>
#include <QTimer>
#include <QJsonObject>
#include <QEventLoop>
#include <QNetworkReply>
#include "gpsglobal.h"
#include "locationget.h"

namespace kmre_gps
{
class CurrentLocation : public QObject
{
    Q_OBJECT
public:
    CurrentLocation(QObject *parent = nullptr);
    ~CurrentLocation();
    ItemData m_currentlocation;
    bool getLocation();

private:
    QString ipLocation(const QString &url);
    bool isTimeout(QTimer *timer, QNetworkReply *reply, QEventLoop *eventLoop);
    QTimer m_timer;
    QTimer m_timerIp;
};

} // namespace kmre_gps
#endif // IPLOCATION_H
