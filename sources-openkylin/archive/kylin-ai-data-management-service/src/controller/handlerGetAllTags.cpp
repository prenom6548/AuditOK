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

Response handleGetAllTags(const Request& request) {
    PRINT_DEBUG("handleGetAllTags request: %s\n", request.toString().c_str());

    SQLite::Database database = FileDatabase::createDatabase();
    std::vector<std::string> allTags = FileDatabase::getAllTags(database);

    return Response::success(allTags);
}

REGISTER_SERVICE_HANDLER("getAllTags", handleGetAllTags)
