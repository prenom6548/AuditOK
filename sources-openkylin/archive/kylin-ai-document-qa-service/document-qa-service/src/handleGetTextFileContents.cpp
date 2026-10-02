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

#include "service.h"

Response handleGetTextFileContents(const Request& request) {
    PRINT_DEBUG("handleGetTextFileContents request: %s\n", request.toString().c_str());

    nlohmann::json values = request.getValues();
    if (!values.is_array() || values.empty()) {
        PRINT_ERROR("handleGetTextFileContents request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    std::vector<std::string> filepaths;
    for (const auto& item : values) {
        if (!item.is_object() || !item.contains("filepath") || !item["filepath"].is_string()) {
            PRINT_ERROR("handleGetTextFileContents request json is error.\n");
            continue;
        }
        filepaths.emplace_back(item["filepath"].get<std::string>());
    }

    KnowledgebaseService service;
    std::string fileContent;
    ERROR_CODE status = service.getFileContent(filepaths, fileContent);
    if (status != SUCCESS) {
        PRINT_ERROR("handleGetTextFileContents getFileContent error.\n");
        return Response(status);
    }

    Response ret = Response::success();
    ret.add("file_content", fileContent);

    return ret;
}

REGISTER_SERVICE_HANDLER("getTextFileContents", handleGetTextFileContents)