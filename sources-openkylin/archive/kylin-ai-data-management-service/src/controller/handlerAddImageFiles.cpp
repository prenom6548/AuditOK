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

#include "imageService.h"

Response handleAddImageFiles(const Request& request) {
    PRINT_DEBUG("HandleAddImageFiles request: %s\n", request.toString().c_str());

    // 解析参数
    nlohmann::json values = request.getValues();
    if (!values.is_array() || values.empty()) {
        PRINT_ERROR("HandleAddImageFiles request is not array or is empty.\n");
        return Response(PARSE_PARAM_FAILED);
    }
    std::unordered_set<std::string> filePaths;
    for (const auto& item : values) {
        if (!item.is_object() || !item.contains("filepath") || !item["filepath"].is_string() ||
            !item.contains("fileformat") || !item["fileformat"].is_string()) {
            PRINT_ERROR("HandleAddImageFiles request json is error.\n");
            continue;
        }
        filePaths.insert(item["filepath"].get<std::string>());
    }

    if (!filePaths.empty()) {
        ImageService().embedding(filePaths);
    }

    return Response::success();
}

REGISTER_SERVICE_HANDLER("addImageFiles", handleAddImageFiles)
