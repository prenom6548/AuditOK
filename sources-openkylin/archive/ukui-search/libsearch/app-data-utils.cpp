/*
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: kirito <baijunjie@kylinos.cn>
 */

#include "app-data-utils.h"
#include <QFile>
#include <QDir>

using namespace UkuiSearch;

static const QString ALL_APP_DIRS_CONF = "/etc/ukui/ukui-search/application-dirs.conf";
#define ANDROID_APP_DESKTOP_PATH QDir::homePath() + "/.local/share/applications"


QStringList AppDataUtils::getDesktopPaths() {
    QStringList paths;
    // 1.读取环境变量中指定的目录
    for (const QString& path : QString(getenv("XDG_DATA_DIRS")).split(":", Qt::SkipEmptyParts)) {
        paths.append(QDir::cleanPath(path + "/applications"));
    }
    // 2.读取配置文件中的目录
    QFile file(ALL_APP_DIRS_CONF);
    file.open(QIODevice::ReadOnly);
    QString content = file.readAll();
    file.close();
    paths.append(content.split(",", Qt::SkipEmptyParts));
    // 3.添加特殊目录
    paths.append(ANDROID_APP_DESKTOP_PATH);
    paths.removeDuplicates();
    return paths;
}