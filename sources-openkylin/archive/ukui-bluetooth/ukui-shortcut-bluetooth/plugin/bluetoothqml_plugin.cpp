/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetoothqml_plugin.h"
#include "bluetoothinterface.h"

#include <qqml.h>
#include <QQmlContext>


void BluetoothqmlPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString(uri) == QLatin1String("org.ukui.shortcut.bluetooth"));
    qmlRegisterModule(uri, 1, 0);
    qmlRegisterSingletonType<BluetoothInterface>(uri, 1, 0, "BtInterface", [] (QQmlEngine *, QJSEngine *) -> QObject* {
        BtInterface::instance();
        return BtInterface::getInstance();
    });
}

