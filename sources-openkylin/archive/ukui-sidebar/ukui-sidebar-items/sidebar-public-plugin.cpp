/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "sidebar-public-plugin.h"
#include "theme-palette.h"
#include "theme-icon.h"
#include "desktop-background.h"
#include "image-item.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>

void SidebarPublicPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));

    qmlRegisterType<ImageItem>(uri, 1, 0, "ImageItem");
    qmlRegisterType<ThemeIcon>(uri, 1, 0, "ThemeIcon");
    qmlRegisterType<DesktopBackground>(uri, 1, 0, "DesktopBackground");
    qmlRegisterUncreatableType<PaletteRole>(uri, 1, 0, "PaletteRole", "Only enumeration variables are required");
    qmlRegisterUncreatableType<BackgroundType>(uri, 1, 0, "BackgroundType", "Only enumeration variables are required");

    qRegisterMetaType<BackgroundType::Type>("BackgroundType::Type");
    qRegisterMetaType<PaletteRole::ColorGroup>("PaletteRole::ColorGroup");
    qRegisterMetaType<PaletteRole::ColorRole>("PaletteRole::ColorRole");
}

void SidebarPublicPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String(PLUGIN_IMPORT_URI));

    engine->rootContext()->setContextProperty("themePalette", ThemePalette::getInstance());
}
