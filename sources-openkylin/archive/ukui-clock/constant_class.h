/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 */
#ifndef CONSTANTCLASS_H
#define CONSTANTCLASS_H

#include <QDebug>
#include "utils.h"
//声明一些必须的宏
#define ORG_UKUI_STYLE            "org.ukui.style"
#define STYLE_NAME                "styleName"
#define STYLE_NAME_KEY_DARK       "ukui-dark"
#define STYLE_NAME_KEY_DEFAULT    "ukui-default"
#define STYLE_NAME_KEY_BLACK       "ukui-black"
#define STYLE_NAME_KEY_LIGHT       "ukui-light"
#define STYLE_NAME_KEY_WHITE       "ukui-white"
#define STYLE_ICON                "icon-theme-name"
#define STYLE_ICON_NAME           "iconThemeName"
#define THEME_COLOR               "themeColor"
#define TIME_SEPARATOR             ":"
#define TIME_SEPARATOR_CN             "："
#define APPLICATION_NAME "ukui-clock"
#define CLOCK_TITLE_NAME "Alarm"
#define KYLIN_CLOCK_APP_NAME "Alarm"

const static  QString chooseColor = "61,107,229,255";
const static  QString hoverColor = "55,144,250,255";
#define SYSTEM_FONT_EKY            "system-font-size"
/*!
 * 系统时间
 */
#define FORMAT_SCHEMA   "org.ukui.control-center.panel.plugins"
#define TIME_FORMAT_KEY "hoursystem"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define HOUR_SYSTEM "hoursystem"
#ifdef NOTKAIMING
#define DEFAULT_BELL_SAVE_PATH "/usr/share/ukui-clock/"
#else
#define DEFAULT_BELL_SAVE_PATH "/opt/apps/top.openkylin.ukui-clock/files/share/ukui-clock/"
#endif
#define DIY_BELL_SAVE_PATH "/kylindata/ukui-clock/"
#define WHEEL_KEY_HW "wheel-speed"
#define WHEEL_KEY_SP "wheelSpeed"
#define MOUSE_SCHEMA          "org.ukui.peripherals-mouse"
const static int TIME_SCROLL_NUM_SIZE = 3;
const static int ITEM_RADIUS=6;
const static int WINDOWN_RADIUS=12;
const static int SELECT_DIA_RADIUS=8;
const static int SELECT_ITEM_RADIUS=6;
const static int COUNTDOWN_TIME=6;
#define SSWND_DBUS_SERVICE     "org.ukui.ScreenSaverWnd"
#define SSWND_DBUS_PATH        "/"
#define SSWND_DBUS_INTERFACE   "org.ukui.ScreenSaverWnd"
#define SCREENSAVE_DBUS_SERVICE     "org.ukui.ScreenSaver"
#define SCREENSAVE_DBUS_PATH        "/"
#define SCREENSAVE_DBUS_INTERFACE   "org.ukui.ScreenSaver"
#endif // CONSTANTCLASS_H
