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

enum FeatureStatus : int { UNKNOWN = -1, AVAILABLE = 0, NOT_INSTALLED, NOT_SUPPORTED };

Response handleGetFeatureStatus(const Request& request) {
    // 模型默认集成
    return Response::success(nlohmann::json{{"result", AVAILABLE}});
}

REGISTER_SERVICE_HANDLER("getFeatureStatus", handleGetFeatureStatus)
