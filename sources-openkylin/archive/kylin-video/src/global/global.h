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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <QString>
#include <QSettings>
#include <QGSettings>

#include "core/sqlitehandle.h"
#include "core/mpvtypes.h"
#include "globalconfig.h"
#include "shortcutsetting.h"

#include <windowmanager/windowmanager.h>
#include <ukuistylehelper/ukuistylehelper.h>

// Some global objects
#define MENUWIDTH 180
#define MENU_BAR_BTN_SIZE 30
#define CONTRAL_BAR_BTN_SIZE 16
#define THEME_BUTTON_SIZE 36

class QSettings;
class QGSettings;
class ShortCutSetting;

//using namespace Control;
#define KYLIN_USER_GUIDE_PATH       "/"
#define KYLIN_USER_GUIDE_SERVICE    "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE  "com.guide.hotel"

#define PERSONALISE_SHEME   "org.ukui.control-center.personalise"
#define ORG_UKUI_STYLE      "org.ukui.style"
#define STYLE_UKUI_DEFAULT  "ukui-default"
#define STYLE_UKUI_LIGHT    "ukui-light"
#define STYLE_UKUI_BLACK    "ukui-dark"
#define ICO_DIR_DEFAULT     "ico_light"
#define ICO_DIR_DBLACK      "ico"
#define FOLLOW_SYS_THEME    true

namespace Global {

// 错误类型
enum KERROR_TYPE{
    NO_ERROR = 0
};

extern Mpv::PlayState   g_playstate;
extern QGSettings       *g_gsettings;
extern QGSettings       *g_gsettings_control_center;
//! Read and store application settings
extern QSettings        *g_settings;
extern SqliteHandle     *g_sqlite;
extern GlobalConfig     *g_config;
extern ShortCutSetting  *g_shortcut;
extern bool isWayland;
extern bool isTablet;
extern bool isHaWeiKe;
extern bool isUrl;
extern bool is4Kscreen;
extern bool isSpecialCard;
extern bool isDisableShortcuts;
extern bool unsupportedVDPAUCOPY;
extern bool isD3000M; //是否使用d3000m集显

void initGlobal();
}

#endif
