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
#include "mpris_player_control.h"
#include <QMetaObject>
MprisPlayerControl::MprisPlayerControl(MprisPlayerWeakPtr player, QObject *parent)
    : QObject(parent)
    , m_player(player)
{

}

void MprisPlayerControl::operation(const QString &operation, const QVariantList &arguments)
{
    if (m_player.expired()) {
        qWarning() << "pointer player reference count is 0 .";
        return;
    }

    if (arguments.count() > 0) {
        QMetaObject::invokeMethod(m_player.lock().get(), operation.toLatin1().data(), Qt::QueuedConnection,
                                  QGenericArgument(arguments.at(0).typeName(), arguments.at(0).constData()));

    } else {
        QMetaObject::invokeMethod(m_player.lock().get(), operation.toLatin1().data());
    }
}
