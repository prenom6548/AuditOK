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

#include "serviceResponse.h"

#include "constant.h"
#include "dataManagement.h"
#include "knowledgebase.h"
#include "knowledgebaseservice.h"
#include "log.h"

#if !defined(ENABLE_EXAMPLE) && !defined(ENABLE_TEST)
/**
 * 获取具体业务的GDBUS通信委托对象
 * 新增业务场景，需要对应增加 skeleton_new 方法，返回值要转为 GObject*
 * @param serviceName
 * @return
 */
GObject* scene::getDelegate(const std::string& serviceName) {
    if (serviceName == SERVICE_NAME_KNOWLEDGEBASE) {
        return reinterpret_cast<GObject*>(ai_business_knowledgebase_skeleton_new());
    } else if (serviceName == SERVICE_DATA_MANAGEMENT) {
        return reinterpret_cast<GObject*>(ai_business_data_management_skeleton_new());
    } else if (serviceName == SERVICE_NAME_KNOWLEDGEBASESERVICE) {
        return reinterpret_cast<GObject*>(ai_business_knowledge_base_service_skeleton_new());
    } else {
        PRINT_ERROR("getDelegate: unknown service: %s\n", serviceName.c_str());
        return nullptr;
    }
}

/**
 * GDBUS同步返回结果
 * 新增业务场景，需要根据服务名称调用对应的业务返回方法，若服务无同步接口，无需增加
 * @param serviceName
 * @param method
 * @param delegate
 * @param invocation
 * @param output
 */
void scene::serviceSyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                GDBusMethodInvocation* invocation, const std::string& output) {
    if (serviceName == SERVICE_DATA_MANAGEMENT) {
        dataManagementSyncResponse(method, delegate, invocation, output);
    } else if (serviceName == SERVICE_NAME_KNOWLEDGEBASE) {
        knowledgebaseSyncResponse(method, delegate, invocation, output);
    } else if (serviceName == SERVICE_NAME_KNOWLEDGEBASESERVICE) {
        knowledgeBaseserviceSyncResponse(method, delegate, invocation, output);
    } else {
        PRINT_ERROR("serviceSyncResponse: unknown service: %s\n", serviceName.c_str());
    }
}

/**
 * GDBUS异步返回结果
 * 新增业务场景，需要根据服务名称调用对应的业务返回方法，若服务无异步接口，无需增加
 * @param serviceName
 * @param method
 * @param delegate
 * @param invocation
 * @param output
 */
void scene::serviceAsyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                 GDBusMethodInvocation* invocation, const std::string& output) {
    if (serviceName == SERVICE_NAME_KNOWLEDGEBASESERVICE) {
        knowledgeBaseserviceAsyncResponse(method, delegate, invocation, output);
    } else {
        PRINT_ERROR("serviceAsyncResponse: unknown service: %s\n", serviceName.c_str());
    }
}

/**
 * GDBUS异步流式返回结果
 * 新增业务场景，需要根据服务名称调用对应的业务返回方法，若服务无异步流式接口，无需增加
 * @param serviceName
 * @param method
 * @param connection
 * @param output
 */
void scene::serviceAsyncFlowResponse(const std::string& serviceName, const std::string& method,
                                     GDBusConnection* connection, GObject* delegate, GDBusMethodInvocation* invocation,
                                     GVariant* output, bool end) {
    if (serviceName == SERVICE_NAME_KNOWLEDGEBASESERVICE) {
        knowledgeBaseserviceAsyncFlowResponse(method, connection, delegate, invocation, output, end);
    } else {
        PRINT_ERROR("serviceAsyncFlowResponse: unknown service: %s\n", serviceName.c_str());
    }
}
#elif defined(ENABLE_EXAMPLE)
#include "example.h"

GObject* scene::getDelegate(const std::string& serviceName) {
    return reinterpret_cast<GObject*>(ai_business_example_skeleton_new());
}

void scene::serviceSyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                GDBusMethodInvocation* invocation, const std::string& output) {
    exampleSyncResponse(method, delegate, invocation, output);
}

void scene::serviceAsyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                 GDBusMethodInvocation* invocation, const std::string& output) {
    exampleAsyncResponse(method, delegate, invocation, output);
}

void scene::serviceAsyncFlowResponse(const std::string& serviceName, const std::string& method,
                                     GDBusConnection* connection, GVariant* output) {
    exampleAsyncFlowResponse(method, connection, output);
}

#elif defined(ENABLE_TEST)
#include "testResponse.h"

GObject* scene::getDelegate(const std::string& serviceName) {
    return reinterpret_cast<GObject*>(ai_business_test_skeleton_new());
}

void scene::serviceSyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                GDBusMethodInvocation* invocation, const std::string& output) {
    testSyncResponse(method, delegate, invocation, output);
}

void scene::serviceAsyncResponse(const std::string& serviceName, const std::string& method, GObject* delegate,
                                 GDBusMethodInvocation* invocation, const std::string& output) {
    testAsyncResponse(method, delegate, invocation, output);
}

void scene::serviceAsyncFlowResponse(const std::string& serviceName, const std::string& method,
                                     GDBusConnection* connection, GVariant* output) {
    testAsyncFlowResponse(method, connection, output);
}

#endif
