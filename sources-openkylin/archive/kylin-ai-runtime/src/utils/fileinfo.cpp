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

#include "fileinfo.h"

namespace util {

FileInfo::FileInfo(const std::filesystem::path& path) : path_{path} {}

bool FileInfo::isDir() const { return std::filesystem::is_directory(path_); }

bool FileInfo::isFile() const { return std::filesystem::is_regular_file(path_); };

std::string FileInfo::filePath() { return path_.string(); }

std::string FileInfo::extension() { return path_.extension().string(); }

std::string FileInfo::fileName() { return path_.filename().string(); }

bool FileInfo::operator==(const FileInfo& other) const { return path_ == other.path_; }

bool FileInfo::operator<(const FileInfo& other) const { return path_ < other.path_; }

}  // namespace util
