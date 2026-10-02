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

void workMethod3(const std::string &taskId) {
    PRINT_INFO("workMethod3 start, input: %s.\n", taskId.c_str());
    sleep(10);
    Response output = Response::success();
    output.add("p1", 11);
    output.add("p2", "hehe");
    output.add("task", taskId);
    ASYNC_FLOW_RESPONSE_CONTINUE(taskId, output);
    PRINT_INFO("workMethod3 first call.\n");

    output.clear();
    sleep(10);
    output.add("p3", 22);
    output.add("p4", "hello");
    output.add("task", taskId);
    ASYNC_FLOW_RESPONSE_CONTINUE(taskId, output);
    PRINT_INFO("workMethod3 second call.\n");

    output = Response(ASYNC_FLOW_END);
    sleep(10);
    output.add("p5", 33);
    output.add("p6", "world");
    output.add("task", taskId);
    ASYNC_FLOW_RESPONSE_END(taskId, output);
    PRINT_INFO("workMethod3 end.\n");
}

Response handleAsyncFlowMethod(const std::string &taskId, const Request &request) {
    PRINT_DEBUG("method3 taskId: %s, request: %s\n", taskId.c_str(), request.toString().c_str());

    // 异步处理
    std::thread t(workMethod3, taskId);
    t.detach();

    return Response::success();
}

REGISTER_SERVICE_ASYNC_FLOW_HANDLER("asyncFlowMethod", handleAsyncFlowMethod)
