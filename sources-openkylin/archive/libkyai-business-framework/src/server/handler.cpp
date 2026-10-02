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

#include "handler.h"

#include <memory>

#include "businessServer.h"
#include "log.h"
#include "processorBuilder.h"
#include "utils.h"

gboolean scene::onNewConnection(GDBusServer* server, GDBusConnection* connection, gpointer userData) {
    // 打印客户端信息
    GCredentials* credentials = g_dbus_connection_get_peer_credentials(connection);
    gchar* s = g_credentials_to_string(credentials);
    PRINT_INFO("Client connected. Peer credentials: %s. Negotiated capabilities: unix-fd-passing=%d\n", s,
               g_dbus_connection_get_capabilities(connection) & G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING);

    auto businessServer = static_cast<BusinessServer*>(userData);
    // 根据serviceName创建具体的processor
    auto processor = ProcessorBuilder::build(businessServer->serviceName, connection);
    // 存当前连接和processor的映射关系，待连接断开时释放processor资源
    businessServer->connectionProcessorMap.insert(std::make_pair(connection, processor));
    g_object_ref(connection);

    // 注册接口
    processor->connectSignal();
    processor->exportSkeleton();
    // 启动任务消费线程
    processor->workHandler();

    processor->closedHandlerId = g_signal_connect(connection, "closed", G_CALLBACK(onConnectionClosed), businessServer);
    // PRINT_DEBUG("build processor %s, use count %lu\n", server->serviceName.c_str(), processor.use_count());

    return true;
}

void scene::onConnectionClosed(GDBusConnection* connection, gboolean remotePeerVanished, GError* error,
                               gpointer userData) {
    PRINT_INFO("Connection closed. clear processor.\n");

    auto server = static_cast<BusinessServer*>(userData);
    auto it = server->connectionProcessorMap.find(connection);
    if (it == server->connectionProcessorMap.end()) {
        PRINT_WARN("onConnectionClosed cannot find connection.\n");
        return;
    }

    if (it->second->closedHandlerId > 0) {
        PRINT_DEBUG("onConnectionClosed disconnect closedHandlerId: %lu\n", it->second->closedHandlerId);
        g_signal_handler_disconnect(connection, it->second->closedHandlerId);
        it->second->closedHandlerId = 0;
    }

    // 使用智能指针释放 processor 资源
    server->connectionProcessorMap.erase(connection);
    g_object_unref(connection);
    PRINT_INFO("Connection closed. clear processor end.\n");
}

/**
 * 请求消息统一进行分发，可扩展公共逻辑（鉴权），请求转任务结构体后入队
 * @param delegate
 * @param invocation
 * @param request
 * @param userData
 * @return
 */
bool scene::dispatchRequest(GObject* delegate, GDBusMethodInvocation* invocation, gchar* request, gpointer userData) {
    auto dispatchData = static_cast<DispatchData*>(userData);
    PRINT_INFO("dispatchRequest start, method: %s, request: %s\n", dispatchData->method.c_str(), request);

    // 请求参数转换为任务结构体
    auto task = std::make_shared<HandlerTask>();
    task->id = Utils::generateUUID();
    task->delegate = delegate;
    task->invocation = invocation;
    task->request = request;
    task->dispatchData = dispatchData;
    PRINT_DEBUG("create task id: %s, method: %s.\n", task->id.c_str(), dispatchData->method.c_str());

    // 任务入队
    dispatchData->processor->pushTask(task);
    // PRINT_DEBUG("push task %s use count %lu\n", task->id.c_str(), task.use_count());

    PRINT_INFO("dispatchRequest end.\n");
    return true;
}
