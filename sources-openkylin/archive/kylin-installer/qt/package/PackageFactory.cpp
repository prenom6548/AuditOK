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
#include "PackageFactory.h"


QMap<QString, Package*> PackageFactory::packageMap;

// todo: 添加其他类型的包
Package* PackageFactory::createPackage(QString debPath, bool reuseExisting)
{
    if (reuseExisting && packageMap.contains(debPath)) {
//        qDebug() << "Returning existing package for path:" << debPath;
        return packageMap.value(debPath);
    }

    Package* package = nullptr;
    QFileInfo fileInfo(debPath);
    if (fileInfo.exists() && fileInfo.isFile()) {
        if (fileInfo.suffix() == "ok" || fileInfo.suffix() == "oab"){
            package = new KaimingPackage(debPath);
        }
        else if (fileInfo.suffix() == "deb"){
            package = new DebPackage(debPath);
        }
        else if (fileInfo.suffix() == "apk")
        {
            package = new ApkPackage(debPath);
        }

    }

    if (package) {
        // 暂不删除老对象，这个逻辑需要优化。目前会有内存泄漏
        // if (packageMap.contains(debPath)) {
        //     delete packageMap[debPath];  // 删除旧对象
        // }
        packageMap[debPath] = package;  // 存储新对象
        qDebug() << "Created new package for path:" << debPath;
    } else {
        qDebug() << "Failed to create package for path:" << debPath;
    }

    return package;
}
