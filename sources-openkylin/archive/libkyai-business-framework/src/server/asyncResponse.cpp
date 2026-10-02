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

#include "asyncResponse.h"

#include "baseProcessor.h"
#include "log.h"
#include "serviceResponse/serviceResponse.h"

bool scene::asyncResponse(const std::string& taskId, const Response& response) {
    std::lock_guard<std::mutex> lock(scene::BaseProcessor::asyncTaskMapMutex);
    auto it = scene::BaseProcessor::asyncTaskMap.find(taskId);
    if (it == scene::BaseProcessor::asyncTaskMap.end()) {
        PRINT_ERROR("asyncResponse cannot find async task: %s.\n", taskId.c_str());
        return false;
    }

    auto task = it->second;
    scene::serviceAsyncResponse(task->dispatchData->processor->serviceName, task->dispatchData->method, task->delegate,
                                task->invocation, response.toString());

    scene::BaseProcessor::asyncTaskMap.erase(it);
    task->dispatchData->processor->deleteTaskIds(taskId);
    return true;
}

bool scene::asyncFlowResponse(const std::string& taskId, const Response& response, bool end) {
    std::lock_guard<std::mutex> lock(scene::BaseProcessor::asyncTaskMapMutex);
    auto it = scene::BaseProcessor::asyncTaskMap.find(taskId);
    if (it == scene::BaseProcessor::asyncTaskMap.end()) {
        PRINT_ERROR("asyncFlowResponse cannot find async task: %s.\n", taskId.c_str());
        return false;
    }

    auto task = it->second;
    GVariant* result = g_variant_new("(s)", response.toString().c_str());
    scene::serviceAsyncFlowResponse(task->dispatchData->processor->serviceName, task->dispatchData->method,
                                    task->dispatchData->processor->connection, task->delegate, task->invocation, result,
                                    end);
    if (end) {
        scene::BaseProcessor::asyncTaskMap.erase(it);
        task->dispatchData->processor->deleteTaskIds(taskId);
    }
    return true;
}
