/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "settingscommon.h"

#include <QGuiApplication>
#include <QDebug>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <ukuisdk/kylin-com4cxx.h>

static const QString MAXTABLET = "MaxTablet";
static const QString PROJECT_INTEL  = "V10SP1-edu";

QString SettingsCommon::m_systemCategory = QString();

QString SettingsCommon::m_prjCodeName = QString();
int SettingsCommon::m_productFeat = -1;

bool SettingsCommon::isMaxTabletSystemCategory()
{
    if (m_systemCategory.isEmpty()) {
        char* vername = kdk_system_get_systemCategory();
        m_systemCategory = vername;
        free(vername);
        vername = nullptr;
    }
    return (m_systemCategory == MAXTABLET);
}

bool SettingsCommon::isTabletProductFeat()
{
    if (m_productFeat == -1) {
        m_productFeat = QString::fromStdString(KDKGetOSRelease("PRODUCT_FEATURES")).toInt();
    }

    return (m_productFeat == 3);
}

bool SettingsCommon::isIntelProject()
{
    if (m_prjCodeName.isEmpty()) {
        m_prjCodeName = QString::fromStdString(KDKGetPrjCodeName());
    }
    return (m_prjCodeName == PROJECT_INTEL);
}

bool SettingsCommon::isWaylandEnv()
{
    if (qgetenv("XDG_SESSION_TYPE") == "wayland")
        return true;
    if (QGuiApplication::platformName().contains(QLatin1String("wayland")))
        return true;

    qDebug() << "It is not wayland environment XDG_SESSION_TYPE:" << qgetenv("XDG_SESSION_TYPE")
             << "QPA:" << QGuiApplication::platformName();
    return false;
}
