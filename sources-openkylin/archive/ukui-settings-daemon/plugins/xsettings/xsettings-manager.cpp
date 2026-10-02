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
#include <QDebug>
#include "xsettings-manager.h"
#include "xsettings-const.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "clib-syslog.h"

static Time get_server_time (Display *display,
                             Window   window);
static XSettingsList *Settings;
XsettingsManager::XsettingsManager(Display                *display,
                                   int                     screen,
                                   XSettingsTerminateFunc  terminate,
                                   int                    *cb_data)
{
    XClientMessageEvent xev;

    this->m_pDisplay = display;
    this->m_displayScreen = screen;

    m_selectionName = QString("_XSETTINGS_S%1").arg(this->m_displayScreen);
    this->m_selectionAtom = XInternAtom (display, m_selectionName.toLatin1().data(), False);
    this->m_xsettingsAtom = XInternAtom (display,
                                        C_XSETTINGS_NAME.toLatin1().data(), False);
    this->m_managerAtom = XInternAtom (display,
                                      C_MANAGER_NAME.toLatin1().data(), False);

    this->terminate = terminate;
    this->cb_data = cb_data;
    this->settings = NULL;

    this->serial = 0;

    XSetWindowAttributes attr;
    attr.override_redirect = True;

    this->m_xSettingsWinId = createXSettingsWindows(display, screen, cb_data);
//    managerDisplayScreen();
}

XsettingsManager::~XsettingsManager()
{
    USD_LOG(LOG_DEBUG,"bye~");
    XDestroyWindow (m_pDisplay, m_xSettingsWinId);
    xsettings_list_free (settings);
}

/**
 * get_server_time:
 * @display: display from which to get the time
 * @window: a #Window, used for communication with the server.
 *          The window must have PropertyChangeMask in its
 *          events mask or a hang will result.
 *
 * Routine to get the current X server time stamp.
 *
 * Return value: the time stamp.
 **/
typedef struct
{
    Window window;
    Atom timestamp_prop_atom;
} TimeStampInfo;

static Bool
timestamp_predicate (Display *display,
                     XEvent  *xevent,
                     XPointer arg)
{
    TimeStampInfo *info = (TimeStampInfo *)arg;

    if (xevent->type == PropertyNotify &&
        xevent->xproperty.window == info->window &&
        xevent->xproperty.atom == info->timestamp_prop_atom)
        return True;
    return False;
}

static Time get_server_time (Display *display,
                             Window   window)
{
    unsigned char c = 'a';
    XEvent xevent;
    TimeStampInfo info;

    info.timestamp_prop_atom = XInternAtom  (display, "_TIMESTAMP_PROP", False);
    info.window = window;

    XChangeProperty (display, window,
                     info.timestamp_prop_atom, info.timestamp_prop_atom,
                     8, PropModeReplace, &c, 1);

    XIfEvent (display, &xevent,
              timestamp_predicate, (XPointer)&info);

    return xevent.xproperty.time;
}

Window XsettingsManager::get_window()
{
    return m_xSettingsWinId;
}

Bool   XsettingsManager::process_event (XEvent           *xev)
{
    if (xev->xany.window == this->m_xSettingsWinId &&
            xev->xany.type == SelectionClear &&
            xev->xselectionclear.selection == this->m_selectionAtom)
    {
        this->terminate ((int *)this->cb_data);
        return True;
    }

    return False;
}

XSettingsResult XsettingsManager::delete_setting (const char       *name)
{
    return xsettings_list_delete (&Settings, name);
}

XSettingsResult XsettingsManager::set_setting    (XSettingsSetting *setting)
{
    XSettingsSetting *old_setting = xsettings_list_lookup (Settings, setting->name);
    XSettingsSetting *new_setting;
    XSettingsResult result;
    if (old_setting)
    {
        if (xsettings_setting_equal (old_setting, setting))
            return XSETTINGS_SUCCESS;

        xsettings_list_delete (&Settings, setting->name);
    }
    new_setting = xsettings_setting_copy (setting);
    if (!new_setting)
        return XSETTINGS_NO_MEM;

    new_setting->last_change_serial = this->serial;

    result = xsettings_list_insert (&Settings, new_setting);

    if (result != XSETTINGS_SUCCESS)
        xsettings_setting_free (new_setting);
    return result;
}

XSettingsResult XsettingsManager::set_int        (const char       *name,
                                                  int               value)
{
    XSettingsSetting setting;

    setting.name = (char *)name;
    setting.type = XSETTINGS_TYPE_INT;
    setting.data.v_int = value;

    return set_setting (&setting);
}

XSettingsResult XsettingsManager::set_string     (const char       *name,
                                                  const char       *value)
{
    XSettingsSetting setting;

    setting.name = (char *)name;
    setting.type = XSETTINGS_TYPE_STRING;
    setting.data.v_string = (char *)value;

    return set_setting (&setting);
}

