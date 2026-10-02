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
#ifndef PACKAGEUTILS_H
#define PACKAGEUTILS_H

#include "PackageEnum.h"
#include <QStringList>
#include <QCryptographicHash>

class PackageUtils
{
public:
    static QString getOsDistribution();
    static QStringList getDebDistribution(QString packagePath);
    static PackageEnum::Type getPackageType(QString packagePath);
    static QStringList getKareImageDistribution();
    static void getPackageEnumInfo(QString debPath, PackageEnum::Type &type, QString &distribution, bool &isInstalledBaseEnv);
    static QString getDebInfoFromDeb(QString debPath, QString type);
    static bool isInstalledFromDebFilePath(QString debpathstring);
    static bool isInstalledFromDpkg(QString packageName);
    static QString getMd5Sum(QString filePath);
    static QString getMd5SumPart(QString);
    static constexpr const char *s_packageVersionSplitKey = ",";
    static constexpr const char *s_packageVersionFileName = "_compat";
    static constexpr const char *s_osDistributionFilePath = "/etc/os-release";
};

#endif
