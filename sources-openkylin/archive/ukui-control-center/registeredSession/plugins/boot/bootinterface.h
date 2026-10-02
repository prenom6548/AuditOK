/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#ifndef BOOTINTERFACE_H
#define BOOTINTERFACE_H

#include <ukui-service-interface/serviceobject.h>
#include <QObject>
#include <QtDBus>
#include <QDBusContext>
#include <QDBusConnectionInterface>

#define UKCC_SCHAME "org.ukui.control-center"
#define SYSTEMS_SAME "systems-same"

class BootInterface : public UKUI::ServiceObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Boot")

public:
    BootInterface();
    ~BootInterface();

private:
    int gotGrubSystems();
    bool callerPathCompare(QString path);
};

#endif // BOOTINTERFACE_H
