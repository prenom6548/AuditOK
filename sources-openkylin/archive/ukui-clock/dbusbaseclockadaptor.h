/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef DBUSBASECLOCKADAPTOR_H
#define DBUSBASECLOCKADAPTOR_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include "ClockInterface.h"
#include "CJsonObject.hpp"
#include "fieldvalidutil.h"

class DbusBaseClockAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
public:
    explicit DbusBaseClockAdaptor(QObject *parent = nullptr);

signals:
public:
    QString formatReturnMsg(neb::CJsonObject oJson,clockInterface::STATUS_CODE status,std::string msg);
    QString formatReturnMsg(clockInterface::STATUS_CODE status,std::string msg);
};

#endif // DBUSBASECLOCKADAPTOR_H
