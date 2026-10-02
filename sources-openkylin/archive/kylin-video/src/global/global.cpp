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
#include "global.h"

#include <QSettings>
#include <QGSettings>
#include <QApplication>
#include <QFile>
#include <QDebug>
#include "path.h"

// #include <kysdk/kysdk-system/libkysysinfo.h>

QSettings       *Global::g_settings     = nullptr;
SqliteHandle    *Global::g_sqlite       = nullptr;
GlobalConfig    *Global::g_config       = nullptr;
QGSettings      *Global::g_gsettings    = nullptr;
ShortCutSetting *Global::g_shortcut     = nullptr;
QGSettings      *Global::g_gsettings_control_center = nullptr;
Mpv::PlayState  Global::g_playstate     = Mpv::Idle;
bool Global::isWayland = false;
bool Global::isTablet = false;
bool Global::isHaWeiKe = false;
bool Global::isUrl = false;
bool Global::is4Kscreen = false;
bool Global::isSpecialCard = false;
bool Global::isDisableShortcuts = false;
bool Global::unsupportedVDPAUCOPY = false;
bool Global::isD3000M = false;
//KPlayControl *Global::k_playcontrol = KPlayControl::getInstanece();

using namespace Global;

void Global::initGlobal() {

    // g_settings
    QString filename = Paths::iniPath() + "/kylin-video3.ini";
    QString dbname = Paths::iniPath() + "/kylin-video3.db";

    g_settings = new QSettings(filename, QSettings::IniFormat);
    g_settings->setObjectName("kylin-video_QSettings_g_settings");
    g_settings->setIniCodec("UTF-8");
    // 运行环境
    if(!g_settings->contains("General/display_env"))
    {
        if((QString(qgetenv("XDG_SESSION_TYPE")) == "wayland"))
            g_settings->setValue("General/display_env", "wayland");
        else
            g_settings->setValue("General/display_env", "x11");
    }
    isWayland   = QString(qgetenv("XDG_SESSION_TYPE")) == "wayland";

    g_gsettings = new QGSettings(ORG_UKUI_STYLE);
    g_gsettings->setObjectName("kylin-video_QGSettings_g_gsettings");
    if (QGSettings::isSchemaInstalled(PERSONALISE_SHEME))
        g_gsettings_control_center = new QGSettings(PERSONALISE_SHEME);
        g_gsettings_control_center->setObjectName("kylin-video_QGSettings_g_gsettings_control_center");

    g_shortcut  = ShortCutSetting::getInstance(g_settings);
    g_sqlite    = SqliteHandle::getInstance(dbname);
    g_config    = GlobalConfig::getInstance();

    // if (QString(kdk_system_get_projectName()) == "V10SP1" && QString(kdk_system_get_productFeatures()).toInt() >= 2) {
    //     isTablet = true;
    // }
    // else {
    //     isTablet = false;
    // }
}
