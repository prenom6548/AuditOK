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

Response handleExtractSearchConditions(const Request& request) {
    PRINT_DEBUG("handleExtractSearchConditions request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_object() || values.empty() || !values.contains("description") ||
        !values["description"].is_string()) {
        PRINT_ERROR("handleExtractSearchConditions request is not object of is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    auto description = values["description"].get<std::string>();

    nlohmann::json conditions;
    conditions["condition type"] = 3;
    conditions["orCondition"] = false;
    conditions["content"] = description;

    nlohmann::json labels(nlohmann::json::array());

    nlohmann::json root;
    root["0"] = conditions;
    root["conditions count"] = 1;
    root["labels"] = labels;

    return Response::success(root);
}

REGISTER_SERVICE_HANDLER("extractSearchConditions", handleExtractSearchConditions)
