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
#ifndef XSETTINGSMANAGER_H
#define XSETTINGSMANAGER_H

#include <QString>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>
#include "xsettings-common.h"

typedef void (*XSettingsTerminateFunc)  (int *cb_data);

#if defined __cplusplus
class XsettingsManager
{

public:
    XsettingsManager(Display                *display,
                     int                     m_displayScreen,
                     XSettingsTerminateFunc  terminate,
                     int                   *cb_data);
    ~XsettingsManager();

    Window get_window    ();
    Bool   process_event (XEvent           *xev);
    XSettingsResult delete_setting (const char       *name);
    XSettingsResult set_setting    (XSettingsSetting *setting);
    XSettingsResult set_int        (const char       *name,
                                    int               value);
    XSettingsResult set_string     (const char       *name,
                                    const char       *value);
    XSettingsResult set_color      (const char       *name,
                                    XSettingsColor   *value);
    void setting_store (XSettingsSetting *setting,
                        XSettingsBuffer *buffer);
    XSettingsResult setXsettingsWindowsProperty         ();
private:
    Window createXSettingsWindows(Display                *display,
                                int                     m_displayScreen,
                                int                   *cb_data);

    bool managerDisplayScreen();

private:
    Display *m_pDisplay;
    int m_displayScreen;

    Window m_xSettingsWinId;

    Atom m_managerAtom;
    Atom m_selectionAtom;
    Atom m_xsettingsAtom;

    XSettingsTerminateFunc terminate;
    void *cb_data;

    XSettingsList *settings;
    unsigned long serial;
    Time m_timestamp;
    QString m_selectionName;
    const QString C_XSETTINGS_NAME = "_XSETTINGS_SETTINGS";
    const QString C_MANAGER_NAME = "MANAGER";
    const QString C_NET_WM_NAME = "_NET_WM_NAME";
    const QString C_UTF8_STRING = "UTF8_STRING";
    const QString C_NET_WM_PID = "_NET_WM_PID";
};

#endif
Bool
xsettings_manager_check_running (Display *display,
                                 int      screen);
#endif // XSETTINGSMANAGER_H
