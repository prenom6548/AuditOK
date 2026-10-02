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

#include "baseProcessor.h"

#include "constant.h"
#include "handler.h"
#include "log.h"
#include "processorWrapper.h"
#include "response.h"
#include "utils.h"

std::mutex scene::BaseProcessor::asyncTaskMapMutex;
std::map<std::string, std::shared_ptr<scene::HandlerTask>> scene::BaseProcessor::asyncTaskMap;

scene::BaseProcessor::BaseProcessor(const std::string& serviceName, GDBusConnection* connection)
    : serviceName(serviceName), connection(connection), closedHandlerId(0), isExported(false), delegate(nullptr) {}

void scene::BaseProcessor::connectSignal() {
    // 注册同步接口信号
    for (const auto& item : ServiceRegister::methodHandlerMap) {
        PRINT_DEBUG("connect signal: %s\n", item.first.c_str());
        std::string signal = SINGLE_PRE + Utils::camelCaseToKebab(item.first);
        auto data = std::make_shared<DispatchData>();
        data->method = item.first;
        data->type = SYNC_METHOD;
        data->workHandler = reinterpret_cast<void*>(item.second);
        data->processor = this;
        dispatchDataVec.push_back(data);
        g_signal_connect(delegate, signal.c_str(), G_CALLBACK(dispatchRequest), data.get());
    }

    // 注册异步接口信号
    for (const auto& item : ServiceRegister::asyncMethodHandlerMap) {
        PRINT_DEBUG("connect async signal: %s\n", item.first.c_str());
        std::string signal = SINGLE_PRE + Utils::camelCaseToKebab(item.first);
        auto data = std::make_shared<DispatchData>();
        data->method = item.first;
        data->type = ASYNC_METHOD;
        data->workHandler = reinterpret_cast<void*>(item.second);
        data->processor = this;
        dispatchDataVec.push_back(data);
        g_signal_connect(delegate, signal.c_str(), G_CALLBACK(dispatchRequest), data.get());
    }

    // 注册流式异步返回接口信号
    for (const auto& item : ServiceRegister::asyncFlowMethodHandlerMap) {
        PRINT_DEBUG("connect async flow signal: %s\n", item.first.c_str());
        std::string signal = SINGLE_PRE + Utils::camelCaseToKebab(item.first);
        auto data = std::make_shared<DispatchData>();
        data->method = item.first;
        data->type = ASYNC_FLOW_METHOD;
        data->workHandler = reinterpret_cast<void*>(item.second);
        data->processor = this;
        dispatchDataVec.push_back(data);
        g_signal_connect(delegate, signal.c_str(), G_CALLBACK(dispatchRequest), data.get());
    }
}

void scene::BaseProcessor::exportSkeleton() {
    std::string objectPath = OBJECT_PATH_PRE + serviceName;
    PRINT_DEBUG("object path is %s\n", objectPath.c_str());

    GError* error = nullptr;
    isExported =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate), connection, objectPath.c_str(), &error);
    if (!isExported) {
        if (error != nullptr) {
            PRINT_ERROR("Error creating server at address %s: %s\n", objectPath.c_str(), error->message);
            g_error_free(error);
        }
    }
}

/**
 * 处理异步接口请求
 * @param dispatchData
 * @param task
 */
void handlerAsyncMethod(scene::DispatchData* dispatchData, const std::shared_ptr<scene::handlerTask>& task) {
    auto asyncHandler = reinterpret_cast<scene::AsyncWorkHandler>(dispatchData->workHandler);
    // 调用业务逻辑
    Response ret = asyncHandler(task->id, Request(task->request));
    if (ret.isSuccess()) {
        // 异步任务正常开始处理，将任务放入队列中，方便业务逻辑回调返回结果
        std::lock_guard<std::mutex> lock(scene::BaseProcessor::asyncTaskMapMutex);
        scene::BaseProcessor::asyncTaskMap.insert(std::make_pair(task->id, task));
        dispatchData->processor->insertTaskIds(task->id);
        PRINT_DEBUG("workHandler save async task: %s\n", task->id.c_str());
    } else {
        // 异步任务未正常开始处理，直接返回失败结果
        if (dispatchData->type == scene::ASYNC_METHOD) {
            // 异步请求返回失败，直接返回结果
            scene::ProcessorWrapper::asyncResponse(task, ret.toString());
        } else {
            // 异步流式请求返回失败，直接返回结果
            scene::ProcessorWrapper::asyncFlowResponse(task, ret.toString());
        }
        PRINT_WARN("workHandler async task failed, %s\n", ret.toString().c_str());
    }
}

