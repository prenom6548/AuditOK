/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukccsessionservice.h"
#include "ukccsessionserver.h"
#include "plugins/keyboard/keyboardinterface.h"
#include "plugins/mouse/mouseinterface.h"
#include "plugins/area/areainterface.h"
#include "plugins/default/defaultinterface.h"
#include "plugins/autoboot/autostartinterface.h"
#include "plugins/about/aboutinterface.h"
#include "plugins/datetime/datetimeinterface.h"
#include "plugins/wallpaper/wallpaperinterface.h"
#include "plugins/screenlock/screenlockinterface.h"
#include "plugins/shortcut/shortcutinterface.h"
#include "plugins/screensaver/screensaverinterface.h"
#include "plugins/vino/vinointerface.h"
#include "plugins/font/fontinterface.h"
#include "plugins/boot/bootinterface.h"
#include "plugins/theme/themeinterface.h"
#include "plugins/accessibility/accessibilityinterface.h"

ukccSessionService::ukccSessionService(QObject *parent)
    : QObject{parent}
{

}

QList<UKUI::ServiceObject *> ukccSessionService::getServiceObjects()
{
    QList<UKUI::ServiceObject *> services;
    auto instance = new ukccSessionServer;
    auto about = new AboutInterface;
    auto area = new AreaInterface;
    auto autoboot = new AutoStartInterface;
    auto datetime = new DatetimeInterface;
    auto defaultapp = new DefaultInterface;
    auto keyboard = new KeyBoardInterface;
    auto mouse = new MouseInterface;
    auto screenlock = new ScreenlockInterface;
    auto screensaver = new ScreensaverInterface;
    auto shortcut = new ShortcutInterface;
    auto vino = new VinoInterface;
    auto wallpaper = new WallpaperInterface;
    auto font = new FontInterface;
    auto boot = new BootInterface;
    auto theme = new ThemeInterface;
    auto accessibility = new AccessibilityInterface;
    services << instance << about << area << autoboot << datetime << defaultapp
             <<keyboard << mouse << screenlock << screensaver
             << shortcut << vino << wallpaper << font << boot << theme << accessibility;
    return services;
}
