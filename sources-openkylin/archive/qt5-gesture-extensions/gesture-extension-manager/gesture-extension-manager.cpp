/*
Copyright (c) KylinSoft  Co., Ltd. 2021.All rights reserved.
License: GPL-2+
 This package is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 .
 This package is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 .
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>
 .
 On Debian systems, the complete text of the GNU General
 Public License version 2 can be found in "/usr/share/common-licenses/GPL-2".
*/

#include "gesture-extension-manager.h"

#include "gesture-extension-manager-private.h"
#include "../gesture-extensions/gesture-extension.h"
#include "../gesture-extensions/slide-gesture.h"

#include <syslog.h>
#include <QDebug>

namespace UkuiGestureExtension {

static GestureExtensionManager* global_exentsion = nullptr;

GestureExtensionManager::GestureExtensionManager(QObject* parent) : QObject(parent)
{
//    qDebug() << "GestureExtensionManager";
    m_p = new GestureExtensionManagerPrivate(this);
}

GestureExtensionManager::~GestureExtensionManager()
{
//    qDebug() << "~GestureExtensionManager";
    if (global_exentsion == this)
        global_exentsion = nullptr;
}

GestureExtensionManager* GestureExtensionManager::getInstance(QObject* parent)
{
    if (!global_exentsion) {
        global_exentsion = new GestureExtensionManager(parent);
//        static GestureExtensionManager global_exentsion;
    }
    return global_exentsion;
//    return &global_exentsion;
}

void GestureExtensionManager::release()
{
    if (global_exentsion)
        global_exentsion->deleteLater();
}

void GestureExtensionManager::registerWidget(QWidget* widget)
{
    m_p->registerWidget(widget);
}

void GestureExtensionManager::unregisterWidget(QWidget* widget)
{
    m_p->unregisterWidget(widget);
}

} // namespace

void registerWidget(QWidget* widget, QObject* parent)
{
    UkuiGestureExtension::GestureExtensionManager::getInstance(parent)->registerWidget(widget);
}
void unregisterWidget(QWidget* widget, QObject* parent)
{
    UkuiGestureExtension::GestureExtensionManager::getInstance(parent)->unregisterWidget(widget);
}
