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
#ifndef MPRISPLAYERCONTROL_H
#define MPRISPLAYERCONTROL_H

#include <QObject>
#include "mpris_player_manager.h"
class MprisPlayerControl : public QObject
{
    Q_OBJECT
public:
    explicit MprisPlayerControl(MprisPlayerWeakPtr m_player, QObject *parent = nullptr);
public:
    void operation(const QString &operation, const QVariantList &arguments);
private:
    MprisPlayerWeakPtr m_player;
};

#endif // MPRISPLAYERCONTROL_H
