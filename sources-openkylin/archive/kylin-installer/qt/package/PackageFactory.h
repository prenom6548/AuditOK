/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef PACKAGEFACTORY_H
#define PACKAGEFACTORY_H

#include "Package.h"
#include "KaimingPackage.h"
#include "DebPackage.h"
#include "ApkPackage.h"
#include <QDebug>
#include <QFileInfo>

class PackageFactory
{
public:
    static Package* createPackage(QString debPath, bool reuseExisting = true);
private:
    static QMap<QString, Package*> packageMap;
};


#endif
