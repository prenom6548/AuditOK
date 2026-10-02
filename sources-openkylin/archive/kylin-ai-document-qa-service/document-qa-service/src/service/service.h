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
#include <kyai-business-framework/errorCode.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "database.h"

class KnowledgebaseService {
public:
    ERROR_CODE addTexts(const std::string& collectionName, const std::string& filepath);
    ERROR_CODE getFileContent(const std::vector<std::string>& filepaths, std::string& output);
    ERROR_CODE similaritySearch(const std::string& collectionName, const std::string& searchCondition,
                                std::string& output) const;

private:
    std::string getFileContentJson(std::unordered_map<std::string, std::string>& fileContents) const;
    std::string getSearchResultJson(std::vector<KnowledgebaseSearchResult>& searchResult) const;
};