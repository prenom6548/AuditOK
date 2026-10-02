/*
 * kylin-os-manager
 *
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
 */

#include <QString>
#include <QDebug>
#include <QDir>
#include <QDebug>

#include "utils.h"
#include "utils-config.h"

void createCacheDir(void)
{
    QString path = Utils::getSelfCachePath();

    QDir dir;
    if (!dir.exists(path)) {
        if (!dir.mkpath(path)) {
            qCritical() << "create self cache dir fail !";
            return;
        }

        qInfo() << "create self cache dir success !";
    }

    return;
}

int main(void)
{
    /* 创建自身缓目录 */
    createCacheDir();

    /* 配置工具箱安装图标到桌面 */
    UtilsConfig::installToolBoxDesktop();

    return 0;
}