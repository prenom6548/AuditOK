/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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

#ifndef PLASMASHELLMANAGER_H
#define PLASMASHELLMANAGER_H

#include <QObject>
#include <QWindow>
#include <QMap>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/shell.h>
#include <KWayland/Client/fakeinput.h>
#include <KWayland/Client/keystate.h>

class PlasmaShellManager : public QObject
{
    Q_OBJECT
public:
    static PlasmaShellManager *getInstance();

    bool setAppWindowActive();
    bool setAppWindowKeepAbove(bool keep);
    bool setMaximized(QWindow *window);
    bool setRole(QWindow *window, KWayland::Client::PlasmaShellSurface::Role role);
    bool setPos(QWindow *window, const QPoint &pos);
    void setKeyPressed(quint32 key);

    bool supportPlasmaShell();
    bool supportShell();
    bool supportPlasmaWindowManagement();
    bool supportFakeInput();
    bool supportKeyState();
    KWayland::Client::Keystate::State getKeyState(KWayland::Client::Keystate::Key key);
Q_SIGNALS:
    void keyStateChanged();
    void capslockStateChanged(bool isLocked);

private:
    explicit PlasmaShellManager(QObject *parent = nullptr);

    KWayland::Client::PlasmaShell *m_plasmaShell = nullptr;
    KWayland::Client::Shell *m_shell = nullptr;
    KWayland::Client::PlasmaWindowManagement *m_windowManager = nullptr;
    KWayland::Client::PlasmaWindow *m_appWindow = nullptr;
    KWayland::Client::FakeInput *m_fakeInput = nullptr;
    KWayland::Client::Keystate *m_keyState = nullptr;

    bool isFirstCreate = true;

    QMap<KWayland::Client::Keystate::Key, KWayland::Client::Keystate::State> m_keyStateMap;
};

#endif // PLASMASHELLMANAGER_H
