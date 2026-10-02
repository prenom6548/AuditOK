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

#include "fileinfolist.h"

#include <algorithm>

namespace util {

FileInfoList::FileInfoList(const std::filesystem::path &path, bool isDirsFirst) {
    if (isDirsFirst) {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                fileInfoList_.emplace_back(entry.path());
            }
        }
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (!entry.is_directory()) {
                fileInfoList_.emplace_back(entry.path());
            }
        }
    } else {
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            fileInfoList_.emplace_back(entry.path());
        }
    }
}

bool FileInfoList::removeAll(const FileInfo &fileInfo) {
    if (fileInfoList_.empty()) {
        return true;
    }

    const auto iter = std::remove_if(fileInfoList_.begin(), fileInfoList_.end(),
                                     [&fileInfo](const FileInfo &theFileInfo) { return theFileInfo == fileInfo; });
    if (iter == fileInfoList_.end()) {
        return false;
    }
    fileInfoList_.erase(iter, fileInfoList_.end());
    return true;
}

size_t FileInfoList::size() const { return fileInfoList_.size(); }

FileInfo FileInfoList::at(size_t index) const {
    if (index >= fileInfoList_.size()) {
        return FileInfo{};
    }
    return fileInfoList_.at(index);
}

void FileInfoList::append(const FileInfo &fileInfo) { fileInfoList_.emplace_back(fileInfo); }

void FileInfoList::sort() { std::sort(fileInfoList_.begin(), fileInfoList_.end()); }

}  // namespace util
