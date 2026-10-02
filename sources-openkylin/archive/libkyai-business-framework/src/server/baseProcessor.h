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

#pragma once

#include <gio/gio.h>

#include <condition_variable>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <unordered_set>

#include "register.h"

namespace scene {
class BaseProcessor;

enum MethodType : int {
    SYNC_METHOD = 1,        // 同步接口
    ASYNC_METHOD = 2,       // 异步接口
    ASYNC_FLOW_METHOD = 3,  // 异步流式返回接口
};

// 消息分发的结构体
typedef struct dispatchData {
    std::string method;        // 接口名称
    int type;                  // 接口类型：MethodType：同步接口、异步接口、异步流式返回接口
    void* workHandler;         // 注册进来的业务逻辑函数指针：WorkHandler 或 AsyncWorkHandler
    BaseProcessor* processor;  // 处理请求的processor
} DispatchData;

// 请求任务结构体
typedef struct handlerTask {
    std::string id;
    GObject* delegate;
    GDBusMethodInvocation* invocation;
    std::string request;
    DispatchData* dispatchData;
} HandlerTask;

class BaseProcessor {
public:
    BaseProcessor(const std::string& serviceName, GDBusConnection* connection);
    void connectSignal();
    void exportSkeleton();
    void workHandler();
    bool pushTask(const std::shared_ptr<HandlerTask>& task);

    virtual void syncResponse(const std::string& method, GDBusMethodInvocation* invocation,
                              const std::string& output) {};

    void insertTaskIds(const std::string& taskId);
    void deleteTaskIds(const std::string& taskId);
    void deleteAsyncTaskIds();

    virtual ~BaseProcessor();

public:
    std::string serviceName;
    GDBusConnection* connection;
    gulong closedHandlerId;
    static std::mutex asyncTaskMapMutex;
    static std::map<std::string, std::shared_ptr<HandlerTask>> asyncTaskMap;  // 存放异步任务的队列

protected:
    bool isExported;
    GObject* delegate;  // 处理请求的委托对象
    std::vector<std::shared_ptr<DispatchData>>
        dispatchDataVec;  // 用于连接断开后，方便释放connectSignal中创建的DispatchData内存
    std::mutex taskMutex;
    std::queue<std::shared_ptr<HandlerTask>> taskQueue;  // 所有请求的任务队列
    std::condition_variable taskQueueCV;
    std::unordered_set<std::string> asyncTaskIds; // 共用asyncTaskMapMutex锁

private:
    std::thread workThread;
};
}  // namespace scene
