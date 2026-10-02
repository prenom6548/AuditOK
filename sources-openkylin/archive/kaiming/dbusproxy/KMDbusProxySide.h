#ifndef __KMDBUS_PROXY_SIDE_H__
#define __KMDBUS_PROXY_SIDE_H__

#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>
#include <string>
#include <unordered_map>

#include "common/KMLogger.h"

class KMDbusProxySide
{
public:
    KMDbusProxySide() = default;
    ~KMDbusProxySide() = default;

public:
    GDBusConnection *ser;                        // 服务器连接
    GDBusConnection *cli;                        // 客户端连接
    std::unordered_map<guint32, gint32> serials; // 用于存储请求和回复的序列号映射

public:
    /**
     * @brief 复制并修改 D-Bus 消息中的 IconName 属性（如果包含路径分隔符）
     * @param msg 原始 D-Bus 消息
     * @return 修改后的 D-Bus 消息副本
     */
    void modifyMsgTrayIcon(GDBusMessage *msg, guint32 serial, GDBusConnection *connection);

    /**
     * @brief 查找图标文件的完整路径
     * @param iconName 图标名称
     * @param newIconName 输出参数，返回找到的图标完整路径
     * @return true 找到图标文件，false 未找到
     */
    bool findIconName(const std::string &iconName, std::string &newIconName);
};

#endif // __KMDBUS_PROXY_SIDE_H__