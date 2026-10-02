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

#include "processorWrapper.h"

#include "log.h"
#include "serviceResponse/serviceResponse.h"

scene::ProcessorWrapper::ProcessorWrapper(const std::string& serviceName, GDBusConnection* connection)
    : BaseProcessor(serviceName, connection) {
    this->delegate = scene::getDelegate(serviceName);
}

void scene::ProcessorWrapper::syncResponse(const std::string& method, GDBusMethodInvocation* invocation,
                                           const std::string& output) {
    if (delegate == nullptr) {
        PRINT_ERROR("ProcessorWrapper::syncResponse delegate is null.\n");
        return;
    }
    scene::serviceSyncResponse(serviceName, method, delegate, invocation, output);
}

void scene::ProcessorWrapper::asyncResponse(const std::shared_ptr<handlerTask>& task, const std::string& output) {
    if (task->delegate == nullptr) {
        PRINT_ERROR("ProcessorWrapper::asyncResponse delegate is null.\n");
        return;
    }
    scene::serviceAsyncResponse(task->dispatchData->processor->serviceName, task->dispatchData->method, task->delegate,
                                task->invocation, output);
}

void scene::ProcessorWrapper::asyncFlowResponse(const std::shared_ptr<handlerTask>& task, const std::string& output) {
    GVariant* result = g_variant_new("(s)", output.c_str());
    scene::serviceAsyncFlowResponse(task->dispatchData->processor->serviceName, task->dispatchData->method,
                                    task->dispatchData->processor->connection, task->delegate, task->invocation, result,
                                    true);
}
