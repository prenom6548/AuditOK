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

#include "dirinfo.h"

namespace util {

DirInfo::DirInfo() = default;

DirInfo::DirInfo(const std::filesystem::path &dir) : dir_{dir} {}

DirInfo::~DirInfo() = default;

FileInfoList DirInfo::entryInfoList() {
    // 先满足需求，后续再完善
    // 先目录，后文件
    FileInfoList fileInfoList;
    for (const auto &entry : std::filesystem::directory_iterator(dir_)) {
        if (entry.is_directory()) {
            fileInfoList.append(FileInfo{entry.path()});
        }
    }
    for (const auto &entry : std::filesystem::directory_iterator(dir_)) {
        if (!entry.is_directory()) {
            fileInfoList.append(FileInfo{entry.path()});
        }
    }
    fileInfoList.sort();
    return fileInfoList;
}

bool DirInfo::exists() { return std::filesystem::exists(dir_); }

}  // namespace util
