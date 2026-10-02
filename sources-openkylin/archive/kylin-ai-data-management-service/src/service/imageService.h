/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <unordered_set>

#include "fileDatabase.h"

class ImageService {
public:
    ImageService() : db(FileDatabase::createDatabase()) {}

    void embedding(const std::unordered_set<std::string>& imgFilePaths);

    ~ImageService() = default;

private:
    bool insertImageFileInfo(FileInfoDO& fileInfo, const std::string& modifyTime, const std::vector<float>& embedding);
    bool updateImageFileInfo(FileInfoDO& fileInfo, const std::string& modifyTime, const std::vector<float>& embedding);
    bool updateVectorDatabaseData(const FileInfoDO& fileInfo,const std::vector<float>& embedding);

private:
    SQLite::Database db;
};
