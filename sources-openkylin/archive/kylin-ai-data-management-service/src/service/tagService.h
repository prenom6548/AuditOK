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

#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "fileDatabase.h"

class TagService {
public:
    TagService() : db(FileDatabase::createDatabase()) {}

    std::vector<std::string> searchSimilarTagNames(const std::vector<std::string>& tags);

    std::vector<std::string> searchSimilarFilePaths(const std::vector<std::string>& tags);

    void deleteFileTags(const std::unordered_set<int64_t>& deletedFileIds);

    void updateFileTags(int64_t fileId, const std::set<std::string>& newTags);

private:
    static std::unordered_set<int64_t> searchSimilarTagIdsByTagNames(const std::vector<std::string>& tags);

    void deleteFileTags(int64_t fileId, const std::vector<int64_t>& tagIds);

private:
    SQLite::Database db;
};
