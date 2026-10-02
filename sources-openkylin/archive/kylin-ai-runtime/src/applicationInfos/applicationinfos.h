/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef APPLICATIONINFOS_H
#define APPLICATIONINFOS_H

#include <string>
#include <vector>

namespace appinfo {

struct ApplicationInfo {
    std::string desktopFile;
    std::string name;
    std::string comment;
};

using ApplicationInfos = std::vector<ApplicationInfo>;
ApplicationInfos getAllApplicationInfos();

}  // namespace appinfo

#endif  // APPLICATIONINFOS_H
