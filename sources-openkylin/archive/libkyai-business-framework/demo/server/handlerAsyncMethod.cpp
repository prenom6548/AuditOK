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

#include <thread>

#include "scene.h"

void workMethod2(const std::string &taskId) {
    PRINT_INFO("workMethod2 start, input: %s.\n", taskId.c_str());
    sleep(10);
    Response output = Response::success();
    output.add("p1", 11);
    output.add("p2", "hehe");
    ASYNC_RESPONSE(taskId, output);
    PRINT_INFO("workMethod2 end.\n");
}

Response handleAsyncMethod(const std::string &taskId, const Request &request) {
    PRINT_DEBUG("method2 taskId: %s, request: %s\n", taskId.c_str(), request.toString().c_str());

    // 异步处理
    std::thread t(workMethod2, taskId);
    t.detach();

    return Response::success();
}

REGISTER_SERVICE_ASYNC_HANDLER("asyncMethod", handleAsyncMethod)
