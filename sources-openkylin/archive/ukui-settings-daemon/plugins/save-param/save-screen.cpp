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

#include <QScreen>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <kwindowsystem.h>
#include <QFile>
#include <QDir>
#include <QtXml>

#include "xrandroutput.h"
#include "save-screen.h"
#include "clib-syslog.h"
#include "usd_base_class.h"

#include "wayland-screen-manager.h"


SaveScreenParam::SaveScreenParam(QObject *parent)
{
    UsdBaseClass::printTimeWithMsg("init saveScreenParam.", __FUNCTION__, __LINE__);
}

SaveScreenParam::~SaveScreenParam()
{
    UsdBaseClass::printTimeWithMsg("exit saveScreenParam.", __FUNCTION__, __LINE__);
}

void SaveScreenParam::setUserConfigParam()
{
    WaylandScreenManager *screenManager = new WaylandScreenManager();
    screenManager->setScreenConfig();
}
