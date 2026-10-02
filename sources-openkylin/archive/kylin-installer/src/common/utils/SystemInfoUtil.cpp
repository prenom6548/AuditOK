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
#include "SystemInfoUtil.h"

bool SystemInfoUtil::getOsTreeStatus()
{
    QString directoryPath = "/ostree";
    QString directoryPath2 = "/sysroot";

    QDir dir(directoryPath);
    QDir dir2(directoryPath2);
    if (dir.exists(directoryPath) && dir.exists(directoryPath) && QFile::exists("/etc/ostree-release")) {
        return true;
    }
    return false;
}

QString SystemInfoUtil::getValidFilePathFromV11(QString fileType,QString fileName)
{
    QString validFilePath;
    char **path = kdk_package_get_standard_path(fileType.toStdString().c_str());
    if (path == nullptr) {
        return validFilePath;
    }
    int num = 0;
    while (path[num]) {
        QString dirPath = QString(path[num]);
        QDirIterator it(dirPath,QDir::Files,QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFileInfo fi(it.next());
            if (fi.fileName() == fileName)
            {
                validFilePath = fi.absoluteFilePath();
                break;
            }
        }

        num++;
    }
    return validFilePath;

}

bool SystemInfoUtil::isOsTree = SystemInfoUtil::getOsTreeStatus();
