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

#include <scene.h>

#include "fileDatabase.h"

Response handleGetSummarysOfFiles(const Request& request) {
    PRINT_DEBUG("handleGetSummarysOfFiles request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty()) {
        PRINT_ERROR("handleGetSummarysOfFiles request is not object or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    if (!values.contains("files") || !values["files"].is_array()) {
        PRINT_ERROR("handleGetSummarysOfFiles request json is error.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    std::unordered_set<std::string> files;
    for (const auto& item : values["files"]) {
        if (!item.is_string()) {
            PRINT_ERROR("handleGetSummarysOfFiles request json format is error.\n");
            continue;
        }
        files.insert(item.get<std::string>());
    }

    if (files.empty()) {
        PRINT_ERROR("handleGetSummarysOfFiles request format is wrong.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    SQLite::Database database = FileDatabase::createDatabase();
    nlohmann::json root(nlohmann::json::array());
    for (const auto& file : files) {
        std::string summary = FileDatabase::getFileSummary(database, file);
        nlohmann::json value;
        value["filepath"] = file;
        value["summary"] = summary;
        root.push_back(value);
    }

    return Response::success(root);
}

REGISTER_SERVICE_HANDLER("getSummarysOfFiles", handleGetSummarysOfFiles)
