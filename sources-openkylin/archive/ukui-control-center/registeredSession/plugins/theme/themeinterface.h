/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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
#ifndef THEMEINTERFACE_H
#define THEMEINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QDBusArgument>
#include <QVariant>
#include <QDBusMetaType>
#include "../../universalinterface.h"

class ThemeInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Theme")

public:
    ThemeInterface();
    bool initWindowRadius();

public Q_SLOTS:
    bool isHardwareRendering();
    bool setEffectState(bool status);

Q_SIGNALS:
    void effectChanged(bool status);

private:
    QDBusInterface *m_wlcomDBus = nullptr;
    QGSettings *m_styleGSetting = nullptr;
    QGSettings *m_ukccPersonliseGSetting = nullptr;
    bool m_isHardwareRendering = true;
};

#endif // THEMEINTERFACE_H
