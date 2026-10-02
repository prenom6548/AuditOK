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
#ifndef APKPAKCAGE_H
#define APKPAKCAGE_H

#include "Package.h"


class ApkPackage : public Package {

public:
    ApkPackage(QString debPath);
private:
    static void getApkInfo(QString debPath, QString &name, QString &version, QString &arch,QString &zhName);
private:
    virtual void initPackageInfo() override;
    virtual void initPackageInstallStatus() override;

};

#endif
