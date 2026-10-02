
/*
 * Combined KMDbusProxyCall Header-Only Implementation
 * This file merges KMDbusProxyCall.h and KMDbusProxyCall.cpp into one for header-only use.
 */

#ifndef __KM_DBUS_PROXY_CALL_HEADER_ONLY_H__
#define __KM_DBUS_PROXY_CALL_HEADER_ONLY_H__

#include <gio/gio.h>
#include <glib.h>

#include <string>
#include <optional>
#include <tuple>

#include "KMLogger.h"

class KMDbusProxyCall
{
public:
    /**
     * @brief 构造函数
     * @param name D-Bus 服务名称
     * @param path D-Bus 对象路径
     * @param interface D-Bus 接口名称
     */
    KMDbusProxyCall(const std::string &name, const std::string &path, const std::string &interface)
        : serviceName(name)
        , objectPath(path)
        , interfaceName(interface)
    {
    }
    ~KMDbusProxyCall() = default;

    /**
     * @brief 调用 D-Bus 系统总线方法
     * @param method 方法名
     * @param parameters 方法参数
     * @param expectReplyType 预期的返回值类型（nullptr 表示不检查）
     * @return true 成功，false 失败
     */
    template<typename T>
    bool methodCallSync(const std::string &method, GVariant *parameters, const std::string &expectReplyType, T &reply, const bool &isSystem = true);

    /**
     * @brief 解析 GVariant 返回值
     * @param variant 返回值的 GVariant 对象
     * @return 返回值的 std::optional<T>，如果解析失败则返回 std::nullopt
     */
    template<typename T>
    std::optional<T> parseVariant(GVariant *variant);

private:
    std::string serviceName;
    std::string objectPath;
    std::string interfaceName;
};

template<typename T>
bool KMDbusProxyCall::methodCallSync(const std::string &method, GVariant *parameters, const std::string &expectReplyType, T &reply, const bool &isSystem)
{
    GError *error = nullptr;
    GDBusConnection *connection = isSystem ? g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error) : g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (!connection)
    {
        KMError("Failed to get D-Bus connection: " + std::string(error->message));
        g_error_free(error);
        return false;
    }

    // KMInfo("Calling D-Bus method: " + method + " on service: " + serviceName + ", object: " + objectPath + ", interface: " + interfaceName);

    GVariant *result = g_dbus_connection_call_sync(
        connection,
        serviceName.c_str(),
        objectPath.c_str(),
        interfaceName.c_str(),
        method.c_str(),
        parameters,
        expectReplyType.empty() ? nullptr : g_variant_type_new(expectReplyType.c_str()),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        nullptr,
        &error);

    if (!result)
    {
        KMError("D-Bus method call failed: " + std::string(error->message));
        g_error_free(error);
        return false;
    }

    auto parsed = parseVariant<T>(result);
    g_variant_unref(result);
    g_object_unref(connection);

    if (parsed)
    {
        reply = *parsed;
        return true;
    }

    KMError("Failed to parse reply variant");
    return false;
}

// parseVariant specializations
template<>
inline std::optional<int> KMDbusProxyCall::parseVariant<int>(GVariant *variant)
{
    int value = 0;
    g_variant_get(variant, "(i)", &value);
    return value;
}

template<>
inline std::optional<std::string> KMDbusProxyCall::parseVariant<std::string>(GVariant *variant)
{
    const gchar *str = nullptr;
    g_variant_get(variant, "(s)", &str);
    return std::string(str);
}

template<>
inline std::optional<std::tuple<int, std::string>> KMDbusProxyCall::parseVariant<std::tuple<int, std::string>>(GVariant *variant)
{
    int ret;
    const gchar *msg;
    g_variant_get(variant, "(is)", &ret, &msg);
    return std::make_tuple(ret, std::string(msg));
}

template<>
inline std::optional<std::tuple<bool>> KMDbusProxyCall::parseVariant(GVariant *variant)
{
    if (!variant || !g_variant_is_of_type(variant, G_VARIANT_TYPE_TUPLE))
    {
        return std::nullopt;
    }

    gboolean value;
    g_variant_get(variant, "(b)", &value);
    return std::make_tuple(static_cast<bool>(value));
}

template<>
inline std::optional<std::tuple<bool, std::string>> KMDbusProxyCall::parseVariant(GVariant *variant)
{
    if (!variant || !g_variant_is_of_type(variant, G_VARIANT_TYPE_TUPLE))
    {
        return std::nullopt;
    }

    bool ret;
    const gchar *msg;
    g_variant_get(variant, "(bs)", &ret, &msg);
    return std::make_tuple(ret, std::string(msg));
}

#endif // __KM_DBUS_PROXY_CALL_HEADER_ONLY_H__
