#include "KMDbusProxySide.h"

#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief 查找图标文件的完整路径
 * @param iconName 图标名称
 * @param newIconName 输出参数，返回找到的图标完整路径
 * @return true 找到图标文件，false 未找到
 */
bool KMDbusProxySide::findIconName(const std::string &iconName, std::string &newIconName)
{
    if (iconName.empty() || iconName[0] != '/')
    {
        return false;
    }
    if (fs::exists(iconName))
    {
        return false;
    }
    std::error_code ec;
    std::string dir = "/opt/kaiming/layers/stable";
    for (const auto &entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec))
    {
        if (entry.is_regular_file() && entry.path().filename() == fs::path(iconName).filename())
        {
            newIconName = entry.path().string();
            KMDebug("icon name: " + iconName + " new icon: " + newIconName);
            return true;
        }
    }
    return false;
}

/**
 * @brief 复制并修改包含IconName字段的消息，在图标路径前添加前缀
 * @param msg 原始D-Bus消息
 * @param prefix 要添加的前缀，默认为"/opt/kare"
 * @return 新的D-Bus消息，其中IconName字段已添加前缀
 */
void KMDbusProxySide::modifyMsgTrayIcon(GDBusMessage *msg, guint32 serial, GDBusConnection *connection)
{
    do
    {
        // 获取原始消息体
        GVariant *body = g_dbus_message_get_body(msg);
        if (body == nullptr)
            break;

        // 检查消息体类型是否为(a{sv})
        if (!g_variant_is_of_type(body, G_VARIANT_TYPE("(a{sv})")))
            break;

        // 获取元组中的字典数组 从消息体中提取第一个子元素，应该是包含键值对的字典数组
        GVariant *dictArray = g_variant_get_child_value(body, 0);
        if (dictArray == nullptr)
            break;

        // 构建新的字典数组
        gchar *key = nullptr;
        GVariant *value = nullptr;
        GVariantBuilder builder;
        GVariantIter iter;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
        g_variant_iter_init(&iter, dictArray);
        while (g_variant_iter_next(&iter, "{&sv}", &key, &value))
        {
            if (std::string newIconName; g_strcmp0(key, "IconName") == 0 && g_variant_is_of_type(value, G_VARIANT_TYPE_STRING) && findIconName(g_variant_get_string(value, nullptr), newIconName))
            {
                g_variant_builder_add(&builder, "{sv}", key, g_variant_new_string(newIconName.c_str()));
                g_variant_unref(value);
                continue;
            }

            gsize size = g_variant_get_size(value);
            gpointer datacopy = g_malloc(size);
            memcpy(datacopy, g_variant_get_data(value), size);
            GVariant *newvalue = g_variant_new_from_data(g_variant_get_type(value), datacopy, size, false, (GDestroyNotify)g_free, datacopy);
            g_variant_builder_add(&builder, "{sv}", key, newvalue);
            g_variant_unref(value);
        }

        // 释放原始字典数组引用
        g_variant_unref(dictArray);

        // 创建新的字典数组
        GVariant *newDictArray = g_variant_builder_end(&builder);
        if (newDictArray == nullptr)
            break;

        // 创建新的消息体元组
        GVariant *newbody = g_variant_new_tuple(&newDictArray, 1);
        if (newbody == nullptr)
        {
            g_variant_unref(newDictArray);
            break;
        }

        g_dbus_message_set_body(msg, newbody);

    } while (0);

    GError *error = nullptr;
    g_dbus_message_set_reply_serial(msg, serial);
    g_dbus_connection_send_message(connection, msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, &error);
    if (error)
        g_error_free(error);
}