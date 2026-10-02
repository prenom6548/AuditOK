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
#include "mpris_controller.h"
#include "mpris_controller_adaptor.h"
#include "mpris_player_control.h"
#define CPLUSPLUS14 (__cplusplus >= 201402L)
const static QString s_mprisService = QStringLiteral("org.ukui.mpris.controller");
MprisController::MprisController(QObject *parent) : QObject(parent)
{
    init();
}

const QStringList MprisController::supportOperations() const
{
    return m_supportOperations;
}

#if CPLUSPLUS14
using std::make_unique;
#else
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

bool MprisController::init()
{
    new MprisControllerAdaptor(this);
    QDBusConnection con = QDBusConnection::sessionBus();

//    if (!con.registerService(s_mprisService))
//    {
//        qWarning() << "Could not register service!";
//        return false;
//    }

    if (!con.registerObject(QStringLiteral("/mpris/controller"), this))
    {
        qWarning() <<"Could not register Chat object!";
        return false;
    }
    m_mprisPlayerManager = make_unique<MprisPlayerManager>(nullptr);
    return true;
}

void MprisController::operation(const QString &operation, const QVariantList &arguments)
{
    if (m_supportOperations.contains(operation)) {
        std::unique_ptr<MprisPlayerControl> playerControl = make_unique<MprisPlayerControl>(m_mprisPlayerManager->activePlayer());
        playerControl->operation(operation, arguments);
    } else {
        qWarning() <<"is not support operation :" << operation;
    }
}
