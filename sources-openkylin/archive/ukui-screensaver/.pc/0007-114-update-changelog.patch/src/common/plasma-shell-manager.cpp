/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "plasma-shell-manager.h"

#include <QApplication>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>

#include <KWayland/Client/surface.h>
#include <unistd.h>
#include <QDebug>

static PlasmaShellManager *global_instance = nullptr;

PlasmaShellManager *PlasmaShellManager::getInstance()
{
    if (!global_instance) {
        global_instance = new PlasmaShellManager;
        qDebug() << "Here create instance...";
    }
    qDebug() << "Return instance";
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
    if (!supportPlasmaWindowManagement()) {
        qDebug() << "false";
        return false;
    }
    if (keep != m_appWindow->isKeepAbove()) {
        qDebug() << "to keep above";
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

bool PlasmaShellManager::supportFakeInput()
{
    return m_fakeInput;
}

bool PlasmaShellManager::supportKeyState()
{
    return m_keyState;
}

KWayland::Client::Keystate::State PlasmaShellManager::getKeyState(KWayland::Client::Keystate::Key key)
{
    if (!supportKeyState()) {
        return KWayland::Client::Keystate::Unlocked;
    }

    return m_keyStateMap[key];
}

void PlasmaShellManager::setKeyPressed(quint32 key)
{
    if (!supportFakeInput())
        return;

    m_fakeInput->requestKeyboardKeyPress(key);
    m_fakeInput->requestKeyboardKeyRelease(key);
}

PlasmaShellManager::PlasmaShellManager(QObject *parent) : QObject(parent)
{
    m_keyStateMap.insert(KWayland::Client::Keystate::Key::CapsLock, KWayland::Client::Keystate::Unlocked);
    m_keyStateMap.insert(KWayland::Client::Keystate::Key::NumLock, KWayland::Client::Keystate::Unlocked);
    m_keyStateMap.insert(KWayland::Client::Keystate::Key::ScrollLock, KWayland::Client::Keystate::Unlocked);

    auto connection = KWayland::Client::ConnectionThread::fromApplication(qApp);
    auto registry = new KWayland::Client::Registry(this);
    registry->create(connection->display());

    connect(registry, &KWayland::Client::Registry::plasmaShellAnnounced, this, [=]() {
        qDebug() << "plasmaShellAnnounced...";
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::PlasmaShell);
        if (interface.name != 0) {
            qDebug() << "createPlasmaShell...";
            m_plasmaShell = registry->createPlasmaShell(interface.name, interface.version, this);
        }
    });

    connect(registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced, this, [=]() {
        qDebug() << "plasmaWindowManagementAnnounced";
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::PlasmaWindowManagement);
        if (interface.name != 0) {
            qDebug() << "createPlasmaWindowManagement";
            m_windowManager = registry->createPlasmaWindowManagement(interface.name, interface.version, this);
        }
        if (m_windowManager) {
            connect(
                m_windowManager,
                &KWayland::Client::PlasmaWindowManagement::windowCreated,
                [this](KWayland::Client::PlasmaWindow *window) {
                    qDebug() << "PlasmaWindow...";
                    if (window->pid() == getpid()) {
                        if (isFirstCreate) {
                            isFirstCreate = false;
                            m_appWindow = window;

                            connect(m_appWindow, &KWayland::Client::PlasmaWindow::activeChanged, [this]() {
                                this->setAppWindowKeepAbove(true);
                                this->setAppWindowActive();
                            });
                            connect(m_appWindow, &KWayland::Client::PlasmaWindow::keepAboveChanged, [this]() {
                                this->setAppWindowKeepAbove(true);
                                this->setAppWindowActive();
                            });
                        }
                    }
                });
        }
    });

    connect(registry, &KWayland::Client::Registry::shellAnnounced, this, [=]() {
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::Shell);
        if (interface.name != 0) {
            m_shell = registry->createShell(interface.name, interface.version, this);
        }
    });

    connect(registry, &KWayland::Client::Registry::fakeInputAnnounced, this, [=]() {
        qDebug() << "fakeInputAnnounced";
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::FakeInput);
        if (interface.name != 0) {
            qDebug() << "createFakeInput";
            m_fakeInput = registry->createFakeInput(interface.name, interface.version);
            m_fakeInput->authenticate("ukui-screensaver-dialog", "virual keyboard");
        }
    });

    connect(registry, &KWayland::Client::Registry::keystateAnnounced, this, [=]() {
        qDebug() << "keystateAnnounced";
        const auto interface = registry->interface(KWayland::Client::Registry::Interface::Keystate);
        if (interface.name != 0) {
            qDebug() << "createKeyState";
            m_keyState = registry->createKeystate(interface.name, interface.version);
            if (m_keyState) {
                connect(
                    m_keyState,
                    &KWayland::Client::Keystate::stateChanged,
                    [this](KWayland::Client::Keystate::Key key, KWayland::Client::Keystate::State state) {
                        qDebug() << "key = " << key << "state = " << state;
                        m_keyStateMap[key] = state;
                        emit keyStateChanged();
                    });
                m_keyState->fetchStates();
            }
        }
    });

    registry->setup();
    connection->roundtrip();
}
