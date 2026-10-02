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

#include <gio/gio.h>

#include <iostream>

#include "exampleglue.h"
#include "log.h"
#include "response.h"

using namespace std;

GDBusConnection* connection;
AiBusinessExample* proxy;
GMainLoop* loop;
const char* objectPath = "/com/kylin/AiBusiness/example";
const char* interfaceName = "com.kylinos.aisdk.business.example";

// 同步接口
void callMethod1() {
    gchar* output;
    string param = R"({"input1":"hello","input2":"world","input3":10})";
    GError* error = nullptr;

    gboolean success = ai_business_example_call_sync_method_sync(proxy, param.c_str(), &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call method1 Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call method1 result: %s\n", output);
}

void callBack2(GObject* source_object, GAsyncResult* res, gpointer user_data) {
    PRINT_INFO("callBack2 回调成功\n");

    GError* error = nullptr;
    gchar* out_response;
    ai_business_example_call_async_method_finish((AiBusinessExample*)source_object, &out_response, res, &error);
    if (error) {
        PRINT_ERROR("Call method2 Error: %s\n", error->message);
        g_error_free(error);
        return;
    }
    PRINT_INFO("Call method2 response: %s\n", out_response);
}

// 异步接口
void callMethod2() {
    string param = R"({"input1":"method2","input2":"world","input3":10})";

    ai_business_example_call_async_method(proxy, param.c_str(), nullptr, callBack2, nullptr);

    PRINT_INFO("Call method2 success.\n");
}

void callBack3Continue(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path,
                       const gchar* interface_name, const gchar* signal_name, GVariant* parameters,
                       gpointer user_data) {
    PRINT_INFO("callBack3Continue 流式返回\n");
    // cout << sender_name << endl; 打印有问题
    // cout << object_path << endl;
    // cout << interface_name << endl;
    // cout << signal_name << endl;

    Response response = Response::parseFromGVariant(parameters);
    PRINT_INFO("callBack3Continue response: %s\n", response.toString().c_str());

    if (!response.isSuccess()) {
        g_main_loop_quit(loop);
    }
}

void callBack3(GObject* source_object, GAsyncResult* res, gpointer user_data) {
    PRINT_INFO("callBack3 回调成功\n");

    GError* error = nullptr;
    char* out_response;
    ai_business_example_call_async_flow_method_finish(reinterpret_cast<AiBusinessExample*>(source_object),
                                                      &out_response, res, &error);
    if (error) {
        PRINT_ERROR("Call method3 Error: %s\n", error->message);
        g_error_free(error);
        return;
    }
    PRINT_INFO("Call method3 response: %s\n", out_response);
}

// 异步流式返回接口
void callMethod3() {
    string param = R"({"request":"method3"})";
    ai_business_example_call_async_flow_method(proxy, param.c_str(), nullptr, nullptr, nullptr);
    PRINT_INFO("call method3 success.\n");
}

void startServer() {
    GError* error = nullptr;

    PRINT_INFO("start connect\n");
    std::string serverUnixPath =
        "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(getuid()) + "/example.sock";
    connection = g_dbus_connection_new_for_address_sync(
        serverUnixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &error);
    if (connection == nullptr) {
        PRINT_ERROR("Error connecting to D-Bus address %s: %s\n", serverUnixPath.c_str(), error->message);
        g_error_free(error);
        return;
    }

    PRINT_INFO("Connected. Negotiated capabilities: unix-fd-passing=%d\n",
               g_dbus_connection_get_capabilities(connection) & G_DBUS_CAPABILITY_FLAGS_UNIX_FD_PASSING);

    proxy =
        ai_business_example_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath, nullptr, &error);
    if (proxy == nullptr) {
        PRINT_ERROR("Error creating speech processor proxy %s: %s\n", objectPath, error->message);
        g_error_free(error);
        return;
    }
    PRINT_INFO("connected server.\n");
}

int main() {
    startServer();

    // 同步请求
    callMethod1();
    callMethod1();
    // 异步请求
    callMethod2();
    callMethod2();
    // 异步流式返回
    unsigned int ret =
        g_dbus_connection_signal_subscribe(connection, nullptr, interfaceName, "callBack3Continue", objectPath, nullptr,
                                           G_DBUS_SIGNAL_FLAGS_NONE, callBack3Continue, nullptr, nullptr);
    std::cout << "g_dbus_connection_signal_subscribe ret = " << ret << std::endl;
    callMethod3();
    callMethod3();

    loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);

    return 0;
}
