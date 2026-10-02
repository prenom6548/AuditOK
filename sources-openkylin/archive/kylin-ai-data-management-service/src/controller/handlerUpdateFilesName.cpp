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

/**
 * 使用场景：当文件移动时，无需修改向量数据库内容，只需修改SQLITE数据库中的文件路径即可
 * 上游业务识别不出来移动，暂不实现
 * @param request
 * @return
 */
Response handleUpdateFilesName(const Request& request) {
    PRINT_DEBUG("handleUpdateFilesName request: %s\n", request.toString().c_str());
    return Response::success();
}

REGISTER_SERVICE_HANDLER("updateFilesName", handleUpdateFilesName)
