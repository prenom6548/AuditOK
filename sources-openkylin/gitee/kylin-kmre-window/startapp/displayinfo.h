/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QString>

class DisplayInfo
{
public:
    QString displayType = "unknown";
    QString cpuType = "";
    bool cpuSupported = false;
    QString gpuVendor = "unknown";
    QString gpuModel = "";
    bool gpuSupported = false;
    int physicalWidth = 0;
    int physicalHeight = 0;
    int displayWidth = 0;
    int displayHeight = 0;
    int density = 320;
};

extern DisplayInfo gDisplayInfo;