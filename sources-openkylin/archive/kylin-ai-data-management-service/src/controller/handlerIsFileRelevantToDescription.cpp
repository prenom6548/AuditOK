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

#include "fileInfoService.h"

Response handleIsFileRelevantToDescription(const Request& request) {
    PRINT_DEBUG("handleIsFileRelevantToDescription request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty()) {
        PRINT_ERROR("handleIsFileRelevantToDescription request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    if (!values.contains("file") || !values["file"].is_string() || !values.contains("description") ||
        !values["description"].is_string()) {
        PRINT_ERROR("handleIsFileRelevantToDescription request json format is error.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::string file = request.getString("file");
    std::string description = request.getString("description");
    if (file.empty() || description.empty()) {
        PRINT_ERROR("handleIsFileRelevantToDescription request format is wrong.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    SimilaritySearchResult result;
    ERROR_CODE retCode = FileInfoService().searchFilesByText(description, result);
    if (retCode != SUCCESS) {
        PRINT_ERROR("handleIsFileRelevantToDescription search files by text failed, ret: %d.\n", retCode);
        return Response(SEARCH_FILE_INFO_FAILED);
    }

    auto it = std::find_if(result.begin(), result.end(),
                           [&file](const std::pair<std::string, float>& element) { return element.first == file; });
    bool isRelevant = it != result.end();

    return Response::success(nlohmann::json{{"is_relevant", isRelevant}});
}

REGISTER_SERVICE_HANDLER("isFileRelevantToDescription", handleIsFileRelevantToDescription)
