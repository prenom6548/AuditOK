#include "clientTest.h"

#include "scene.h"

void callBack2(GObject* source_object, GAsyncResult* res, gpointer user_data) {
    PRINT_INFO("callBack2 回调成功\n");
    auto callMethod2 = static_cast<ClientTest*>(user_data);

    GError* error = nullptr;
    gchar* out_response;
    ai_business_test_call_get_async_finish(callMethod2->delegate_, &out_response, res, &error);
    if (error) {
        PRINT_ERROR("Call method2 Error: %s\n", error->message);
        g_error_free(error);
        return;
    }
    PRINT_INFO("Call method2 response: %s\n", out_response);
}

void callBack3Continue(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path,
                       const gchar* interface_name, const gchar* signal_name, GVariant* parameters,
                       gpointer user_data) {
    PRINT_INFO("callBack3Continue 流式返回\n");
    // PRINT_INFO("sender_name: %s\n", sender_name);
    PRINT_INFO("object_path: %s\n", object_path);
    PRINT_INFO("interface_name: %s\n", interface_name);
    PRINT_INFO("signal_name: %s\n", signal_name);

    Response response = Response::parseFromGVariant(parameters);
    PRINT_INFO("callBack3Continue response: %s\n", response.toString().c_str());
}

bool ClientTest::connect() {
    GError* error = nullptr;

    std::string serverUnixPath =
        "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(geteuid()) + "/test.sock";
    connection_ = g_dbus_connection_new_for_address_sync(
        serverUnixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &error);

    if (connection_ == nullptr) {
        g_printerr("ERROR: connecting to test D-Bus address %s failed, code: %d, msg: %s.\n", serverUnixPath.c_str(),
                   error->code, error->message);
        g_error_free(error);
        error = nullptr;
        return false;
    }

    delegate_ = ai_business_test_proxy_new_sync(connection_, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_.c_str(),
                                                nullptr, &error);

    if (delegate_ == nullptr) {
        g_printerr("ERROR: creating test processor proxy %s failed, code: %d, msg: %s.\n", objectPath_.c_str(),
                   error->code, error->message);
        g_error_free(error);
        error = nullptr;
        return false;
    }
    return true;
}

// 同步接口
void ClientTest::callMethod1() {
    gchar* output;
    std::string param =
        R"({"input1":"hello","input2":"world","input3":10,"input4":10.1,"input5":[1,2,3,4,5],"input6":["1","2","3"]})";
    GError* error = nullptr;

    gboolean success = ai_business_test_call_get_sync_sync(delegate_, param.c_str(), G_DBUS_CALL_FLAGS_NONE, -1,
                                                           &output, nullptr, &error);
    if (!success) {
        if (error) {
            PRINT_ERROR("Call method1 Error: %s\n", error->message);
            g_error_free(error);
            return;
        }
    }
    PRINT_INFO("Call method1 result: %s\n", output);
}

// 异步接口
void ClientTest::callMethod2() {
    std::string param = R"({"input1":"method2","input2":"world","input3":10})";

    ai_business_test_call_get_async(delegate_, param.c_str(), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, callBack2, this);

    PRINT_INFO("Call method2 success.\n");
}

void callBack3Finesh(GObject* source_object, GAsyncResult* res, gpointer user_data) {
    PRINT_INFO("callBack3Finesh 回调成功\n");
    auto callBack3Finesh = static_cast<ClientTest*>(user_data);

    GError* error = nullptr;
    gchar* out_response;
    ai_business_test_call_get_async_flow_finish(callBack3Finesh->delegate_, &out_response, res, &error);
    if (error) {
        PRINT_ERROR("Call callBack3Finesh Error: %s\n", error->message);
        g_error_free(error);
        return;
    }
    PRINT_INFO("Call callBack3Finesh response: %s\n", out_response);
}

// 异步流式接口
void ClientTest::callMethod3() {
    // 流式是通过信号来实现的，需要先订阅信号，然后在信号回调中处理
    unsigned int ret = g_dbus_connection_signal_subscribe(
        connection_, nullptr, interfaceName_.c_str(), "callBack3Continue", objectPath_.c_str(), nullptr,
        G_DBUS_SIGNAL_FLAGS_NONE, callBack3Continue, nullptr, nullptr);
    PRINT_INFO("g_dbus_connection_signal_subscribe ret = %d\n", ret);

    std::string param = R"({"input1":"method3"})";

    // 异步流式我发现，如果传callback了，等超时时间到了 服务端还没有调用complete就会崩溃
    ai_business_test_call_get_async_flow(delegate_, param.c_str(), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr,
                                         nullptr);

    PRINT_INFO("Call method3 success.\n");
}

void ClientTest::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
        delegate_ = nullptr;
    }
    if (connection_ != nullptr) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
}

ClientTest::~ClientTest() { destroy(); }
