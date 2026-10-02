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

#include "xEventMonitor.h"

#define MAX_OPENDISPLAY_COUNT 10

xEventMonitor::xEventMonitor(QObject *parent) : QThread(parent)
{
    start(QThread::LowestPriority);
    USD_LOG(LOG_DEBUG,"start xevent monitor!");
}

void xEventMonitor::run()
{
    int count = 0;
    while (!isOpenDisplay) {
        if(MAX_OPENDISPLAY_COUNT <= count) {
            USD_LOG(LOG_DEBUG,"unable to open display and opendisplay is 10 times");
            break;
        }
        msleep(500*count);
        ++count;
        Display* display = QX11Info::display();//XOpenDisplay(0);
        USD_LOG(LOG_DEBUG,"start xevent monitor init!");
        if (display == 0) {
            USD_LOG(LOG_DEBUG, "unable to open display\n");
            continue;
        }

        // Receive from ALL clients, including future clients.
        XRecordClientSpec clients = XRecordAllClients;
        XRecordRange* range = XRecordAllocRange();
        if (range == 0) {
            USD_LOG(LOG_DEBUG,"unable to allocate XRecordRange\n");
            XCloseDisplay(display);
            continue;
        }

        // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
        memset(range, 0, sizeof(XRecordRange));
        range->device_events.first = KeyPress;
        range->device_events.last  = MotionNotify;

        // And create the XRECORD context.
        m_context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
        if (m_context == 0) {
            USD_LOG(LOG_DEBUG,"XRecordCreateContext failed\n");//fprintf(stderr, "XRecordCreateContext failed\n");
            XFree(range);
            continue;
        }
        XFree(range);
        XSync(display, True);

        m_displayDatalink = XOpenDisplay(0);
        if (m_displayDatalink == 0) {
            USD_LOG(LOG_DEBUG,"unable to open second display\n");
            continue;
        }

        if (!XRecordEnableContext(m_displayDatalink, m_context, callback, (XPointer) this)) {
            USD_LOG(LOG_DEBUG,"XRecordEnableContext() failed\n");
            XCloseDisplay(m_displayDatalink);
            continue;
        }

        isOpenDisplay = true;
        XCloseDisplay(m_displayDatalink);
    }
}
void xEventMonitor::callback(XPointer ptr, XRecordInterceptData* data)
{
    ((xEventMonitor *) ptr)->handleRecordEvent(data);
}


bool xEventMonitor::getWinPressStatus()
{
    return winPress_l | winPress_r;
}

bool xEventMonitor::getCtrlPressStatus()
{
    return ctrlPress_l | ctrlPress_r;
}

bool xEventMonitor::getAltPressStatus()
{
    return altPress_l | altPress_r;
}

bool xEventMonitor::getShiftPressStatus()
{
    return shiftPress_l | shiftPress_r;
}

uint xEventMonitor::getModifier()
{
    return m_modifier;
}

void xEventMonitor::freeXres()
{
    XRecordDisableContext(QX11Info::display(),m_context);
    XRecordFreeContext(QX11Info::display(),m_context);
    XSync(QX11Info::display(),True);
}

void xEventMonitor::updateModifier()
{
    if (getCtrlPressStatus()) {
        if (!(m_modifier & ControlMask)) {
            m_modifier |= ControlMask;
        }
    } else {
        if (m_modifier & ControlMask) {
            m_modifier ^= ControlMask;
        }
    }
    if (getAltPressStatus()) {
        if (!(m_modifier & Mod1Mask)) {
            m_modifier |= Mod1Mask;
        }
    } else {
        if (m_modifier & Mod1Mask) {
            m_modifier ^= Mod1Mask;
        }
    }
    if (getShiftPressStatus()) {
        if (!(m_modifier & ShiftMask)) {
            m_modifier |= ShiftMask;
        }
    } else {
        if (m_modifier & ShiftMask) {
            m_modifier ^= ShiftMask;
        }
    }
    if (getWinPressStatus()) {
        if (!(m_modifier & Mod4Mask)) {
            m_modifier |= Mod4Mask;
        }
    } else {
        if (m_modifier & Mod4Mask) {
            m_modifier ^= Mod4Mask;
        }
    }
}

void xEventMonitor::handleRecordEvent(XRecordInterceptData* data)
{
    int eventKeysym;
    static int Bpress = 0;
    if (data->category == XRecordFromServer) {
        xEvent * event = (xEvent *)data->data;

        if (event->u.u.type!=KeyPress && event->u.u.type!=KeyRelease){
            goto ERR;
        }
        KeySym sym = XkbKeycodeToKeysym(QX11Info::display(), event->u.u.detail, 0, 0);
        switch (event->u.u.type) {
        case KeyPress:
            switch(sym){
            case XK_Shift_L:
                shiftPress_l = true;
            case XK_Shift_R:
                shiftPress_r = true;
                break;
            case XK_Control_L:
                ctrlPress_l = true;
            case XK_Control_R:
                ctrlPress_r = true;
                Q_EMIT keyPress(sym);
                break;
            case XK_Alt_L:
                altPress_l = true;
                break;
            case XK_Alt_R:
                altPress_r = true;
                break;
            case XK_Meta_L:
            case XK_Super_L:
                winPress_l = true;
            case XK_Meta_R:
            case XK_Super_R:
                winPress_r = true;
                break;
             default :
                updateModifier();
                Q_EMIT keyPress(sym);
                break;
            }
            break;
        case KeyRelease:
            switch (sym) {
            case XK_Shift_L:
                shiftPress_l = false;
            case XK_Shift_R:
                shiftPress_r = false;
                break;
            case XK_Control_L:
                ctrlPress_l = false;
            case XK_Control_R:
                ctrlPress_r = false;
                break;
            case XK_Alt_L:
                altPress_l = false;
            case XK_Alt_R:
                altPress_r = false;
                break;
            case XK_Meta_L:
            case XK_Super_L:
                winPress_l = false;
            case XK_Meta_R:
            case XK_Super_R:
                winPress_r = false;
                break;
            default :
                updateModifier();
                Q_EMIT keyRelease(sym);
                break;
            }
        default:
            break;
        }
    }
ERR:
    XRecordFreeData(data);
}

bool xEventMonitor::filterWheelEvent(int detail)
{
    return detail != WheelUp && detail != WheelDown && detail != WheelLeft && detail != WheelRight;
}
