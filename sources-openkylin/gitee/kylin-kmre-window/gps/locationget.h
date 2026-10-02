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

#ifndef LOCATIONGET_H
#define LOCATIONGET_H
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QJsonObject>

#include <QEventLoop>
#include <QPersistentModelIndex>
#include "gpsglobal.h"

namespace kmre_gps
{
class LocationGet : public QObject
{
public:
    static LocationGet *getInstance();
    ~LocationGet();
    bool getLocation(QString keywords);
    ItemData m_locationArray[5];

private:
    LocationGet(QObject *parent = nullptr);
    QTimer m_timer;
    bool isTimeout(QTimer *timer, QNetworkReply *reply, QEventLoop *eventloop);
};

} // namespace kmre_gps
#endif // LOCATIONGET_H
