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

Response handleGetAllFileInfos(const Request& request) {
    PRINT_DEBUG("handleGetAllFileInfos request: %s\n", request.toString().c_str());

    SQLite::Database database = FileDatabase::createDatabase();
    std::vector<FileInfoDO> fileInfos = FileDatabase::getAllFileInfos(database);

    nlohmann::json root(nlohmann::json::array());
    for (const auto& info : fileInfos) {
        nlohmann::json file;
        file["filepath"] = info.filePath;
        file["timestamp"] = info.modifyTime;
        root.push_back(file);
    }

    return Response::success(root);
}

REGISTER_SERVICE_HANDLER("getAllFileInfos", handleGetAllFileInfos)
