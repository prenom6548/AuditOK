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

Response handleSimilaritySearch(const Request& request) {
    PRINT_DEBUG("handleSimilaritySearch request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty()) {
        PRINT_ERROR("handleSimilaritySearch request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    if (!values.contains("text") || !values["text"].is_string()) {
        PRINT_ERROR("handleSimilaritySearch request json format is error.\n");
        return Response(PARSE_PARAM_FAILED);
    }

    std::string text = request.getString("text");
    if (text.empty()) {
        PRINT_ERROR("handleSimilaritySearch request text is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    // double threshold = request.getDouble("similarity-threshold"); 目前没用传过来的阈值

    SimilaritySearchResult result;
    ERROR_CODE retCode = FileInfoService().searchFilesByText(text, result);
    if (retCode != SUCCESS) {
        PRINT_ERROR("handleSimilaritySearch search files by text failed, ret: %d.\n", retCode);
        return Response(SEARCH_FILE_INFO_FAILED);
    }

    nlohmann::json root(nlohmann::json::array());
    for (const auto& info : result) {
        nlohmann::json file;
        file["filepath"] = info.first;
        file["similarity"] = info.second;
        root.push_back(file);
    }

    return Response::success(root);
}

REGISTER_SERVICE_HANDLER("similaritySearch", handleSimilaritySearch)
