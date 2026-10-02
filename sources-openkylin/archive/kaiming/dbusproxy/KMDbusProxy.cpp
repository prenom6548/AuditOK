#include "KMDbusProxy.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

KMDbusProxy::KMDbusProxy()
    : busType(G_BUS_TYPE_SESSION)
    , socketService(nullptr)
{
}

KMDbusProxy::~KMDbusProxy()
{
    if (socketService)
    {
        g_object_unref(socketService);
    }
}

/**
 * @brief 解析命令行参数
 * @param argc 参数个数
 * @param argv 参数列表
 * @return true 解析成功，false 解析失败
 */
bool KMDbusProxy::parseCommandLine(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg.find("--type=") == 0)
        {
            std::string typeStr = arg.substr(7);
            busType = stringToBusType(typeStr);
        }
        else if (arg.find("--socket=") == 0)
        {
            socketPath = arg.substr(9);
        }
        else if (arg == "--help")
        {
            printHelp();
            return false;
        }
        else
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            return false;
        }
    }
    return true;
}

/**
 * @brief 打印帮助信息
 */
void KMDbusProxy::printHelp()
{
    std::cout << "Usage: dbus-proxy [OPTIONS]\n\n"
              << "Options:\n"
              << "  --help                Show this help message and exit\n"
              << "  --type=[session|system]\n"
              << "                        Select bus type to proxy (default: session)\n"
              << "  --config=FILE         Path to config file\n"
              << "  --socket=PATH         Path to the proxy's UNIX socket\n"
              << std::endl;
}

/**
 * @brief dbus类型转换
 * @param typeStr 字符串类型
 * @return 返回dbus类型
 */
GBusType KMDbusProxy::stringToBusType(const std::string &typeStr)
{
    if (typeStr == "system" || typeStr == "SYSTEM")
    {
        return G_BUS_TYPE_SYSTEM;
    }
    else if (typeStr == "session" || typeStr == "SESSION")
    {
        return G_BUS_TYPE_SESSION;
    }
    else
    {
        return G_BUS_TYPE_SESSION;
    }
}

/**
 * @brief D-Bus 消息过滤回调函数
 * @param connection D-Bus 连接
 * @param message 收到或即将发送的消息
 * @param incoming true 表示收到客户端消息，false 表示要发送出去的消息
 * @param userData 用户数据（通常为 KMDbusProxy 实例指针）
 * @return 处理后的消息（可以修改或丢弃）
 */
