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

#ifndef UTILS_CONFIG_H_
#define UTILS_CONFIG_H_

#include <QStringList>

class UtilsConfig final
{
public:
    UtilsConfig() = default;
    ~UtilsConfig() = default;

    /**
     * @brief 依据百宝箱配置文件 , 将百宝箱的 desktop 文件复制到桌面上
     *
     * @param 无
     *
     * @return 无
     */
    static void installToolBoxDesktop(void);

private:
    static void saveInstallIcon(QString name);
    static QStringList getInstallIcon(void);
};

#endif