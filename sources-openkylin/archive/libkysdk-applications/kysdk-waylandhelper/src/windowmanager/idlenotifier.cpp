/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "idlenotifier.h"

using namespace kdk;

IdleNotifier::IdleNotifier(QObject *parent)
    : QObject(parent)
{
    m_wm = new WmRegister(this);
    connect(m_wm->winInterface(), &AbstractInterface::idled, this, &IdleNotifier::idled);
    connect(m_wm->winInterface(), &AbstractInterface::resumed, this, &IdleNotifier::resumed);
}

void IdleNotifier::setIdleInterval(int msec)
{
    return m_wm->winInterface()->setIdleInterval(msec);
}
