/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
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
#ifndef KeyboardWaylandManager_H
#define KeyboardWaylandManager_H

#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWidget>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <QTimer>
#include <QtX11Extras/QX11Info>

#include <QGSettings/qgsettings.h>
#include <QApplication>

#include "xeventmonitor.h"
#include "keyboard-xkb.h"
#include "usd_base_class.h"
#include "plugin-manager-interface.h"
/**************************/
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/keystate.h>
using namespace KWayland::Client;
/**************************/

#ifdef HAVE_X11_EXTENSIONS_XF86MISC_H
#include <X11/extensions/xf86misc.h>
#endif

#ifdef HAVE_X11_EXTENSIONS_XKB_H
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#endif

class KeyState;

class KeyboardXkb;
class KeyboardWaylandManager : public ManagerInterface
{
    Q_OBJECT
private:
    KeyboardWaylandManager()=delete;
    KeyboardWaylandManager(KeyboardWaylandManager&)=delete;
    KeyboardWaylandManager&operator=(const KeyboardWaylandManager&)=delete;
    KeyboardWaylandManager(QObject *parent = nullptr);


public:
    ~KeyboardWaylandManager();
    static KeyboardWaylandManager *KeyboardWaylandManagerNew();
    virtual bool Start();
    virtual void Stop();
public Q_SLOTS:
    void start_keyboard_idle_cb ();
    void apply_settings  (QString);
    void onKeyStateChange(Keystate::Key key,Keystate::State state);

private:
    friend void apply_repeat     (KeyboardWaylandManager *manager);
    void showCustomOsdByDbus(const QString& icon);

private:
    QTimer                 *time;
    static KeyboardWaylandManager *mKeyboardWaylandManager;
    QGSettings             *settings;
    QGSettings             *ksettings;
    int                     old_state;
    bool                    stInstalled;

    KeyState*               m_keyState;

    Keystate::State         m_capsLock;
    Keystate::State         m_numLock;

};

class KeyState : public QObject
{
    Q_OBJECT
public:
    explicit KeyState(QObject* parent = nullptr);
    //连接状态改变信号
    void connectInit();
Q_SIGNALS:
    void keyStateChange(Keystate::Key, Keystate::State);


private:
    Registry m_registry;

};


#endif // KeyboardWaylandManager_H
