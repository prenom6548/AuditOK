#ifndef __KMDBUS_PROXY_H__
#define __KMDBUS_PROXY_H__

#include "KMDbusProxySide.h"

class KMDbusProxy
{
public:
    KMDbusProxy();
    ~KMDbusProxy();

    /**
     * @brief 解析命令行参数
     * @param argc 参数个数
     * @param argv 参数列表
     * @return true 解析成功，false 解析失败
     */
    bool parseCommandLine(int argc, char *argv[]);

    /**
     * @brief 打印帮助信息
     */
    void printHelp();

    /**
     * @brief dbus类型转换
     * @param typeStr 字符串类型
     * @return 返回dbus类型
     */
    GBusType stringToBusType(const std::string &typeStr);

    /**
     * @brief 创建 DBus 代理监听的套接字服务
     * @param socketPath 套接字路径
     * @return 创建成功返回 GSocketService 指针，失败返回 nullptr
     */
    GSocketService *createSocketServer(const std::string &socketPath);

    /**
     * @brief 创建与真实总线的连接
     * @note 根据命令行或配置文件选择连接 system bus 或 session bus
     */
    void createConnection(GSocketConnection *connection);

    /**
     * @brief 新客户端连接回调函数
     * @param connection 客户端连接
     * @param remotePeerVanished 源对象
     * @param userData 用户数据（通常为 DbusProxy 实例指针）
     */
    static void onClientClosed(GDBusConnection *connection, gboolean remotePeerVanished, GError *error, gpointer user_data);

    /**
     * @brief 新客户端连接回调函数
     * @param service 监听的 GSocketService
     * @param connection 新建的客户端连接
     * @param sourceObject 源对象
     * @param userData 用户数据（通常为 DbusProxy 实例指针）
     */
    static void onNewConnection(GSocketService *service, GSocketConnection *connection, GObject *sourceObject, gpointer userData);

    /**
     * @brief D-Bus 消息过滤回调函数（处理客户端到服务端的消息）
     * @param connection D-Bus 连接
     * @param message 收到或即将发送的消息
     * @param incoming true 表示收到客户端消息，false 表示要发送出去的消息
     * @param userData 用户数据（通常为 DbusProxy 实例指针）
     * @return 处理后的消息（可以修改或丢弃）
     */
    static GDBusMessage *cliMsgFilterFunc(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer userData);

    /**
     * @brief D-Bus 消息过滤回调函数（处理服务端到客户端的消息）
     * @param connection D-Bus 连接
     * @param message 收到或即将发送的消息
     * @param incoming true 表示收到服务端消息，false 表示要发送出去的消息
     * @param userData 用户数据（通常为 DbusProxy 实例指针）
     * @return 处理后的消息（可以修改或丢弃）
     */
    static GDBusMessage *serMsgFilterFunc(GDBusConnection *connection, GDBusMessage *message, gboolean incoming, gpointer userData);

public:
    /**
     * @brief 启动代理主循环
     * @note 包含套接字监听、消息转发和事件循环
     */
    void run();

public:
    GBusType busType;
    std::string socketPath;
    GSocketService *socketService;
};

#endif // __KMDBUS_PROXY_H__