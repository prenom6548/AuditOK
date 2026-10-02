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
#ifndef SYSTEMINFOUTIL_H
#define SYSTEMINFOUTIL_H
#include <QDir>
#include <QString>
#include <kysdk/kysdk-system/libkypackages.h>
#include <QDirIterator>

class SystemInfoUtil
{
public:
    static bool getOsTreeStatus();
    static QString getValidFilePathFromV11(QString fileType,QString fileName);

    static bool isOsTree;
    static constexpr const char *INSTALL_LIB_PATH = "INSTALL_LIB_PATH";
    static constexpr const char *KMRE_CONF_PATH="KMRE_CONF_PATH";
};

#endif