GDBusMessage *KMDbusProxy::cliMsgFilterFunc(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer userData)
{
    // 只处理从客户端传入的消息
    if (!incoming)
    {
        return message;
    }

    // 复制消息以便修改
    GDBusMessage *copy = g_dbus_message_copy(message, nullptr);
    if (copy == nullptr)
    {
        KMError("failed to copy method call message");
        g_object_unref(message);
        return nullptr;
    }

    // 处理不同类型的消息
    GError *error = nullptr;
    KMDbusProxySide *client = static_cast<KMDbusProxySide *>(userData);
    switch (g_dbus_message_get_message_type(copy))
    {
    case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
    {
        // 特殊处理 Hello 消息 处理 Hello：直接回复，不转发到 real bus
        const gchar *interfaceName = g_dbus_message_get_interface(copy);
        const gchar *methodName = g_dbus_message_get_member(copy);
        if (g_strcmp0(interfaceName, "org.freedesktop.DBus") == 0 && g_strcmp0(methodName, "Hello") == 0)
        {
            KMInfo("received Hello message, unique name: " + std::string(g_dbus_connection_get_unique_name(client->ser)));
            GDBusMessage *reply = g_dbus_message_new_method_reply(copy);
            g_dbus_message_set_body(reply, g_variant_new("(s)", g_dbus_connection_get_unique_name(client->ser)));
            g_dbus_connection_send_message(client->cli, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
            g_object_unref(reply);
            break;
        }

        // 转发到 dbus-daemon
        volatile guint32 outSerial = 0;
        guint32 serial = g_dbus_message_get_serial(copy);
        if (g_dbus_connection_send_message(client->ser, copy, G_DBUS_SEND_MESSAGE_FLAGS_NONE, &outSerial, &error))
        {
            guint32 outserial = outSerial;
            client->serials.insert({ outserial, serial });
        }
        else
        {
            KMError("failed to send method call message to real D-Bus bus: " + std::string(error->message));
            // 构造错误回复并发送给客户端
            GDBusMessage *errorMsg = g_dbus_message_new_method_error(copy,
                                                                     "org.freedesktop.DBus.Error.Failed",
                                                                     "Failed to forward message to Real D-Bus");
            g_dbus_connection_send_message(client->cli, errorMsg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
            g_object_unref(errorMsg);
        }
        break;
    }

    case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
    {
        guint32 replySerial = g_dbus_message_get_reply_serial(copy);
        auto it = client->serials.find(replySerial);
        if (it != client->serials.end())
        {
            // 修改托盘图标名称
            client->modifyMsgTrayIcon(copy, it->second, client->ser);
            client->serials.erase(replySerial);
        }
        else
        {
            KMError("method return no pending call found for reply serial: " + std::to_string(replySerial));
        }
        break;
    }
    case G_DBUS_MESSAGE_TYPE_ERROR:
    {
        guint32 replySerial = g_dbus_message_get_reply_serial(copy);
        auto it = client->serials.find(replySerial);
        if (it != client->serials.end())
        {
            g_dbus_message_set_reply_serial(copy, it->second);
            g_dbus_connection_send_message(client->ser, copy, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, &error);
            client->serials.erase(replySerial);
        }
        break;
    }

    case G_DBUS_MESSAGE_TYPE_SIGNAL:
    {
        g_dbus_connection_send_message(client->ser, copy, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, &error);
        break;
    }

    default:
        break;
    }

    // 处理错误
    if (error)
    {
        g_error_free(error);
    }
    // 释放引用
    g_object_unref(copy);
    g_object_unref(message);

    return nullptr;
}

/**
 * @brief D-Bus 消息过滤回调函数（处理服务端到客户端的消息）
 * @param connection D-Bus 连接
 * @param message 收到或即将发送的消息
 * @param incoming true 表示收到服务端消息，false 表示要发送出去的消息
 * @param userData 用户数据（通常为 DbusProxy 实例指针）
 * @return 处理后的消息（可以修改或丢弃）
 */
GDBusMessage *KMDbusProxy::serMsgFilterFunc(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer userData)
{
    // 只处理从服务端传入的消息
    if (!incoming)
    {
        return message;
    }

    // 复制消息以便修改
    GDBusMessage *copy = g_dbus_message_copy(message, nullptr);
    if (copy == nullptr)
    {
        KMError("failed to copy message .");
        g_object_unref(message);
        return nullptr;
    }

    // 处理不同类型的消息
    GError *error = nullptr;
    KMDbusProxySide *side = static_cast<KMDbusProxySide *>(userData);
    switch (g_dbus_message_get_message_type(copy))
    {
    case G_DBUS_MESSAGE_TYPE_METHOD_CALL:
    {
        volatile guint32 outSerial = 0;
        guint32 serial = g_dbus_message_get_serial(copy);
        if (g_dbus_connection_send_message(side->cli, copy, G_DBUS_SEND_MESSAGE_FLAGS_NONE, &outSerial, &error))
        {
            guint32 outserial = outSerial;
            side->serials.insert({ outserial, serial });
        }
        else
        {
            KMError("failed to send method call message to real D-Bus bus: " + std::string(error->message));
            GDBusMessage *errorMsg = g_dbus_message_new_method_error(copy,
                                                                     "org.freedesktop.DBus.Error.Failed",
                                                                     "Failed to forward message to Client");
            g_dbus_connection_send_message(side->ser, errorMsg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
            g_object_unref(errorMsg);
        }
        break;
    }

    case G_DBUS_MESSAGE_TYPE_METHOD_RETURN:
    case G_DBUS_MESSAGE_TYPE_ERROR:
    {
        guint32 replySerial = g_dbus_message_get_reply_serial(copy);
        auto it = side->serials.find(replySerial);
        if (it != side->serials.end())
        {
            g_dbus_message_set_reply_serial(copy, it->second);
            g_dbus_connection_send_message(side->cli, copy, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, &error);
            side->serials.erase(replySerial);
        }
        else
        {
            KMError("method return no pending call found for reply serial: " + std::to_string(replySerial));
        }
        break;
    }

    case G_DBUS_MESSAGE_TYPE_SIGNAL:
    {
        g_dbus_connection_send_message(side->cli, copy, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, &error);
        break;
    }

    default:
        break;
    }

    // 处理错误
    if (error)
    {
        g_error_free(error);
    }

    // 释放引用
    g_object_unref(copy);
    g_object_unref(message);

    return nullptr;
}

/**
 * @brief 客户端连接关闭时的回调函数
 * @param connection 被关闭的 GDBusConnection
 * @param user_data  用户数据（DbusProxy 实例指针）
 */
void KMDbusProxy::onClientClosed(GDBusConnection *connection, gboolean remotePeerVanished, GError *error, gpointer userdata)
{
    // 打印关闭原因
    if (error)
    {
        KMInfo("client connection closed due to " + std::string(error->message));
    }
    else if (remotePeerVanished)
    {
        KMInfo("client connection closed: remote peer vanished .");
    }
    else
    {
        KMInfo("client connection closed normally .");
    }

    // 释放连接对象
    KMDbusProxySide *side = static_cast<KMDbusProxySide *>(userdata);
    g_object_unref(side->cli);
    g_object_unref(side->ser);
    side->cli = nullptr;
    side->ser = nullptr;

    // 释放连接信息结构体
    if (side)
        delete side;
}

/**
 * @brief 创建与真实总线的连接
 * @note 根据命令行或配置文件选择连接 system bus 或 session bus
 * @return 成功返回 GDBusConnection 指针，失败返回 nullptr
 */
void KMDbusProxy::createConnection(GSocketConnection *connection)
{
    GError *error = nullptr;
    gchar *address = g_dbus_address_get_for_bus_sync(busType, nullptr, &error);
    if (error)
    {
        KMError("failed to get address for bus type : " + std::string(error->message));
        g_error_free(error);
        return;
    }

    g_dbus_connection_new_for_address(
        address,
        (GDBusConnectionFlags)(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT | G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
        nullptr,
        nullptr,
        [](GObject *sourceObject, GAsyncResult *res, gpointer userData) {
            GError *error = nullptr;
            GDBusConnection *serConnection = g_dbus_connection_new_for_address_finish(res, &error);
            if (error)
            {
                KMError("failed to create dbus-daemon connection");
                g_error_free(error);
                return;
            }

            // 创建连接信息结构体
            KMDbusProxySide *side = new KMDbusProxySide();
            if (side == nullptr)
            {
                KMError("failed to allocate memory for proxy side .");
                g_object_unref(serConnection);
                return;
            }
            side->ser = serConnection;

            // 创建与客户端的连接
            GSocketConnection *connection = static_cast<GSocketConnection *>(userData);
            gchar *guid = g_dbus_generate_guid();
            GDBusConnection *clientConnection = g_dbus_connection_new_sync(G_IO_STREAM(connection), guid, G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_SERVER, nullptr, nullptr, &error);
            if (error)
            {
                KMError("failed to create client dbus connection, error: " + std::string(error->message));
                g_free(guid);
                g_object_unref(serConnection);
                g_object_unref(connection);
                g_object_unref(clientConnection);
                g_error_free(error);
                delete side;
                return;
            }
            side->cli = clientConnection;

#if 0
            // 获取pid和uid方法
            GError *err = nullptr;
            GCredentials *cred = g_socket_get_credentials(g_socket_connection_get_socket(connection), &err);
            if (err)
            {
                KMError("cannot get peer creds: " + std::string(err->message));
                g_error_free(err);
            }
            else
            {
                pid_t pid = g_credentials_get_unix_pid(cred, &err);
                uid_t uid = g_credentials_get_unix_user(cred, &err);

                KMInfo("peer pid=" + std::to_string(pid) + " uid=" + std::to_string(uid));
                g_object_unref(cred);
            }
#endif

            // 释放guid
            g_free(guid);
            // 添加客户端到服务端消息的过滤器
            g_dbus_connection_add_filter(clientConnection, cliMsgFilterFunc, side, nullptr);
            // 添加服务端到客户端消息的过滤器
            g_dbus_connection_add_filter(serConnection, serMsgFilterFunc, side, nullptr);
            // 连接关闭信号
            g_signal_connect(clientConnection, "closed", G_CALLBACK(onClientClosed), side);
            // 释放connection引用
            g_object_unref(connection);
        },
        connection);

    g_free(address);
}

/**
 * @brief 新客户端连接回调函数
 * @param service 监听的 GSocketService
 * @param connection 新建的客户端连接
 * @param sourceObject 源对象
 * @param userdata 用户数据（通常为 DbusProxy 实例指针）
 */
void KMDbusProxy::onNewConnection(GSocketService *service, GSocketConnection *connection, GObject *sourceObject, gpointer userData)
{
    KMDbusProxy *proxy = static_cast<KMDbusProxy *>(userData);
    g_object_ref(connection);
    // 创建与真实总线的连接
    proxy->createConnection(connection);
}

/**
 * @brief 创建 DBus 代理监听的套接字服务
 * @param socketPath 套接字路径
 * @return 创建成功返回 GSocketService 指针，失败返回 nullptr
 */
GSocketService *KMDbusProxy::createSocketServer(const std::string &socketPath)
{
    if (std::error_code ec; fs::exists(socketPath, ec))
    {
        fs::remove(socketPath, ec);
    }

    // 创建socket服务
    GSocketService *service = g_socket_service_new();

    // 创建socket地址
    GSocketAddress *address = g_unix_socket_address_new(socketPath.c_str());

    // 添加地址到服务
    GError *error = nullptr;
    g_socket_listener_add_address(G_SOCKET_LISTENER(service),
                                  address,
                                  G_SOCKET_TYPE_STREAM,
                                  G_SOCKET_PROTOCOL_DEFAULT,
                                  nullptr,
                                  nullptr,
                                  &error);

    if (error)
    {
        KMError("failed to create dbusproxy socket server: " + std::string(error->message));
        g_error_free(error);
        g_object_unref(address);
        g_object_unref(service);
        return nullptr;
    }

    // 释放地址引用
    g_object_unref(address);

    // 连接新连接信号
    g_signal_connect(service, "incoming", G_CALLBACK(onNewConnection), this);

    // 启动服务
    g_socket_service_start(G_SOCKET_SERVICE(service));

    return service;
}

/**
 * @brief 启动代理主循环
 * @note 包含套接字监听、消息转发和事件循环
 */
void KMDbusProxy::run()
{
    if ((socketService = createSocketServer(socketPath)) == nullptr)
    {
        KMError("Failed to create socket server.");
        return;
    }

    // 创建 GMainLoop 并运行
    GMainLoop *loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}

/**
 * @brief 信号处理函数 
 * @param sig 接收到的信号编号
 */
static void handleSigTerm(int sig)
{
    KMInfo("received signal " + std::to_string(sig));
    const std::string socketPath = "/tmp/kmsessdbusproxy-" + std::to_string(getuid()) + ".sock";

    if (std::error_code ec; fs::exists(socketPath, ec))
    {
        fs::remove(socketPath, ec);
    }

    exit(0);
}

int main(int argc, char *argv[])
{
    KMInfo("uid=" + std::to_string(getuid()));
    signal(SIGTERM, handleSigTerm);
    signal(SIGKILL, handleSigTerm);
    KMDbusProxy proxy;
    proxy.parseCommandLine(argc, argv);

    proxy.socketPath = "/tmp/kmsessdbusproxy-" + std::to_string(getuid()) + ".sock";
    proxy.run();
    return 0;
}
