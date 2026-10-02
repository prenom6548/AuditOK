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

#ifndef WAYLANDSCREENMANAGER_H
#define WAYLANDSCREENMANAGER_H

#include <QObject>

#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/log.h>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>

#include "../xrandr/xrandr-config.h"
#include "../xrandr/xrandr-output.h"
#include "clib-syslog.h"

class WaylandScreenManager:public QObject
{
public:
    WaylandScreenManager();
    ~WaylandScreenManager();
public:
    void setScreenConfig();

private:
    void applyConfig();
    void setOutputsModeToCloneWithPreferredMode();
    double getPreferredScale(const KScreen::OutputPtr output);

private:
    std::unique_ptr<xrandrConfig> m_outputsConfig = nullptr;
};

#endif // WAYLANDSCREENMANAGER_H