void scene::BaseProcessor::workHandler() {
    // 从任务队列中消费任务的线程
    workThread = std::thread([this]() {
        PRINT_DEBUG("workHandler start\n");
        while (isExported) {
            std::unique_lock<std::mutex> lock(taskMutex);
            PRINT_DEBUG("workHandler get lock\n");
            taskQueueCV.wait(lock, [this]() { return !taskQueue.empty() || !isExported; });

            // 连接断开时，退出线程
            if (!isExported) {
                while (!taskQueue.empty()) {
                    taskQueue.pop();
                }
                PRINT_INFO("workHandler end.\n");
                break;
            }

            auto task = taskQueue.front();
            taskQueue.pop();
            DispatchData* dispatchData = task->dispatchData;
            PRINT_DEBUG("workHandler get task id: %s, method: %s, type: %d\n", task->id.c_str(),
                        dispatchData->method.c_str(), dispatchData->type);

            if (dispatchData->type == SYNC_METHOD) {
                // 处理同步接口请求
                auto syncHandler = reinterpret_cast<SyncWorkHandler>(dispatchData->workHandler);
                // 调用业务逻辑
                Response ret = syncHandler(Request(task->request));
                // 连接异常断开
                if (!isExported) {
                    PRINT_ERROR("workHandler is not exported.\n");
                    break;
                }
                // 返回业务逻辑结果
                dispatchData->processor->syncResponse(dispatchData->method, task->invocation, ret.toString());
                PRINT_DEBUG("workHandler response: %s\n", ret.toString().c_str());
            } else if (dispatchData->type == ASYNC_METHOD || dispatchData->type == ASYNC_FLOW_METHOD) {
                // 处理异步接口请求
                handlerAsyncMethod(dispatchData, task);
            } else {
                PRINT_ERROR("workHandler unknown task id: %s, method: %s, type: %d\n", task->id.c_str(),
                            dispatchData->method.c_str(), dispatchData->type);
            }
        }
    });
}

bool scene::BaseProcessor::pushTask(const std::shared_ptr<HandlerTask>& task) {
    // 检查当前队列长度
    if (taskQueue.size() > TASK_QUEUE_MAX_SIZE) {
        PRINT_ERROR("dispatchRequest queue size larger than %d.\n", TASK_QUEUE_MAX_SIZE);
        return false;
    }

    // 将任务入队，唤醒消费线程
    std::unique_lock<std::mutex> lock(taskMutex);
    taskQueue.push(task);
    taskQueueCV.notify_all();
    PRINT_INFO("pushTask success, taskQueue size: %lu\n", taskQueue.size());
    return true;
}

void scene::BaseProcessor::insertTaskIds(const std::string& taskId) {
    asyncTaskIds.insert(taskId);
}

void scene::BaseProcessor::deleteTaskIds(const std::string& taskId) {
    asyncTaskIds.erase(taskId);
}

void scene::BaseProcessor::deleteAsyncTaskIds() {
    if (asyncTaskIds.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(asyncTaskMapMutex);
    for (const auto& item : asyncTaskIds) {
        auto it = asyncTaskMap.find(item);
        if (it == asyncTaskMap.end()) {
            PRINT_ERROR("BaseProcessor deleteAsyncTaskIds cannot find async task: %s.\n", item.c_str());
            continue;
        }
        asyncTaskMap.erase(it);
    }
}

scene::BaseProcessor::~BaseProcessor() {
    PRINT_DEBUG("BaseProcessor clear.\n");
    deleteAsyncTaskIds();
    dispatchDataVec.clear();

    if (delegate == nullptr) {
        return;
    }

    PRINT_DEBUG("BaseProcessor delegate is not null.\n");
    if (isExported) {
        PRINT_DEBUG("BaseProcessor clear delegate.\n");
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate), connection);
        isExported = false;
    }
    g_object_unref(delegate);
    delegate = nullptr;

    taskQueueCV.notify_all();

    if (workThread.joinable()) {
        workThread.join();
    }
}
