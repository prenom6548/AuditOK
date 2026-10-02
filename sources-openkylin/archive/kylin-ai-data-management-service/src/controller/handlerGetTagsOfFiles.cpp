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

Response handleGetTagsOfFiles(const Request& request) {
    PRINT_DEBUG("handleGetTagsOfFiles request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty()) {
        PRINT_ERROR("handleGetTagsOfFiles request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    if (!values["files"].is_array()) {
        PRINT_ERROR("handleGetTagsOfFiles request json format is error.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    std::vector<std::string> files;
    for (const auto& item : values["files"]) {
        if (!item.is_string()) {
            PRINT_ERROR("handleGetTagsOfFiles request json files key is error.\n");
            continue;
        }
        files.push_back(item.get<std::string>());
    }

    if (files.empty()) {
        PRINT_ERROR("handleGetTagsOfFiles request format is wrong.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    nlohmann::json output(nlohmann::json::array());
    SQLite::Database database = FileDatabase::createDatabase();
    for (const auto& file : files) {
        std::vector<std::string> fileTags = FileDatabase::getFileTags(database, file);
        nlohmann::json tags = fileTags;

        nlohmann::json value;
        value["filepath"] = file;
        value["tags"] = tags;
        output.push_back(value);
    }

    return Response::success(output);
}

REGISTER_SERVICE_HANDLER("getTagsOfFiles", handleGetTagsOfFiles)
