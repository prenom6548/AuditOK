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

#include <algorithm>

#include "tagService.h"

Response handleIsFileRelevantToTags(const Request& request) {
    PRINT_DEBUG("handleIsFileRelevantToTags request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty()) {
        PRINT_ERROR("handleIsFileRelevantToTags request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    if (!values.contains("file") || !values["file"].is_string() || !values.contains("tags") ||
        !values["tags"].is_array()) {
        PRINT_ERROR("handleIsFileRelevantToTags request json format is error.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::string file = request.getString("file");
    std::vector<std::string> tags;
    for (const auto& item : values["tags"]) {
        if (!item.is_string()) {
            PRINT_ERROR("handleIsFileRelevantToTags request json tags key is error.\n");
            continue;
        }
        tags.push_back(item.get<std::string>());
    }

    if (file.empty() || tags.empty()) {
        PRINT_ERROR("handleIsFileRelevantToTags request format is wrong.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::vector<std::string> similarTagFilePaths = TagService().searchSimilarFilePaths(tags);
    if (similarTagFilePaths.empty()) {
        return Response::success(nlohmann::json{{"is_relevant", false}});
    }

    bool isRelevant =
        std::find(similarTagFilePaths.begin(), similarTagFilePaths.end(), file) != similarTagFilePaths.end();

    return Response::success(nlohmann::json{{"is_relevant", isRelevant}});
}

REGISTER_SERVICE_HANDLER("isFileRelevantToTags", handleIsFileRelevantToTags)
