/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MPRISPLAYERMANAGER_H
#define MPRISPLAYERMANAGER_H

#include <QObject>
#include "mpris_player.h"
using namespace Mpris;
class MprisPlayerManagerPrivate;
class MprisPlayerManager : public QObject
{
    Q_OBJECT
public:
    explicit MprisPlayerManager(QObject *parent = nullptr);
    ~MprisPlayerManager();
    MprisPlayerWeakPtr player(const QString &service) const;
    MprisPlayerWeakPtr player(const uint &pid) const;

    MprisPlayerWeakPtr activePlayer();

private:
    MprisPlayerManagerPrivate* d = nullptr;
};

#endif // MPRISPLAYERMANAGER_H
