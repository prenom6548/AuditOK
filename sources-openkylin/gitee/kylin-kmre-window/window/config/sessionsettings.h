/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
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

#include "singleton.h"
#include "typedef.h"
#include <QSize>

enum class DisplayType : int32_t
{
    TYPE_UNKNOWN = -1,
    TYPE_DRM,
    TYPE_SW,
    TYPE_EMUGL,
    TYPE_EMUGL_SHM,
};

struct DisplayInfo 
{
    DisplayType displayType = DisplayType::TYPE_UNKNOWN;
    QSize virtualDisplaySize = QSize(0, 0);
    QSize physicalDisplaySize = QSize(0, 0);
    bool supportsThreadedOpenGL = false;
};

class SessionSettings: public kmre::Singleton<SessionSettings>
{
public:
    enum DisplayPlatform {
        NONE = -1,
        X11,
        WAYLAND,
    };

    kmre::sp<DisplayInfo> getDisplayInfo() { return mDisplayInfo; }
    DisplayType getDisplayType() { return mDisplayInfo->displayType; }
    QSize getVirtualDisplaySize() { return mDisplayInfo->virtualDisplaySize; }
    QSize getPhysicalDisplaySize() { return mDisplayInfo->physicalDisplaySize; }
    DisplayPlatform displayPlatform() { return mDisplayPlatform; };
    bool windowUsePlatformX11();
    bool windowUsePlatformWayland();
    bool hasWaylandPlasmaShellSupport();
    bool hasWaylandPlasmaWindowManagementSupport();
    bool hasWaylandUKUIDecorationSupport();
    bool hasWaylandXdgShellV6Support();
    bool hasWaylandXdgShellSupport();
    bool hasWaylandXdgActivationV1Support();
    bool hasWaylandGtkShell1Support();
    bool hasWaylandRemoteAccessSupport();

    bool supportsWaylandXdgShell();

private:
    explicit SessionSettings();
    ~SessionSettings(){}

    bool windowCouldUseWayland();
    friend Singleton<SessionSettings>;

private:
    kmre::sp<DisplayInfo> mDisplayInfo;
    DisplayPlatform mDisplayPlatform;
    DisplayPlatform mWindowPlatform;
};
