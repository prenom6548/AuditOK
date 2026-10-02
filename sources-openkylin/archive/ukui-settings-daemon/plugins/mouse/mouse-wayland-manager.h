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
#ifndef MouseWaylandManager_H
#define MouseWaylandManager_H

#include "mouse-common.h"
class MouseWaylandManager : public QObject
{
    Q_OBJECT

private:
    MouseWaylandManager()=delete;
    MouseWaylandManager(MouseWaylandManager&)=delete;
    MouseWaylandManager&operator=(const MouseWaylandManager&)=delete;
    MouseWaylandManager(QObject *parent = nullptr);
public:
    ~MouseWaylandManager();
    static MouseWaylandManager * MouseWaylandManagerNew();
    bool start();
    void stop();

public Q_SLOTS:
    void MouseWaylandManagerIdleCb();
    void MouseCallback(QString);
    void TouchpadCallback(QString);
    void initWaylandDbus();
    void initWaylandMouseStatus();
    void deviceChange(QString);

public:
    void SetLeftHandedAll (bool mouse_left_handed,
                           bool touchpad_left_handed);
    void SetMouseMotion();
    void SetTouchpadMotion();

    bool GetTouchpadHandedness (bool mouse_left_handed);

    void SetDisableWTyping  (bool state);

    void SetMiddleButtonAll   (bool middle_button);
    void SetLocatePointer     (bool     state);
    void SetTapToClickAll ();
    void SetNaturalScrollTouchPad ();
    void SetNaturalScrollMouse ();
    void SetMouseWheelSpeed (int speed);
    void SetMouseSettings();
    void SetTouchSettings();
    void SetTouchpadDoubleClickAll(bool state);
    void SetScrollingAll (QString keys);
    void SetTouchpadEnabledAll (bool state);
    void SetPlugMouseDisbleTouchpad(QGSettings*);


private: 
    friend GdkFilterReturn devicepresence_filter (GdkXEvent *xevent,
                                                  GdkEvent  *event,
                                                  gpointer   data);

private:
    unsigned long mAreaLeft;
    unsigned long mAreaTop;
    QTimer * time;
    QGSettings *settings_mouse;
    QGSettings *settings_touchpad;
#if 0   /* FIXME need to fork (?) mousetweaks for this to work */
    gboolean mousetweaks_daemon_running;
#endif
    gboolean syndaemon_spawned;
    gboolean locate_pointer_spawned;
    GPid     locate_pointer_pid;
    bool     imwheelSpawned;
    bool     mTouchDeviceFlag = false;
    bool     mMouseDeviceFlag = false;

    QDBusInterface *mWaylandIface;
    QList<QDBusInterface*> *mMouseDeviceIface;
    QDBusInterface *mTouchDeviceIface;

    static MouseWaylandManager *mMouseWaylandManager;
};

#endif // MouseWaylandManager_H
