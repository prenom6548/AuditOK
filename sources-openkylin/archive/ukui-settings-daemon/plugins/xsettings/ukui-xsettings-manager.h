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
#ifndef UKUIXSETTINGSMANAGER_H
#define UKUIXSETTINGSMANAGER_H

#include <glib.h>
#include <QObject>
#include <QTimer>
#include <X11/Xatom.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
//#include "ixsettings-manager.h"
#include "xsettings-manager.h"
#include "fontconfig-monitor.h"
#include "abstract-manager.h"
#include "QGSettings/qgsettings.h"

class ukuiXSettingsManager: public AbstractManager
{
    Q_OBJECT
public:
    ukuiXSettingsManager(QObject *parent = nullptr);
    ~ukuiXSettingsManager();
    bool start() override;
    void stop() override;

    void setKwinMouseSize(int size);
public Q_SLOTS:
    void doStartTimeout();
public:
    //gboolean setup_xsettings_managers (ukuiXSettingsManager *manager);

    XsettingsManager **pManagers;
    GHashTable  *m_pGsettingsTable;
    GSettings   *m_pFontGsettings;

    //GSettings   *plugin_settings;
    fontconfig_monitor_handle_t *m_pFontConfigHandle;
    Display *m_pdpy = nullptr;
    QGSettings *m_pGnomeSettings = nullptr;
private:
    QTimer *m_pStartTimer;
};

#endif // UKUIXSETTINGSMANAGER_H
