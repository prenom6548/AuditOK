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
#ifndef MPRISCONTROLLER_H
#define MPRISCONTROLLER_H

#include <QObject>
#include <mpris_player_manager.h>
class MprisController : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.ukui.mpris.controller")
    Q_PROPERTY( QStringList supportOperations READ supportOperations)

public:
    explicit MprisController(QObject *parent = nullptr);
    bool init();
    const QStringList supportOperations() const;
private:
public Q_SLOTS:
    void operation(const QString& operation, const QVariantList &arguments);
private:
    std::unique_ptr<MprisPlayerManager> m_mprisPlayerManager;

    //通过supportOperations 查询当前支持哪些操作
    const QStringList m_supportOperations =
    {"raise", "quit", "next", "previous", "pause",
    "playPause", "stop", "play", "seek", "setPosition",
    "openUri", "setLoopStatus", "setShuffle", "setRate",
    "setVolume"};
};

#endif // MPRISCONTROLLER_H
