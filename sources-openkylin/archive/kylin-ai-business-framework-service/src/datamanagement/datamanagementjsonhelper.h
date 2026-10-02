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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENTJSONHELPER_H
#define DATAMANAGEMENTJSONHELPER_H

#include <string>
#include <vector>

class DataManagementJsonHelper
{
public:
    static std::pair<std::string, double> parserSearchConditions(const std::string& searchConditionsJson);
    static std::vector<std::pair<std::string, std::string>> parserFilePathAndFormat(const std::string& fileinfos);
    static std::vector<std::string> parserDeletedFileInfos(const std::string& fileinfos);
    static std::string convertSearchResultToJson(const std::vector<std::pair<std::string, double>>& searchResult);
    static std::string convertFileInfosResultToJson(const std::vector<std::pair<std::string, int>>& fileinfos);
};

#endif // DATAMANAGEMENTJSONHELPER_H