XSettingsResult XsettingsManager::set_color      (const char       *name,
                                                  XSettingsColor   *value)
{
    XSettingsSetting setting;

    setting.name = (char *)name;
    setting.type = XSETTINGS_TYPE_COLOR;
    setting.data.v_color = *value;

    return set_setting (&setting);

}
static size_t setting_length (XSettingsSetting *setting)
{
    size_t length = 8;    /* type + pad + name-len + last-change-serial */
    length += XSETTINGS_PAD (strlen (setting->name), 4);

    switch (setting->type)
    {
    case XSETTINGS_TYPE_INT:
        length += 4;
        break;
    case XSETTINGS_TYPE_STRING:
        length += 4 + XSETTINGS_PAD (strlen (setting->data.v_string), 4);
        break;
    case XSETTINGS_TYPE_COLOR:
        length += 8;
        break;
    }

    return length;
}

Bool xsettings_manager_check_running (Display *display,
                                      int      screen)
{
    char buffer[256];
    Atom selection_atom;

    sprintf(buffer, "_XSETTINGS_S%d", screen);
    selection_atom = XInternAtom (display, buffer, False);

    if (XGetSelectionOwner(display, selection_atom)){
        return True;
    }
    else
        return False;
}

void XsettingsManager::setting_store (XSettingsSetting *setting,
                                      XSettingsBuffer *buffer)
{
    size_t string_len;
    size_t length;

    *(buffer->pos++) = setting->type;
    *(buffer->pos++) = 0;

    string_len = strlen (setting->name);
    *(CARD16 *)(buffer->pos) = string_len;
    buffer->pos += 2;

    length = XSETTINGS_PAD (string_len, 4);
    memcpy (buffer->pos, setting->name, string_len);
    length -= string_len;
    buffer->pos += string_len;

    while (length > 0)
    {
        *(buffer->pos++) = 0;
        length--;
    }

    *(CARD32 *)(buffer->pos) = setting->last_change_serial;
    buffer->pos += 4;

    switch (setting->type)
    {
    case XSETTINGS_TYPE_INT:
        USD_LOG(LOG_DEBUG,"%s:%d", setting->name, setting->data.v_int);
        *(CARD32 *)(buffer->pos) = setting->data.v_int;
        buffer->pos += 4;
        break;
    case XSETTINGS_TYPE_STRING:
        USD_LOG(LOG_DEBUG,"%s:%s", setting->name, setting->data.v_string);
        string_len = strlen (setting->data.v_string);
        *(CARD32 *)(buffer->pos) = string_len;
        buffer->pos += 4;

        length = XSETTINGS_PAD (string_len, 4);
        memcpy (buffer->pos, setting->data.v_string, string_len);
        length -= string_len;
        buffer->pos += string_len;

        while (length > 0)
        {
            *(buffer->pos++) = 0;
            length--;
        }
        break;
    case XSETTINGS_TYPE_COLOR:
        *(CARD16 *)(buffer->pos) = setting->data.v_color.red;
        *(CARD16 *)(buffer->pos + 2) = setting->data.v_color.green;
        *(CARD16 *)(buffer->pos + 4) = setting->data.v_color.blue;
        *(CARD16 *)(buffer->pos + 6) = setting->data.v_color.alpha;
        buffer->pos += 8;
        break;
    }
}

XSettingsResult XsettingsManager::setXsettingsWindowsProperty()
{
    XSettingsBuffer buffer;
    XSettingsList *iter;
    int n_settings = 0;
    buffer.len = 12;              /* byte-order + pad + SERIAL + N_SETTINGS */
    iter = Settings;
    while (iter)
    {
        buffer.len += setting_length (iter->setting);
        n_settings++;
        iter = iter->next;
    }
    buffer.data = buffer.pos = new unsigned char[buffer.len];
    if (!buffer.data)
        return XSETTINGS_NO_MEM;

    *buffer.pos = xsettings_byte_order ();

    buffer.pos += 4;
    *(CARD32 *)buffer.pos = this->serial++;
    buffer.pos += 4;
    *(CARD32 *)buffer.pos = n_settings;
    buffer.pos += 4;

    iter = Settings;
    while (iter)
    {
        setting_store (iter->setting, &buffer);
        iter = iter->next;
    }
    USD_LOG(LOG_DEBUG,"ready write 0x%x  settings_atom:%x :%d", this->m_xSettingsWinId, this->m_xsettingsAtom, buffer.len);
    XChangeProperty (this->m_pDisplay, this->m_xSettingsWinId,
                     this->m_xsettingsAtom, this->m_xsettingsAtom,
                     8, PropModeReplace, buffer.data, buffer.len);
    free (buffer.data);
    managerDisplayScreen();
    return XSETTINGS_SUCCESS;
}

