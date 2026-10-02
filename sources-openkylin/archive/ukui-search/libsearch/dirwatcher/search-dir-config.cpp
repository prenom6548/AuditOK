/*
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
 * Authors: maomao <baijunjie@kylinos.cn>
 *
 */

#include "search-dir-config.h"
#include "dir-watcher.h"
#include <QDebug>

using namespace UkuiSearch;

SearchDirConfig::SearchDirConfig(QObject *parent) : QObject(parent)
{
    connect(DirWatcher::getDirWatcher(), &DirWatcher::blockItemAdded, this, &SearchDirConfig::blockItemAdded);
    connect(DirWatcher::getDirWatcher(), &DirWatcher::blockItemRemoved, this, &SearchDirConfig::blockItemRemoved);
}

QStringList SearchDirConfig::getBlockDirOfUser() {
    return DirWatcher::getDirWatcher()->getBlockDirsOfUser();
}
