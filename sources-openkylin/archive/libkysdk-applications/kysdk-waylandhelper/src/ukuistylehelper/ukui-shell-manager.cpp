/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "ukui-shell-manager.h"
#include "ukui-shell-client-protocol.h"
#include "ukuiwaylandinterface.h"
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/surface.h>
#include <QApplication>
#include <QDebug>

static UkuiShellManager *global_instance = nullptr;
static UkuiWaylandInterface *g_ukuiwaylandInterface = nullptr;

UkuiShellManager::UkuiShellManager()
{
    if (!g_ukuiwaylandInterface)
        g_ukuiwaylandInterface = new UkuiWaylandInterface;
}

UkuiShellManager *UkuiShellManager::getInstance()
{
    if (!global_instance)
        global_instance = new UkuiShellManager;
    return global_instance;
}

bool UkuiShellManager::removeHeaderBar(QWindow *windowHandle)
{
    if (!g_ukuiwaylandInterface->ukuiProtocolReady())
        return false;
    return g_ukuiwaylandInterface->removeHeaderBar(windowHandle);
}

bool UkuiShellManager::ukuishellReady()
{
    return g_ukuiwaylandInterface->ukuiProtocolReady();
}
