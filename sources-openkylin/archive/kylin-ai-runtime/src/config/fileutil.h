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

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>

class FileUtil {
public:
    FileUtil();

    bool isFileEmpty(const std::string &filePath);
    bool createDirectory(const std::string &fileDir);
    bool createFile(const std::string &filePath);
    bool createFileWriteContent(const std::string &filePath, const std::string &content);
    bool deleteFile(const std::string &filePath);
};

#endif  // FILEUTIL_H
