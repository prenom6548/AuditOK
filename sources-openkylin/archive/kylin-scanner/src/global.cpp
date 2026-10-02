/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "global.h"

#include <QSettings>
#include <QApplication>
#include <QFile>
#include <QDebug>

QSettings       *Global::g_settings     = nullptr;

using namespace Global;

void Global::global_init() {


    // g_settings
    QString filename = "~/.config/kylin-scanner.ini";

    g_settings = new QSettings(filename, QSettings::IniFormat);
    g_settings->setIniCodec("UTF-8");

    // 运行环境
    if(!g_settings->contains("General/display_env"))
    {
            g_settings->setValue("General/display_env", "x11");
    }

}

void Global::global_end() {
    delete g_settings;
}
