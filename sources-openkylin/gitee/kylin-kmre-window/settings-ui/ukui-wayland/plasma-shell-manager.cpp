/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "plasma-shell-manager.h"

#include <QApplication>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>

#include <KWayland/Client/surface.h>

static PlasmaShellManager* global_instance = nullptr;

PlasmaShellManager *PlasmaShellManager::getInstance()
{
    if (!global_instance)
        global_instance = new PlasmaShellManager;
    return global_instance;
}

bool PlasmaShellManager::setAppWindowActive()
{
    if (!supportPlasmaWindowManagement())
        return false;

    m_appWindow->requestActivate();
    return true;
}

bool PlasmaShellManager::setAppWindowKeepAbove(bool keep)
{
    if (!supportPlasmaWindowManagement())
        return false;

    if(keep != m_appWindow->isKeepAbove()) {
        m_appWindow->requestToggleKeepAbove();
    }
    return true;
}

bool PlasmaShellManager::setMaximized(QWindow *window)
{
    if (!supportShell())
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto shellSurface = m_shell->createSurface(surface, window);
    if (!shellSurface)
        return false;

    shellSurface->setMaximized();
    return true;
}

bool PlasmaShellManager::setRole(QWindow *window, KWayland::Client::PlasmaShellSurface::Role role)
{
    if (!supportPlasmaShell())
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto plasmaShellSurface = m_plasmaShell->createSurface(surface, window);
    if (!plasmaShellSurface)
        return false;

    plasmaShellSurface->setRole(role);
    return true;
}

bool PlasmaShellManager::setPos(QWindow *window, const QPoint &pos)
{
    if (!supportPlasmaShell())
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;

    auto plasmaShellSurface = m_plasmaShell->createSurface(surface, window);
    if (!plasmaShellSurface)
        return false;

    plasmaShellSurface->setPosition(pos);
    return true;
}

bool PlasmaShellManager::supportPlasmaShell()
{
    return m_plasmaShell;
}

bool PlasmaShellManager::supportShell()
{
    return m_shell;
}

bool PlasmaShellManager::supportPlasmaWindowManagement()
{
    return m_windowManager && m_appWindow;
}

PlasmaShellManager::PlasmaShellManager(QObject *parent) : QObject(parent)
{
    auto connection = KWayland::Client::ConnectionThread::fromApplication(qApp);
    auto registry = new KWayland::Client::Registry(this);
    registry->create(connection->display());

    connect(registry, &KWayland::Client::Registry::plasmaShellAnnounced, this, [=](){
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);
        if (interface.name != 0) {
            m_plasmaShell = registry->createPlasmaShell(interface.name, interface.version, this);
        }
    });

    connect(registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced, this, [=](){
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::PlasmaWindowManagement);
        if (interface.name != 0) {
            m_windowManager = registry->createPlasmaWindowManagement(interface.name, interface.version, this);
        }
        if(m_windowManager) {
            connect(m_windowManager, &KWayland::Client::PlasmaWindowManagement::windowCreated,
                [this](KWayland::Client::PlasmaWindow *window) {
               if(window->appId() == QApplication::applicationName()){
                   if(isFirstCreate) {
                       isFirstCreate = false;
                       m_appWindow = window;
                   }
               }
            });
        }
    });

    connect(registry, &KWayland::Client::Registry::shellAnnounced, this, [=](){
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::Shell);
        if (interface.name != 0) {
            m_shell = registry->createShell(interface.name, interface.version, this);
        }
    });

    registry->setup();
    connection->roundtrip();
}
