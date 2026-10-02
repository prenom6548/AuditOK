/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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

 
#ifndef OSD_STRING_H
#define OSD_STRING_H

//这个头文件存放宏定义

//界面尺寸
#define ICON_WINDOW_SIZE 92
#define ICON_BACKGROUND_SIZE 72
#define ICON_SIZE 48

#define BAR_WINDOW_WIDTH 370
#define BAR_WINDOW_HEIGHT 72

#define BAR_BACKGROUND_WIDTH 52
#define BAR_BACKGROUND_HEIGHT 350

//获取图标的路径前缀
#define LOCAL_ICON_PREFIX ":/ukui_res/ukui/"

//GSettings的schema文件名
#define SIZE_SCHEMA "org.ukui.panel.settings"
#define OPACITY_SCHEMA "org.ukui.control-center.personalise"
#define THEME_SCHEMA "org.ukui.style"

//GSettings的关键字名
#define SIZE_KEY "panelsize"
#define OPACITY_KEY "transparency"
#define THEME_KEY "style-name"
#define THEME_COLOR_KEY "theme-color"

//默认颜色
#define OSD_BLACK "#232426"
#define OSD_WHITE "#F5F5F5"

//主题属性
#define LIGHT_THEME "ukui-light"

//注册服务名前缀和服务名
#define SERVER_NAME_PREFIX "org.ukui."
#define SERVER_NAME "OSDService"

#include <QDebug>


#endif // OSD_STRING_H
