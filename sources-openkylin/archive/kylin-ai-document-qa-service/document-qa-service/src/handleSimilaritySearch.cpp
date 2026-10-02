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

Response handleSimilaritySearch(const Request& request) {
    PRINT_DEBUG("handleSimilaritySearch request: %s\n", request.toString().c_str());

    nlohmann::json values = request.getValues();
    if (!values.is_object() || !values.contains("knowledgebase_name") || !values.contains("search_condition")) {
        PRINT_ERROR("handleSimilaritySearch request param error.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    std::string knowledgeBaseName = values["knowledgebase_name"];
    std::string searchCondition = values["search_condition"];

    std::string searchResult;
    KnowledgebaseService service;
    ERROR_CODE status = service.similaritySearch(knowledgeBaseName, searchCondition, searchResult);
    if (status != SUCCESS) {
        PRINT_ERROR("handleSimilaritySearch error.\n");
        return Response(status);
    }

    Response ret = Response::success();
    ret.add("search_result", searchResult);
    return ret;
}

REGISTER_SERVICE_HANDLER("similaritySearch", handleSimilaritySearch)