Window XsettingsManager::createXSettingsWindows(Display *display, int screen, int *cb_data)
{
    if (screen < 0 || screen >= ScreenCount(display))
       return false;

     XSetWindowAttributes attr;
     attr.override_redirect = True;
     m_xSettingsWinId = XCreateWindow(display,
                                RootWindow(display, screen),  // parent
                                -1, -1,                        // x, y
                                1, 1,                          // width, height
                                0,                             // border_width
                                CopyFromParent,                // depth
                                InputOutput,                   // class
                                CopyFromParent,                // visual
                                CWOverrideRedirect,            // attr_mask
                                &attr);
     if (m_xSettingsWinId == None) {
         USD_LOG(LOG_DEBUG,"create XSETTINGS windows error.");
         return 0;
     }

     // This sets a few properties for us, including WM_CLIENT_MACHINE.
     XSetWMProperties(display,
                      m_xSettingsWinId,
                      NULL,   // window_name
                      NULL,   // icon_name
                      NULL,   // argv
                      0,      // argc
                      NULL,   // normal_hints
                      NULL,   // wm_hints
                      NULL);  // class_hints

     XStoreName(display, m_xSettingsWinId,
                C_XSETTINGS_NAME.toLatin1().data());

     XChangeProperty(display,
                     m_xSettingsWinId,
                     XInternAtom(display,
                                 C_NET_WM_NAME.toLatin1().data(), False),  // property
                     XInternAtom(display,
                                 C_UTF8_STRING.toLatin1().data(), False),   // type
                     8,  // format (bits per element)
                     PropModeReplace,
                     reinterpret_cast<const unsigned char*>(C_XSETTINGS_NAME.toLatin1().data()),
                     C_XSETTINGS_NAME.length());

     // Grab a timestamp from our final property change; we'll need it later
     // when announcing that we've taken the manager selection.
     pid_t pid = getpid();
     XSelectInput(display, m_xSettingsWinId, PropertyChangeMask);
     XChangeProperty(display,
                     m_xSettingsWinId,
                     XInternAtom(display, reinterpret_cast<const char*>(C_NET_WM_PID.toLatin1().data()), False),  // property
                     XA_CARDINAL,  // type
                     32,           // format (bits per element)
                     PropModeReplace,
                     reinterpret_cast<const unsigned char*>(&pid), // value
                     1);           // num elements
     XSelectInput(display, m_xSettingsWinId, NoEventMask);

     XEvent event;
     while (true) {
       XWindowEvent(display, m_xSettingsWinId, PropertyChangeMask, &event);
       if (event.type == PropertyNotify) {
         *cb_data = event.xproperty.time;
           m_timestamp =  event.xproperty.time;
         break;
       }
     }
     return m_xSettingsWinId;
}

bool XsettingsManager::managerDisplayScreen()
{
    Window root = RootWindow(m_pDisplay, m_displayScreen);

     Window prevWin = XGetSelectionOwner(m_pDisplay, m_selectionAtom);
     USD_LOG(LOG_DEBUG,"get XGetSelectionOwner :0x%x..", prevWin);
     if (prevWin) {
       XSelectInput(m_pDisplay, prevWin, StructureNotifyMask);
     }

     XSetSelectionOwner(m_pDisplay, m_selectionAtom, m_xSettingsWinId, CurrentTime);
    USD_LOG(LOG_DEBUG,"%s: took ownership of selection %s\n", C_XSETTINGS_NAME.toLatin1().data(),
            m_selectionName.toLatin1().data());
//     if (prevWin) {
//       // Wait for the previous owner to go away.
//       XEvent event;
//       while (true) {
//         XWindowEvent(m_pDisplay, prevWin, StructureNotifyMask, &event);
//         if (event.type == DestroyNotify)
//           USD_LOG(LOG_DEBUG,"get DestroyNotify..");
//           break;
//       }
//     }

     // Make sure that no one else took the selection while we were waiting.
     if (XGetSelectionOwner(m_pDisplay, m_selectionAtom) != m_xSettingsWinId) {

       USD_LOG(LOG_DEBUG, "%s: Someone else took ownership of the %s selection\n",
               C_XSETTINGS_NAME.toLatin1().data(),
               m_selectionName.toLatin1().data());
       return false;
     } else {
         USD_LOG(LOG_DEBUG,"get XGetSelectionOwner :0x%x..", m_xSettingsWinId);
     }

     XEvent ev;
     ev.xclient.type = ClientMessage;
     ev.xclient.window = root;
     ev.xclient.message_type = XInternAtom(m_pDisplay, "MANAGER", False);
     ev.xclient.format = 32;
     ev.xclient.data.l[0] = m_timestamp;
     ev.xclient.data.l[1] = m_selectionAtom;
     ev.xclient.data.l[2] = m_xSettingsWinId;
     ev.xclient.data.l[3] = 0;
     XSendEvent(m_pDisplay,
                root,
                False,                // propagate
                StructureNotifyMask,  // event_mask
                &ev);

    return true;
}
