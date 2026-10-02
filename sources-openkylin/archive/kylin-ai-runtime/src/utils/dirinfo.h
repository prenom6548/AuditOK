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

#ifndef UTILS_DIR_INFO_H
#define UTILS_DIR_INFO_H

#include <filesystem>

#include "utils/fileinfolist.h"

namespace util {

class DirInfo {
public:
    DirInfo();
    explicit DirInfo(const std::filesystem::path &dir);
    ~DirInfo();

    FileInfoList entryInfoList();

    bool exists();

private:
    std::filesystem::path dir_{};
};
}  // namespace util

#endif
