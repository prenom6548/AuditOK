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

#ifndef UTILS_FILE_INFO_LIST_H
#define UTILS_FILE_INFO_LIST_H

#include "fileinfo.h"

namespace util {

class FileInfoList {
public:
    FileInfoList() = default;

    explicit FileInfoList(const std::filesystem::path &dir, bool dirsFirst = true);

    bool removeAll(const FileInfo &fileInfo);

    size_t size() const;

    FileInfo at(size_t index) const;

    void append(const FileInfo &fileInfo);

    void sort();

private:
    std::vector<FileInfo> fileInfoList_;
};

}  // namespace util

#endif
