/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "customplugin.h"
#include "screensaver.h"

CustomPlugin::CustomPlugin(QObject *parent) : QObject(parent) {}

QString CustomPlugin::name() const
{
    return "screensaver-default";
}

QWidget *CustomPlugin::createWidget(bool isScreensaver, QWidget *parent)
{
    return new Screensaver(isScreensaver, parent);
}

QString CustomPlugin::displayName() const
{
    return "screensaver-default";
}
