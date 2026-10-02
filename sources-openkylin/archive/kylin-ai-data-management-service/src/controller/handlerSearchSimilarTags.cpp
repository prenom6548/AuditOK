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

#include "tagService.h"

Response handleSearchSimilarTags(const Request& request) {
    PRINT_DEBUG("handleSearchSimilarTags request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty()) {
        PRINT_ERROR("handleSearchSimilarTags request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    if (!values.contains("tags") || !values["tags"].is_array()) {
        PRINT_ERROR("handleSearchSimilarTags request json format is error.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::vector<std::string> tags;
    for (const auto& item : values["tags"]) {
        if (!item.is_string()) {
            PRINT_ERROR("handleSearchSimilarTags request json tags key is error.\n");
            continue;
        }
        tags.push_back(item.get<std::string>());
    }

    if (tags.empty()) {
        PRINT_ERROR("handleSearchSimilarTags request format is wrong.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::vector<std::string> similarTags = TagService().searchSimilarTagNames(tags);

    return Response::success(similarTags);
}

REGISTER_SERVICE_HANDLER("searchSimilarTags", handleSearchSimilarTags)
