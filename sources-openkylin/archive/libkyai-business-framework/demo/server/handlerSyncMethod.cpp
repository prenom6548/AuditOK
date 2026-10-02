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

#include <unistd.h>

#include "scene.h"

Response handleSyncMethod(const Request& request) {
    PRINT_DEBUG("method1 request: %s\n", request.toString().c_str());
    const std::string& input1 = request.getString("input1");
    PRINT_INFO("method1 request input1: %s\n", input1.c_str());

    // sleep(30);
    return Response::success(nlohmann::json{
        {"aa", 11},
        {"bb", "asdfg"},
        {"cc", 33},
    });
}

REGISTER_SERVICE_HANDLER("syncMethod", handleSyncMethod)
